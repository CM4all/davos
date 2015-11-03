/*
 * Request handlers for "other" commands.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "other.hxx"
#include "error.hxx"
#include "file.hxx"
#include "util.hxx"

extern "C" {
#include <was/simple.h>
#include <fox/unlink.h>
#include <fox/cp.h>
}

#include <errno.h>
#include <unistd.h>
#include <stdio.h>

void
handle_delete(was_simple *w, const FileResource &resource)
{
    fox_error_t error;
    fox_status_t status = fox_unlink(resource.GetPath(), 0, &error);
    if (status != FOX_STATUS_SUCCESS) {
        errno_response(w, status);
        return;
    }
}

void
handle_copy(was_simple *w, const FileResource &src, const FileResource &dest)
{
    // TODO: support "Depth: 0"
    // TODO: overwriting an existing directory?

    unsigned options = FOX_CP_DEVICE|FOX_CP_INODE|FOX_CP_PRESERVE_XATTR;

    if (!get_overwrite_header(w))
        options |= FOX_CP_EXCL;

    fox_error_t error;
    fox_status_t status = fox_copy(src.GetPath(), dest.GetPath(), options,
                                   &error);
    if (status != FOX_STATUS_SUCCESS) {
        errno_response(w, status);
        return;
    }
}

void
handle_move(was_simple *w, const FileResource &src, const FileResource &dest)
{
    // TODO: support "Overwrite"

    if (rename(src.GetPath(), dest.GetPath()) < 0) {
        errno_response(w);
        return;
    }
}
