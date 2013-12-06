/*
 * Error response generator.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_ERROR_HXX
#define DAVOS_ERROR_HXX

#include <http/status.h>
#include <inline/compiler.h>

struct was_simple;

gcc_pure
http_status_t
errno_status(int e);

void
errno_respones(was_simple *was, int e);

void
errno_respones(was_simple *was);

#endif
