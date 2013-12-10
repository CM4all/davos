/*
 * Request handlers for "other" commands.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_OTHER_HXX
#define DAVOS_OTHER_HXX

struct was_simple;

void
handle_delete(was_simple *was, const char *path);

void
handle_copy(was_simple *w, const char *path, const char *destination_path);

void
handle_move(was_simple *w, const char *path, const char *destination_path);

#endif
