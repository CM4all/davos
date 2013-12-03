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
}

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

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
handle_move(was_simple *w, const char *path, const char *destination_path)
{
    if (rename(path, destination_path) < 0) {
        errno_respones(w);
        return;
    }
}
