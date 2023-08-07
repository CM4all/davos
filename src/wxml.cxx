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
wxml_cdata(BufferedOutputStream &o, const char *data)
{
	while (true) {
		const char *p = strpbrk(data, "<>&\"");
		if (p == nullptr) {
			o.Write(data);
			return;
		}

		if (p > data) {
			o.Write(std::string_view(data, p - data));
			data = p;
		}

		o.Write(wxml_escape_char(*data++));
	}
}
