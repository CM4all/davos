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

bool
wxml_cdata(Writer &w, const char *data)
{
	while (true) {
		const char *p = strpbrk(data, "<>&\"");
		if (p == nullptr)
			return w.Write(data);

		if (p > data) {
			if (!w.Write(data, p - data))
				return false;

			data = p;
		}

		if (!w.Write(wxml_escape_char(*data++)))
			return false;
	}
}
