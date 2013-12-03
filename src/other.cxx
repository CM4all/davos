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
handle_options(was_simple *was, const char *path)
{
    const char *allow_new = "OPTIONS,MKCOL,PUT,LOCK";
    const char *allow_file =
        "OPTIONS,GET,HEAD,DELETE,PROPFIND,PROPPATCH,COPY,MOVE,PUT,LOCK,UNLOCK";
    const char *allow_directory =
        "OPTIONS,DELETE,PROPFIND,PROPPATCH,COPY,MOVE,PUT,LOCK,UNLOCK";

    const char *allow;
    struct stat st;
    if (stat(path, &st) < 0)
        allow = allow_new;
    else if (S_ISDIR(st.st_mode))
        allow = allow_directory;
    else
        allow = allow_file;

    was_simple_set_header(was, "allow", allow);
}

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
