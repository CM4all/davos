/*
 * Error response generator.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "error.hxx"

extern "C" {
#include <was/simple.h>
}

#include <errno.h>

void
errno_respones(was_simple *was, int e)
{
    switch (e) {
    case ENOENT:
    case ENOTDIR:
        was_simple_status(was, HTTP_STATUS_NOT_FOUND);
        break;

    case EACCES:
    case EPERM:
    case EROFS:
        was_simple_status(was, HTTP_STATUS_FORBIDDEN);
        break;

    case ENAMETOOLONG:
        was_simple_status(was, HTTP_STATUS_REQUEST_URI_TOO_LONG);
        break;

    case ENOSPC:
        was_simple_status(was, HTTP_STATUS_INSUFFICIENT_STORAGE);
        break;

    case ENOTEMPTY:
    case EBUSY:
        was_simple_status(was, HTTP_STATUS_FAILED_DEPENDENCY);
        break;

    case EINVAL:
        was_simple_status(was, HTTP_STATUS_BAD_REQUEST);
        break;

    default:
        was_simple_status(was, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        break;
    }
}

void
errno_respones(was_simple *was)
{
    errno_respones(was, errno);
}
