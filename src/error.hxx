/*
 * Error response generator.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#pragma once

#include <http/status.h>

struct was_simple;

[[gnu::pure]]
http_status_t
errno_status(int e);

void
errno_response(was_simple *was, int e);

void
errno_response(was_simple *was);
