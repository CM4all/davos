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
        was_simple_status(was, HTTP_STATUS_NOT_FOUND);
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
