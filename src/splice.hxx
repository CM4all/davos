/*
 * Wrapper for libwas and splice().
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_SPLICE_HXX
#define DAVOS_SPLICE_HXX

struct was_simple;
class FileResource;

bool
splice_from_was(was_simple *w, int out_fd);

#endif
