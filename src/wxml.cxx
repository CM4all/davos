// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * XML writer.
 */

#include "wxml.hxx"
#include "util/LightString.hxx"
#include "util/Compiler.h"

#include <string.h>

static const char *
wxml_escape_char(char ch)
{
	switch (ch) {
	case '<':
		return "&lt;";

	case '>':
		return "&gt;";

	case '&':
		return "&amp;";

	case '"':
		return "&quot;";
	}

	std::unreachable();
}

void
wxml_cdata(BufferedOutputStream &o, std::string_view data)
{
	while (true) {
		const auto p = data.find_first_of("<>&\"");
		if (p == data.npos) {
			o.Write(data);
			return;
		}

		if (p > 0) {
			o.Write(data.substr(0, p));
			data = data.substr(p);
		}

		o.Write(wxml_escape_char(data.front()));
		data = data.substr(1);
	}
}
