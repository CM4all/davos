/*
 * WebDAV protocol utilities.
 *
 * author: Max Kellermann <mk@cm4all.com>
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
