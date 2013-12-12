/*
 * Local file resources.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "file.hxx"

#include <errno.h>

FileResource::FileResource(std::string &&_path)
    :path(std::move(_path)), error(0)
{
    if (stat(path.c_str(), &st) < 0)
        error = errno;
}
