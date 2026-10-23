// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * PROPFIND implementation.
 */

#include "propfind.hxx"
#include "uri_escape.hxx"
#include "wxml.hxx"
#include "error.hxx"
#include "file.hxx"
#include "was/WasOutputStream.hxx"
#include "http/Date.hxx"
#include "io/DirectoryReader.hxx"
#include "time/StatxCast.hxx"

#include <was/simple.h>

#include <cassert>
#include <compare>
#include <cstdint>
#include <forward_list>
#include <set>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>

static std::size_t MAX_FILES = 2000;
static unsigned MAX_DEPTH = 3;

/**
 * The request-wide budget: each listed directory entry and each
 * statx() attempt consumes one unit.
 */
static constexpr std::size_t MAX_OPERATIONS = 16384;

static std::forward_list<std::string>
ListDirectory(const char *path, std::size_t &budget)
try {
	std::forward_list<std::string> result;
	std::size_t n = 0;
	DirectoryReader r{path};
	while (n < MAX_FILES && budget > 0) {
		const char *name = r.Read();
		if (name == nullptr)
			break;

		if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
			continue;

		result.emplace_front(name);
		++n;
		--budget;
	}

	return result;
} catch (...) {
	return {};
}

namespace {

struct FileId {
	uint_least32_t dev_major, dev_minor;
	uint_least64_t ino;

	explicit FileId(const struct statx &st) noexcept
		:dev_major(st.stx_dev_major), dev_minor(st.stx_dev_minor),
		 ino(st.stx_ino)
	{
		assert(st.stx_mask & STATX_INO);
	}

	constexpr auto operator<=>(const FileId &) const noexcept = default;
};

class PropfindWalker {
	BufferedOutputStream &o;
	std::string uri, path;

	/**
	 * Directories which have already been listed.  This
	 * prevents symlink loops from multiplying the work.
	 */
	std::set<FileId> visited;

	/**
	 * The remaining request-wide budget (see #MAX_OPERATIONS).
	 * The walk stops when it is exhausted.
	 */
	std::size_t budget = MAX_OPERATIONS;

public:
	PropfindWalker(BufferedOutputStream &_o,
		       std::string_view _uri, std::string_view _path) noexcept
		:o(_o), uri(_uri), path(_path) {}

	void Walk(const struct statx &st, unsigned depth);
};

void
PropfindWalker::Walk(const struct statx &st, unsigned depth)
{
	open_response_prop(o, uri, "HTTP/1.1 200 OK");

	if (S_ISDIR(st.stx_mode)) {
		resourcetype_collection(o);
	} else if (S_ISREG(st.stx_mode)) {
		wxml_fmt_element(o, "D:getcontentlength", "{}", st.stx_size);
	}

	const auto mtime = ToSystemTimePoint(st.stx_mtime);

	wxml_string_element(o, "D:getlastmodified", http_date_format(mtime));

	close_response_prop(o);

	if (depth > 0 && budget > 0 && S_ISDIR(st.stx_mode) &&
	    visited.emplace(st).second) {
		--depth;

		if (uri.back() != '/')
			/* directory URIs should end with a slash - but we don't
			   enforce that everywhere; fix up the URI if the
			   user-supplied URI doesn't */
			uri.push_back('/');

		const auto uri_length = uri.length();

		const auto &children = ListDirectory(path.c_str(), budget);

		path.push_back('/');
		const auto path_length = path.length();

		for (const std::string_view name : children) {
			if (budget == 0)
				break;

			--budget;

			AppendUriEscape(uri, name);
			path.append(name);

			struct statx st2;
			if (statx(-1, path.c_str(), AT_STATX_SYNC_AS_STAT,
				  STATX_TYPE|STATX_MTIME|STATX_SIZE|STATX_INO,
				  &st2) == 0) {
				if (S_ISDIR(st2.stx_mode))
					/* directory URIs should end with a slash */
					uri.push_back('/');

				Walk(st2, depth);
			}

			uri.erase(uri_length);
			path.erase(path_length);
		}
	}
}

} // anonymous namespace

void
handle_propfind(was_simple *was, const char *uri, const FileResource &resource)
{
	if (!resource.Exists()) {
		errno_response(was, resource.GetError());
		return;
	}

	unsigned depth = 0;
	const char *depth_string = was_simple_get_header(was, "depth");
	if (depth_string != nullptr)
		depth = strtoul(depth_string, nullptr, 10);
	if (depth > MAX_DEPTH)
		depth = MAX_DEPTH;

	if (!was_simple_status(was, HTTP_STATUS_MULTI_STATUS) ||
	    !was_simple_set_header(was, "content-type",
				   "text/xml; charset=\"utf-8\""))
		return;

	WasOutputStream wos{was};
	BufferedOutputStream bos{wos};

	begin_multistatus(bos);

	PropfindWalker{bos, uri, resource.GetPathView()}
		.Walk(resource.GetStat(), depth);
	end_multistatus(bos);

	bos.Flush();
}
