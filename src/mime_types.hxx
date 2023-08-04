/*
 * Loader for /etc/mime.types.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#pragma once

[[gnu::pure]]
const char *
LookupMimeTypeByFileName(const char *name);

[[gnu::pure]]
const char *
LookupMimeTypeByFilePath(const char *path);
