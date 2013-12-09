/*
 * Request handler for PUT.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_PUT_HXX
#define DAVOS_PUT_HXX

struct was_simple;

void
handle_put(was_simple *was, const char *path);

#endif
