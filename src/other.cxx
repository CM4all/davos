// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * Request handlers for "other" commands.
 */

#include "other.hxx"
#include "error.hxx"
#include "file.hxx"
#include "util.hxx"
#include "system/Error.hxx"
#include "io/FileDescriptor.hxx"
#include "io/RecursiveCopy.hxx"
#include "io/RecursiveDelete.hxx"

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

	unsigned options = RECURSIVE_COPY_ONE_FILESYSTEM;

	if (!get_overwrite_header(w))
		options |= RECURSIVE_COPY_NO_OVERWRITE;

	try {
		RecursiveCopy(FileDescriptor{AT_FDCWD}, src.GetPath(),
			      FileDescriptor{AT_FDCWD}, dest.GetPath(),
			      options);
	} catch (const std::system_error &e) {
		if (e.code().category() == ErrnoCategory())
			errno_response(w, e.code().value());
		else
			throw;
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
