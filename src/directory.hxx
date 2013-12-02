/*
 * Request handler for directories (e.g. MKCOL).
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_DIRECTORY_HXX
#define DAVOS_DIRECTORY_HXX

struct was_simple;

void
mkcol(was_simple *was, const char *path);

#endif
