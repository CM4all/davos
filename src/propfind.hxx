/*
 * PROPFIND implementation.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_PROPFIND_HXX
#define DAVOS_PROPFIND_HXX

struct was_simple;

void
handle_propfind(was_simple *was, const char *uri, const char *path);

#endif
