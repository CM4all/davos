/*
 * XML writer.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "wxml.hxx"
#include "util/LightString.hxx"
#include "util/Compiler.h"

#include <assert.h>
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

	assert(false);
	gcc_unreachable();
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
