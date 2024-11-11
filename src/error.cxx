// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * Error response generator.
 */

#include "error.hxx"

extern "C" {
#include <was/simple.h>
}

#include <errno.h>

http_status_t
errno_status(int e)
{
	switch (e) {
	case ENOENT:
	case ENOTDIR:
		return HTTP_STATUS_NOT_FOUND;

	case EACCES:
	case EPERM:
	case EROFS:
		return HTTP_STATUS_FORBIDDEN;

	case ENAMETOOLONG:
		return HTTP_STATUS_REQUEST_URI_TOO_LONG;

	case ENOSPC:
		return HTTP_STATUS_INSUFFICIENT_STORAGE;

	case ENOTEMPTY:
	case EBUSY:
		return HTTP_STATUS_FAILED_DEPENDENCY;

	case EINVAL:
		return HTTP_STATUS_BAD_REQUEST;

	case EEXIST:
	case EISDIR:
		return HTTP_STATUS_CONFLICT;

	default:
		return HTTP_STATUS_INTERNAL_SERVER_ERROR;
	}
}

void
errno_response(was_simple *was, int e)
{
	was_simple_status(was, errno_status(e));
}

void
errno_response(was_simple *was)
{
	errno_response(was, errno);
}
