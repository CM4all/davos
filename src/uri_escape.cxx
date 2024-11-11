// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * Escape and unescape in URI style ('%20').
 */

#include "uri_escape.hxx"
#include "util/UriEscape.hxx"
#include "util/LightString.hxx"

void
AppendUriEscape(std::string &dest, const char *src)
{
	const auto escaped = UriEscapePath(src);
	dest.append(escaped.c_str());
}
