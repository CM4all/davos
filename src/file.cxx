// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * Local file resources.
 */

#include "file.hxx"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

FileResource::FileResource(std::string &&_path) noexcept
	:path(std::move(_path)), error(0)
{
	if (statx(-1, path.c_str(),  AT_STATX_SYNC_AS_STAT,
		  STATX_TYPE|STATX_MTIME|STATX_SIZE,
		  &st) < 0)
		error = errno;
}

int
FileResource::CreateExclusive() const noexcept
{
	int fd = open(GetPath(), O_CREAT|O_EXCL|O_WRONLY|O_NOCTTY, 0666);
	if (fd < 0)
		return errno;

	close(fd);
	return 0;
}
