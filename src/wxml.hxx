/*
 * XML writer.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "io/BufferedOutputStream.hxx"

static void
wxml_declaration(BufferedOutputStream &o)
{
	o.Write("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
}

static void
wxml_begin_tag(BufferedOutputStream &o, std::string_view name)
{
	o.Write('<');
	o.Write(name);
}

static void
wxml_end_tag(BufferedOutputStream &o)
{
	o.Write('>');
}

static void
wxml_end_short_tag(BufferedOutputStream &o)
{
	o.Write("/>");
}

static void
wxml_open_element(BufferedOutputStream &o, std::string_view name)
{
	wxml_begin_tag(o, name);
	wxml_end_tag(o);
}

static void
wxml_close_element(BufferedOutputStream &o, std::string_view name)
{
	o.Write("</");
	o.Write(name);
	wxml_end_tag(o);
}

inline void
wxml_short_element(BufferedOutputStream &o, std::string_view name)
{
	wxml_begin_tag(o, name);
	wxml_end_short_tag(o);
}

void
wxml_cdata(BufferedOutputStream &o, std::string_view data);

inline void
wxml_string_element(BufferedOutputStream &o, std::string_view name,
		    std::string_view value)
{
	wxml_open_element(o, name);
	wxml_cdata(o, value);
	wxml_close_element(o, name);
}

template<typename S, typename... Args>
static void
wxml_fmt_element(BufferedOutputStream &o, std::string_view name,
		 const S &fmt, Args&&... args)
{
	wxml_open_element(o, name);
	o.Fmt(fmt, std::forward<Args>(args)...);
	wxml_close_element(o, name);
}

static void
wxml_attribute(BufferedOutputStream &o, std::string_view name, std::string_view value)
{
	o.Write(' ');
	o.Write(name);
	o.Write("=\"");
	wxml_cdata(o, value);
	o.Write("\"");
}

inline void
begin_multistatus(BufferedOutputStream &o)
{
	wxml_declaration(o);
	wxml_begin_tag(o, "D:multistatus");
	wxml_attribute(o, "xmlns:D", "DAV:");
	wxml_end_tag(o);
}

inline void
end_multistatus(BufferedOutputStream &o)
{
	wxml_close_element(o, "D:multistatus");
}

/**
 * @param uri an escaped URI
 */
inline void
href(BufferedOutputStream &o, std::string_view uri)
{
	wxml_string_element(o, "D:href", uri);
}

inline void
resourcetype_collection(BufferedOutputStream &o)
{
	wxml_open_element(o, "D:resourcetype");
	wxml_short_element(o, "D:collection");
	wxml_close_element(o, "D:resourcetype");
}

inline void
open_response_prop(BufferedOutputStream &o, std::string_view uri, std::string_view status)
{
	wxml_open_element(o, "D:response");
	href(o, uri);
	wxml_open_element(o, "D:propstat");
	wxml_string_element(o, "D:status", status);
	wxml_open_element(o, "D:prop");
}

inline void
close_response_prop(BufferedOutputStream &o)
{
	wxml_close_element(o, "D:prop");
	wxml_close_element(o, "D:propstat");
	wxml_close_element(o, "D:response");
}
