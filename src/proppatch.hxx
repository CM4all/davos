/*
 * PROPPATCH implementation.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_PROPPATCH_HXX
#define DAVOS_PROPPATCH_HXX

struct was_simple;
struct FileResource;

void
handle_proppatch(was_simple *w, const char *uri, const FileResource &resource);

#endif
