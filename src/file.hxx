/*
 * Request handler for local files.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_FILE_HXX
#define DAVOS_FILE_HXX

struct was_simple;

void
get(was_simple *was, const char *path);

void
head(was_simple *was, const char *path);

#endif
