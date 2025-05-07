// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * Request handler for local files.
 */

#include "get.hxx"
#include "ETag.hxx"
#include "IfMatch.hxx"
#include "error.hxx"
#include "file.hxx"
#include "mime_types.hxx"
#include "was/ExceptionResponse.hxx"
#include "was/Splice.hxx"
#include "lib/fmt/ToBuffer.hxx"
#include "io/UniqueFileDescriptor.hxx"
#include "http/Date.hxx"
#include "http/Range.hxx"
#include "time/StatxCast.hxx"
#include "util/StringAPI.hxx"

#include <was/simple.h>

#include <fmt/format.h>

#include <unistd.h>
#include <fcntl.h>

static bool
SendETagHeader(was_simple *was, const struct statx &st) noexcept
{
	return was_simple_set_header(was, "etag", MakeETag(st));
}

static bool
static_response_headers(was_simple *was, const FileResource &resource)
{
	const char *content_type = LookupMimeTypeByFilePath(resource.GetPath());
	if (content_type == nullptr)
		content_type = "application/octet-stream";

	if (!was_simple_set_header(was, "content-type", content_type) ||
	    !was_simple_set_header(was, "accept-ranges", "bytes") ||
	    !was_simple_set_header(was, "last-modified",
				   http_date_format(resource.GetModificationTime())))
		return false;

	return SendETagHeader(was, resource.GetStat());
}

static bool
SendNotModified(was_simple *was, const struct statx &st) noexcept
{
	return was_simple_status(was, HTTP_STATUS_NOT_MODIFIED) &&
		SendETagHeader(was, st);
}

static void
HandleIfModifiedSince(was_simple *was, const struct statx &st)
{
	const char *p = was_simple_get_header(was, "if-modified-since");
	if (p == nullptr)
		return;

	const auto t = http_date_parse(p);
	if (t < std::chrono::system_clock::time_point()) {
		was_simple_status(was, HTTP_STATUS_BAD_REQUEST);
		throw Was::EndResponse{};
	}

	if (ToSystemTimePoint(st.stx_mtime) < t) {
		SendNotModified(was, st);
		throw Was::EndResponse{};
	}
}

static void
HandleIfUnmodifiedSince(was_simple *was, const struct statx &st)
{
	const char *p = was_simple_get_header(was, "if-unmodified-since");
	if (p == nullptr)
		return;

	const auto t = http_date_parse(p);
	if (t < std::chrono::system_clock::time_point()) {
		was_simple_status(was, HTTP_STATUS_BAD_REQUEST);
		throw Was::EndResponse{};
	}

	if (ToSystemTimePoint(st.stx_mtime) >= t) {
		was_simple_status(was, HTTP_STATUS_PRECONDITION_FAILED);
		throw Was::EndResponse{};
	}
}

static void
HandleIfMatch(was_simple *was, const struct statx &st)
{
	if (!CheckIfMatch(*was, &st)) {
		was_simple_status(was, HTTP_STATUS_PRECONDITION_FAILED);
		throw Was::EndResponse{};
	}
}

static void
HandleIfNoneMatch(was_simple *was, const struct statx &st)
{
	if (!CheckIfNoneMatch(*was, &st)) {
		SendNotModified(was, st);
		throw Was::EndResponse{};
	}
}

/**
 * Verifies the If-Range request header (RFC 2616 14.27).
 */
static bool
CheckIfRange(const char *if_range, const struct statx &st)
{
	if (if_range == nullptr)
		return true;

	const auto t = http_date_parse(if_range);
	if (t != std::chrono::system_clock::from_time_t(-1))
		return std::chrono::system_clock::from_time_t(st.stx_mtime.tv_sec) == t;

	return StringIsEqual(if_range, MakeETag(st));
}

void
handle_get(was_simple *was, const FileResource &resource)
{
	UniqueFileDescriptor fd;
	if (!fd.Open(resource.GetPath(), O_RDONLY)) {
		errno_response(was);
		return;
	}

	struct statx st;
	if (statx(fd.Get(), "", AT_EMPTY_PATH|AT_STATX_SYNC_AS_STAT,
		  STATX_TYPE|STATX_ATIME|STATX_MTIME|STATX_INO|STATX_SIZE,
		  &st) < 0) {
		errno_response(was);
		return;
	}

	if (!S_ISREG(st.stx_mode)) {
		was_simple_status(was, HTTP_STATUS_METHOD_NOT_ALLOWED);
		return;
	}

	HandleIfModifiedSince(was, st);
	HandleIfUnmodifiedSince(was, st);
	HandleIfMatch(was, st);
	HandleIfNoneMatch(was, st);

	HttpRangeRequest range(st.stx_size);

	const char *p = was_simple_get_header(was, "range");
	if (p != nullptr &&
	    CheckIfRange(was_simple_get_header(was, "if-range"), st))
		range.ParseRangeHeader(p);

	switch (range.type) {
	case HttpRangeRequest::Type::NONE:
		break;

	case HttpRangeRequest::Type::VALID:
		if (!was_simple_status(was, HTTP_STATUS_PARTIAL_CONTENT))
			return;

		if (!was_simple_set_header(was, "content-range",
					   FmtBuffer<128>("bytes {}-{}/{}",
							  range.skip,
							  range.size - 1,
							  st.stx_size)))
		    return;

		break;

	case HttpRangeRequest::Type::INVALID:
		if (!was_simple_status(was, HTTP_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE))
			return;

		if (!was_simple_set_header(was, "content-range",
					   FmtBuffer<128>("bytes */{}",
							  st.stx_size)))
		    return;

		static_response_headers(was, resource);
		return;
	}

	if (range.skip > 0 && fd.Seek(range.skip) < 0) {
		errno_response(was);
		return;
	}

	if (static_response_headers(was, resource))
		SpliceToWas(was, fd, range.size - range.skip);
}

void
handle_head(was_simple *was, const FileResource &resource)
{
	if (!resource.Exists()) {
		errno_response(was, resource.GetError());
		return;
	}

	if (!resource.IsFile()) {
		was_simple_status(was, HTTP_STATUS_METHOD_NOT_ALLOWED);
		return;
	}

	if (!was_simple_set_header(was, "content-length",
				   fmt::format_int{resource.GetSize()}.c_str()))
		return;

	static_response_headers(was, resource);
}
