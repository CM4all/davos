/*
 * Request handler for directories (e.g. MKCOL).
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "directory.hxx"
#include "error.hxx"

extern "C" {
#include <was/simple.h>
}

#include <errno.h>
#include <sys/stat.h>

void
mkcol(was_simple *w, const char *path)
{
    if (mkdir(path, 0777) < 0) {
        const int e = errno;
        if (errno == EEXIST || errno == ENOTDIR)
            was_simple_status(w, HTTP_STATUS_CONFLICT);
        else
            errno_respones(w, e);
        return;
    }

    was_simple_status(w, HTTP_STATUS_CREATED);
}
