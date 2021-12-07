/*
 * Request handler for local files.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "get.hxx"
#include "ETag.hxx"
#include "IfMatch.hxx"
#include "error.hxx"
#include "was.hxx"
#include "file.hxx"
#include "splice.hxx"
#include "mime_types.hxx"
#include "Chrono.hxx"
#include "io/UniqueFileDescriptor.hxx"
#include "http/Date.hxx"
#include "http/Range.hxx"
#include "util/StringView.hxx"

#include <was/simple.h>

#include <unistd.h>
#include <fcntl.h>

static bool
SendETagHeader(was_simple *was, const struct stat &st) noexcept
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
SendNotModified(was_simple *was, const struct stat &st) noexcept
{
    return was_simple_status(was, HTTP_STATUS_NOT_MODIFIED) &&
        SendETagHeader(was, st);
}

static void
HandleIfModifiedSince(was_simple *was, const struct stat &st)
{
    const char *p = was_simple_get_header(was, "if-modified-since");
    if (p == nullptr)
        return;

    const auto t = http_date_parse(p);
    if (t < std::chrono::system_clock::time_point()) {
        was_simple_status(was, HTTP_STATUS_BAD_REQUEST);
        throw WasBreak();
    }

    if (ToSystemTime(st.st_mtim) < t) {
        SendNotModified(was, st);
        throw WasBreak();
    }
}

static void
HandleIfUnmodifiedSince(was_simple *was, const struct stat &st)
{
    const char *p = was_simple_get_header(was, "if-unmodified-since");
    if (p == nullptr)
        return;

    const auto t = http_date_parse(p);
    if (t < std::chrono::system_clock::time_point()) {
        was_simple_status(was, HTTP_STATUS_BAD_REQUEST);
        throw WasBreak();
    }

    if (ToSystemTime(st.st_mtim) >= t) {
        was_simple_status(was, HTTP_STATUS_PRECONDITION_FAILED);
        throw WasBreak();
    }
}

static void
HandleIfMatch(was_simple *was, const struct stat &st)
{
    if (!CheckIfMatch(*was, &st)) {
        was_simple_status(was, HTTP_STATUS_PRECONDITION_FAILED);
        throw WasBreak();
    }
}

static void
HandleIfNoneMatch(was_simple *was, const struct stat &st)
{
    if (!CheckIfNoneMatch(*was, &st)) {
        SendNotModified(was, st);
        throw WasBreak();
    }
}

/**
 * Verifies the If-Range request header (RFC 2616 14.27).
 */
static bool
CheckIfRange(const char *if_range, const struct stat &st)
{
    if (if_range == nullptr)
        return true;

    const auto t = http_date_parse(if_range);
    if (t != std::chrono::system_clock::from_time_t(-1))
        return std::chrono::system_clock::from_time_t(st.st_mtime) == t;

    return strcmp(if_range, MakeETag(st)) == 0;
}

void
handle_get(was_simple *was, const FileResource &resource)
{
    UniqueFileDescriptor fd;
    if (!fd.Open(resource.GetPath(), O_RDONLY)) {
        errno_response(was);
        return;
    }

    struct stat st;
    if (fstat(fd.Get(), &st) < 0) {
        errno_response(was);
        return;
    }

    if (!S_ISREG(st.st_mode)) {
        was_simple_status(was, HTTP_STATUS_METHOD_NOT_ALLOWED);
        return;
    }

    HandleIfModifiedSince(was, st);
    HandleIfUnmodifiedSince(was, st);
    HandleIfMatch(was, st);
    HandleIfNoneMatch(was, st);

    HttpRangeRequest range(st.st_size);

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

        {
            char buffer[128];
            snprintf(buffer, sizeof(buffer), "bytes %llu-%llu/%llu",
                     (unsigned long long)range.skip,
                     (unsigned long long)(range.size - 1),
                     (unsigned long long)st.st_size);
            if (!was_simple_set_header(was, "content-range", buffer))
                return;
        }

        break;

    case HttpRangeRequest::Type::INVALID:
        if (!was_simple_status(was, HTTP_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE))
            return;

        {
            char buffer[128];
            snprintf(buffer, sizeof(buffer), "bytes */%llu",
                     (unsigned long long)st.st_size);
            if (!was_simple_set_header(was, "content-range", buffer))
                return;
        }

        static_response_headers(was, resource);
        return;
    }

    if (range.skip > 0 && fd.Seek(range.skip) < 0) {
        errno_response(was);
        return;
    }

    if (static_response_headers(was, resource))
        splice_to_was(was, fd.Get(), range.size - range.skip);
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

    {
        char buffer[64];
        sprintf(buffer, "%llu", (unsigned long long)resource.GetSize());
        if (!was_simple_set_header(was, "content-length", buffer))
            return;
    }

    static_response_headers(was, resource);
}
