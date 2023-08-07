/*
 * Local file resources.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "file.hxx"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

FileResource::FileResource(std::string &&_path) noexcept
	:path(std::move(_path)), error(0)
{
	if (stat(path.c_str(), &st) < 0)
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
