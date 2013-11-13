/*
 * Error response generator.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_ERROR_HXX
#define DAVOS_ERROR_HXX

struct was_simple;

void
errno_respones(was_simple *was, int e);

void
errno_respones(was_simple *was);

#endif
