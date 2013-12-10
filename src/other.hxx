/*
 * Request handlers for "other" commands.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_OTHER_HXX
#define DAVOS_OTHER_HXX

struct was_simple;
class FileResource;

void
handle_delete(was_simple *was, const FileResource &resource);

void
handle_copy(was_simple *w, const FileResource &src, const FileResource &dest);

void
handle_move(was_simple *w, const FileResource &src, const FileResource &dest);

#endif
