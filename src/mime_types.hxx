/*
 * Loader for /etc/mime.types.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_MIME_TYPES_HXX
#define DAVOS_MIME_TYPES_HXX

#include "util/Compiler.h"

gcc_pure
const char *
LookupMimeTypeByFileName(const char *name);

gcc_pure
const char *
LookupMimeTypeByFilePath(const char *path);

#endif
