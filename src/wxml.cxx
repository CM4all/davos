/*
 * XML writer.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "wxml.hxx"

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
wxml_cdata(was_simple *w, const char *data)
{
    while (true) {
        const char *p = strpbrk(data, "<>&\"");
        if (p == nullptr)
            return was_simple_puts(w, data);

        if (p > data) {
            if (!was_simple_write(w, data, p - data))
                return false;

            data = p;
        }

        if (!was_simple_puts(w, wxml_escape_char(*data++)))
            return false;
    }
}
