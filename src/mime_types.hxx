/*
 * Loader for /etc/mime.types.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_MIME_TYPES_HXX
#define DAVOS_MIME_TYPES_HXX

#include <inline/compiler.h>

#include <string>

gcc_pure
std::string
LookupMimeTypeByFileName(const char *name);

#endif
