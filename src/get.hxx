/*
 * Request handler for local files.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_GET_HXX
#define DAVOS_GET_HXX

struct was_simple;

void
handle_get(was_simple *was, const char *path);

void
handle_head(was_simple *was, const char *path);

#endif
