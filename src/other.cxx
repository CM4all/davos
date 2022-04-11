/*
 * Request handlers for "other" commands.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "other.hxx"
#include "error.hxx"
#include "file.hxx"
#include "util.hxx"
#include "system/Error.hxx"
#include "io/FileDescriptor.hxx"
#include "io/RecursiveDelete.hxx"

extern "C" {
#include <fox/cp.h>
}

#include <fcntl.h>

void
handle_delete(was_simple *w, const FileResource &resource)
{
    try {
        RecursiveDelete(FileDescriptor{AT_FDCWD}, resource.GetPath());
    } catch (const std::system_error &e) {
        if (e.code().category() == ErrnoCategory())
            errno_response(w, e.code().value());
        else
            throw;
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
