// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * WebDAV protocol utilities.
 */

#include "util.hxx"

extern "C" {
#include <was/simple.h>
}

#include <string.h>

bool
get_boolean_header(was_simple *w, const char *name,
		   bool default_value) noexcept
{
	const char *overwrite = was_simple_get_header(w, name);
	if (overwrite == nullptr)
		return default_value;

	return default_value
		? overwrite[0] != 'f' && overwrite[0] != 'F'
		: overwrite[0] == 't' || overwrite[0] == 'T';
}
