/*
 * Request handlers for "other" commands.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "other.hxx"
#include "error.hxx"

extern "C" {
#include <was/simple.h>
}

#include <errno.h>
#include <unistd.h>

void
handle_delete(was_simple *w, const char *path)
{
    if (unlink(path) == 0)
        return;

    if (errno == EISDIR) {
        if (rmdir(path) == 0)
            return;

        // TODO: recursive delete on errno==ENOTEMPTY
    }

    errno_respones(w);
}
