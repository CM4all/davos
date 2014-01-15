/*
 * XML writer.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "wxml.hxx"

#include <glib.h> // TODO: eliminate GLib

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

bool
wxml_uri_escape(was_simple *w, const char *uri)
{
    char *escaped = g_uri_escape_string(uri, G_URI_RESERVED_CHARS_ALLOWED_IN_PATH, true);
    if (escaped == nullptr)
        return false;

    bool success = wxml_cdata(w, escaped);
    g_free(escaped);
    return success;
}
