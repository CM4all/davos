// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * Request handler for directories (e.g. MKCOL).
 */

#include "directory.hxx"
#include "error.hxx"
#include "file.hxx"

extern "C" {
#include <was/simple.h>
}

#include <errno.h>
#include <sys/stat.h>

void
handle_mkcol(was_simple *w, const FileResource &resource)
{
	if (mkdir(resource.GetPath(), 0777) < 0) {
		const int e = errno;
		if (errno == ENOTDIR)
			was_simple_status(w, HTTP_STATUS_CONFLICT);
		else
			errno_response(w, e);
		return;
	}

	was_simple_status(w, HTTP_STATUS_CREATED);
}
