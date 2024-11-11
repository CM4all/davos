// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * Error response generator.
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
