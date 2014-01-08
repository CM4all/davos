/*
 * XML writer.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

extern "C" {
#include <was/simple.h>
}

#include <inline/compiler.h>

#include <algorithm>

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

gcc_unused
static bool
wxml_short_element(was_simple *w, const char *name)
{
    return wxml_begin_tag(w, name) && wxml_end_short_tag(w);
}

gcc_nonnull_all
bool
wxml_cdata(was_simple *w, const char *data);

gcc_unused
static bool
wxml_string_element(was_simple *w, const char *name, const char *value)
{
    return wxml_open_element(w, name) &&
        wxml_cdata(w, value) &&
        wxml_close_element(w, name);
}

template<typename... Args>
static bool
wxml_format_element(was_simple *w, const char *name, const char *fmt,
                    Args... args)
{
    return wxml_open_element(w, name) &&
        was_simple_printf(w, fmt, std::forward<Args>(args)...) &&
        wxml_close_element(w, name);
}

static bool
wxml_attribute(was_simple *w, const char *name, const char *value)
{
    return was_simple_puts(w, " ") && was_simple_puts(w, name) &&
        was_simple_puts(w, "=\"") && wxml_cdata(w, value) &&
        was_simple_puts(w, "\"");
}

gcc_unused
static bool
begin_multistatus(was_simple *w)
{
    return wxml_declaration(w) &&
        wxml_begin_tag(w, "D:multistatus") &&
        wxml_attribute(w, "xmlns:D", "DAV:") &&
        wxml_end_tag(w);
}

gcc_unused
static bool
end_multistatus(was_simple *w)
{
    return wxml_close_element(w, "D:multistatus");
}

gcc_unused
static bool
href(was_simple *w, const char *uri)
{
    return wxml_string_element(w, "D:href", uri);
}

gcc_unused
static bool
resourcetype_collection(was_simple *w)
{
    return wxml_open_element(w, "D:resourcetype") &&
        wxml_short_element(w, "D:collection") &&
        wxml_close_element(w, "D:resourcetype");
}

gcc_unused
static bool
open_response_prop(was_simple *w, const char *uri, const char *status)
{
    return wxml_open_element(w, "D:response") &&
        href(w, uri) &&
        wxml_open_element(w, "D:propstat") &&
        wxml_string_element(w, "D:status", status) &&
        wxml_open_element(w, "D:prop");
}

gcc_unused
static bool
close_response_prop(was_simple *w)
{
    return wxml_close_element(w, "D:prop") &&
        wxml_close_element(w, "D:propstat") &&
        wxml_close_element(w, "D:response");
}
