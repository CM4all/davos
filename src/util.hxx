// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * WebDAV protocol utilities.
 */

#pragma once

struct was_simple;

[[gnu::pure]]
bool
get_boolean_header(was_simple *w, const char *name,
		   bool default_value) noexcept;

[[gnu::pure]]
static inline bool
get_overwrite_header(was_simple *w) noexcept
{
	return get_boolean_header(w, "overwrite", true);
}
