/*
 * Request handler for local files.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "get.hxx"
#include "error.hxx"
#include "file.hxx"
#include "splice.hxx"

extern "C" {
#include "format.h"
#include "date.h"

#include <was/simple.h>
}

#include <unistd.h>
#include <fcntl.h>

static void
static_etag(char *p, const struct stat &st)
{
    *p++ = '"';

    p += format_uint32_hex(p, (uint32_t)st.st_dev);

    *p++ = '-';

    p += format_uint32_hex(p, (uint32_t)st.st_ino);

    *p++ = '-';

    p += format_uint32_hex(p, (uint32_t)st.st_mtime);

    *p++ = '"';
    *p = 0;
}

static bool
static_response_headers(was_simple *was, const FileResource &resource)
{
    const char *content_type = "application/octet-stream";

    if (!was_simple_set_header(was, "content-type", content_type) ||
        !was_simple_set_header(was, "last-modified",
                               http_date_format(resource.GetModificationTime())))
        return false;

    {
        char buffer[128];
        static_etag(buffer, resource.GetStat());
        if (!was_simple_set_header(was, "etag", buffer))
            return false;
    }

    return true;
}

void
handle_get(was_simple *was, const FileResource &resource)
{
    // TODO: range, if-modified-since, if-match, ...

    was_simple_input_close(was);

    const int fd = open(resource.GetPath(), O_RDONLY|O_NOCTTY);
    if (fd < 0) {
        errno_respones(was);
        close(fd);
        return;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        errno_respones(was);
        close(fd);
        return;
    }

    if (!S_ISREG(st.st_mode)) {
        close(fd);
        was_simple_status(was, HTTP_STATUS_METHOD_NOT_ALLOWED);
        return;
    }

    if (static_response_headers(was, resource))
        splice_to_was(was, fd, resource.GetSize());

    close(fd);
}

void
handle_head(was_simple *was, const FileResource &resource)
{
    was_simple_input_close(was);

    if (!resource.Exists()) {
        errno_respones(was, resource.GetError());
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
