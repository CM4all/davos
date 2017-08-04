/*
 * XML writer.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "writer.hxx"
#include "util/Compiler.h"

#include <algorithm>

static bool
wxml_declaration(Writer &w)
{
    return w.Write("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
}

static bool
wxml_begin_tag(Writer &w, const char *name)
{
    return w.Write("<") && w.Write(name);
}

static bool
wxml_end_tag(Writer &w)
{
    return w.Write(">");
}

static bool
wxml_end_short_tag(Writer &w)
{
    return w.Write("/>");
}

static bool
wxml_open_element(Writer &w, const char *name)
{
    return wxml_begin_tag(w, name) && wxml_end_tag(w);
}

static bool
wxml_close_element(Writer &w, const char *name)
{
    return w.Write("</") && w.Write(name) &&
        wxml_end_tag(w);
}

gcc_unused
static bool
wxml_short_element(Writer &w, const char *name)
{
    return wxml_begin_tag(w, name) && wxml_end_short_tag(w);
}

gcc_nonnull_all
bool
wxml_cdata(Writer &w, const char *data);

gcc_nonnull_all
bool
wxml_uri_escape(Writer &w, const char *uri);

gcc_unused
static bool
wxml_string_element(Writer &w, const char *name, const char *value)
{
    return wxml_open_element(w, name) &&
        wxml_cdata(w, value) &&
        wxml_close_element(w, name);
}

gcc_unused
static bool
wxml_uri_element(Writer &w, const char *name, const char *value)
{
    return wxml_open_element(w, name) &&
        wxml_uri_escape(w, value) &&
        wxml_close_element(w, name);
}

template<typename... Args>
static bool
wxml_format_element(Writer &w, const char *name, const char *fmt,
                    Args... args)
{
    return wxml_open_element(w, name) &&
        w.Format(fmt, std::forward<Args>(args)...) &&
        wxml_close_element(w, name);
}

static bool
wxml_attribute(Writer &w, const char *name, const char *value)
{
    return w.Write(" ") && w.Write(name) &&
        w.Write("=\"") && wxml_cdata(w, value) &&
        w.Write("\"");
}

gcc_unused
static bool
begin_multistatus(Writer &w)
{
    return wxml_declaration(w) &&
        wxml_begin_tag(w, "D:multistatus") &&
        wxml_attribute(w, "xmlns:D", "DAV:") &&
        wxml_end_tag(w);
}

gcc_unused
static bool
end_multistatus(Writer &w)
{
    return wxml_close_element(w, "D:multistatus");
}

/**
 * @param uri an escaped URI
 */
gcc_unused
static bool
href(Writer &w, const char *uri)
{
    return wxml_string_element(w, "D:href", uri);
}

gcc_unused
static bool
resourcetype_collection(Writer &w)
{
    return wxml_open_element(w, "D:resourcetype") &&
        wxml_short_element(w, "D:collection") &&
        wxml_close_element(w, "D:resourcetype");
}

gcc_unused
static bool
open_response_prop(Writer &w, const char *uri, const char *status)
{
    return wxml_open_element(w, "D:response") &&
        href(w, uri) &&
        wxml_open_element(w, "D:propstat") &&
        wxml_string_element(w, "D:status", status) &&
        wxml_open_element(w, "D:prop");
}

gcc_unused
static bool
close_response_prop(Writer &w)
{
    return wxml_close_element(w, "D:prop") &&
        wxml_close_element(w, "D:propstat") &&
        wxml_close_element(w, "D:response");
}
