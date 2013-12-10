/*
 * Request handler for local files.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_GET_HXX
#define DAVOS_GET_HXX

struct was_simple;
class FileResource;

void
handle_get(was_simple *was, const FileResource &resource);

void
handle_head(was_simple *was, const FileResource &resource);

#endif
