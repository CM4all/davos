/*
 * Request handlers for "other" commands.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "other.hxx"
#include "error.hxx"

extern "C" {
#include <was/simple.h>
#include <fox/unlink.h>
#include <fox/cp.h>
}

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

void
handle_delete(was_simple *w, const char *path)
{
    fox_error_t error;
    fox_status_t status = fox_unlink(path, 0, &error);
    if (status != FOX_STATUS_SUCCESS) {
        errno_respones(w, status);
        return;
    }
}

void
handle_copy(was_simple *w, const char *path, const char *destination_path)
{
    // TODO: support "Depth: 0"
    // TODO: overwriting an existing directory?

    unsigned options = FOX_CP_DEVICE|FOX_CP_INODE|FOX_CP_PRESERVE_XATTR;

    const char *overwrite = was_simple_get_header(w, "overwrite");
    if (strcmp(overwrite, "f") == 0)
        options |= FOX_CP_EXCL;

    fox_error_t error;
    fox_status_t status = fox_copy(path, destination_path, options, &error);
    if (status != FOX_STATUS_SUCCESS) {
        errno_respones(w, status);
        return;
    }
}

void
handle_move(was_simple *w, const char *path, const char *destination_path)
{
    // TODO: support "Overwrite"

    if (rename(path, destination_path) < 0) {
        errno_respones(w);
        return;
    }
}
