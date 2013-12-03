/*
 * Fake LOCK/UNLOCK implementation.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_LOCK_HXX
#define DAVOS_LOCK_HXX

struct was_simple;

void
handle_lock(was_simple *w, const char *path);

#endif
