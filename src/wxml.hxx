/*
 * XML writer.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "writer.hxx"

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

inline bool
wxml_short_element(Writer &w, const char *name)
{
	return wxml_begin_tag(w, name) && wxml_end_short_tag(w);
}

[[gnu::nonnull]]
bool
wxml_cdata(Writer &w, const char *data);

inline bool
wxml_string_element(Writer &w, const char *name, const char *value)
{
	return wxml_open_element(w, name) &&
		wxml_cdata(w, value) &&
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

inline bool
begin_multistatus(Writer &w)
{
	return wxml_declaration(w) &&
		wxml_begin_tag(w, "D:multistatus") &&
		wxml_attribute(w, "xmlns:D", "DAV:") &&
		wxml_end_tag(w);
}

inline bool
end_multistatus(Writer &w)
{
	return wxml_close_element(w, "D:multistatus");
}

/**
 * @param uri an escaped URI
 */
inline bool
href(Writer &w, const char *uri)
{
	return wxml_string_element(w, "D:href", uri);
}

inline bool
resourcetype_collection(Writer &w)
{
	return wxml_open_element(w, "D:resourcetype") &&
		wxml_short_element(w, "D:collection") &&
		wxml_close_element(w, "D:resourcetype");
}

inline bool
open_response_prop(Writer &w, const char *uri, const char *status)
{
	return wxml_open_element(w, "D:response") &&
		href(w, uri) &&
		wxml_open_element(w, "D:propstat") &&
		wxml_string_element(w, "D:status", status) &&
		wxml_open_element(w, "D:prop");
}

inline bool
close_response_prop(Writer &w)
{
	return wxml_close_element(w, "D:prop") &&
		wxml_close_element(w, "D:propstat") &&
		wxml_close_element(w, "D:response");
}
