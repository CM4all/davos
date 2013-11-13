/*
 * Request handler for local files.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "file.hxx"
#include "error.hxx"

extern "C" {
#include "format.h"
#include "date.h"

#include <was/simple.h>
}

#include <algorithm>
#include <limits>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

static void
static_etag(char *p, const struct stat *st)
{
    *p++ = '"';

    p += format_uint32_hex(p, (uint32_t)st->st_dev);

    *p++ = '-';

    p += format_uint32_hex(p, (uint32_t)st->st_ino);

    *p++ = '-';

    p += format_uint32_hex(p, (uint32_t)st->st_mtime);

    *p++ = '"';
    *p = 0;
}

static bool
static_response_headers(was_simple *was, const struct stat *st)
{
    const char *content_type = "application/octet-stream";

    if (!was_simple_set_header(was, "content-type", content_type) ||
        !was_simple_set_header(was, "last-modified",
                               http_date_format(st->st_mtime)))
        return false;

    {
        char buffer[128];
        static_etag(buffer, st);
        if (!was_simple_set_header(was, "etag", buffer))
            return false;
    }

    return true;
}

static void
copy_from_fd(was_simple *was, int in_fd, uint64_t remaining)
{
    if (!was_simple_set_length(was, remaining))
        return;

    const int out_fd = was_simple_output_fd(was);
    while (remaining > 0) {
        constexpr uint64_t max = std::numeric_limits<size_t>::max();
        size_t length = std::min(remaining, max);
        ssize_t nbytes = splice(in_fd, nullptr, out_fd, nullptr,
                                length,
                                SPLICE_F_MOVE|SPLICE_F_MORE);
        if (nbytes <= 0) {
            if (nbytes < 0)
                fprintf(stderr, "splice() failed: %s\n", strerror(errno));
            break;
        }

        if (!was_simple_sent(was, nbytes))
            break;

        remaining -= nbytes;
    }
}

void
get(was_simple *was, const char *path)
{
    // TODO: range, if-modified-since, if-match, ...

    was_simple_input_close(was);

    const int fd = open(path, O_RDONLY|O_NOCTTY);
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

    if (static_response_headers(was, &st))
        copy_from_fd(was, fd, st.st_size);

    close(fd);
}

void
head(was_simple *was, const char *path)
{
    was_simple_input_close(was);

    struct stat st;
    if (stat(path, &st) < 0) {
        errno_respones(was);
        return;
    }

    if (!S_ISREG(st.st_mode)) {
        was_simple_status(was, HTTP_STATUS_METHOD_NOT_ALLOWED);
        return;
    }

    {
        char buffer[64];
        sprintf(buffer, "%llu", (unsigned long long)st.st_size);
        if (!was_simple_set_header(was, "content-length", buffer))
            return;
    }

    static_response_headers(was, &st);
}
