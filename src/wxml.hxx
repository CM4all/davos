/*
 * XML writer.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

extern "C" {
#include <was/simple.h>
}

static bool
wxml_declaration(was_simple *w)
{
    return was_simple_puts(w, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
}

static bool
wxml_begin_tag(was_simple *w, const char *name)
{
    return was_simple_puts(w, "<") && was_simple_puts(w, name);
}

static bool
wxml_end_tag(was_simple *w)
{
    return was_simple_puts(w, ">");
}

static bool
wxml_end_short_tag(was_simple *w)
{
    return was_simple_puts(w, "/>");
}

static bool
wxml_open_element(was_simple *w, const char *name)
{
    return wxml_begin_tag(w, name) && wxml_end_tag(w);
}

static bool
wxml_close_element(was_simple *w, const char *name)
{
    return was_simple_puts(w, "</") && was_simple_puts(w, name) &&
        wxml_end_tag(w);
}

static bool
wxml_short_element(was_simple *w, const char *name)
{
    return wxml_begin_tag(w, name) && wxml_end_short_tag(w);
}

static bool
wxml_string_element(was_simple *w, const char *name, const char *value)
{
    return wxml_open_element(w, name) &&
        was_simple_puts(w, value) &&
        wxml_close_element(w, name);
}

static bool
wxml_attribute(was_simple *w, const char *name, const char *value)
{
    return was_simple_puts(w, " ") && was_simple_puts(w, name) &&
        was_simple_puts(w, "=\"") && was_simple_puts(w, value) &&
        was_simple_puts(w, "\"");
}
