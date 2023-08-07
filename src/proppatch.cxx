/*
 * PROPPATCH implementation.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "proppatch.hxx"
#include "wxml.hxx"
#include "expat.hxx"
#include "error.hxx"
#include "was/WasOutputStream.hxx"
#include "util/StringSplit.hxx"

extern "C" {
#include <was/simple.h>
}

#include <string.h>
#include <time.h>

static void XMLCALL
start_element(void *userData, const XML_Char *name,
	      [[maybe_unused]] const XML_Char **atts)
{
	ProppatchParserData &data = *(ProppatchParserData *)userData;

	switch (data.state) {
	case ProppatchParserData::ROOT:
		if (strcmp(name, "DAV:|prop") == 0)
			data.state = ProppatchParserData::PROP;
		break;

	case ProppatchParserData::PROP:
		data.state = ProppatchParserData::PROP_NAME;
		data.props.emplace_back(name);
		break;

	case ProppatchParserData::PROP_NAME:
		break;
	}
}

static void XMLCALL
end_element(void *userData, const XML_Char *name)
{
	ProppatchParserData &data = *(ProppatchParserData *)userData;

	switch (data.state) {
	case ProppatchParserData::ROOT:
		break;

	case ProppatchParserData::PROP:
		if (strcmp(name, "DAV:|prop") == 0)
			data.state = ProppatchParserData::ROOT;
		break;

	case ProppatchParserData::PROP_NAME:
		if (name == data.props.back().name)
			data.state = ProppatchParserData::PROP;
		break;
	}
}

static void XMLCALL
char_data(void *userData, const XML_Char *s, int len)
{
	ProppatchParserData &data = *(ProppatchParserData *)userData;

	switch (data.state) {
	case ProppatchParserData::ROOT:
	case ProppatchParserData::PROP:
		break;

	case ProppatchParserData::PROP_NAME:
		data.props.back().value.assign(s, len);
		break;
	}
}

static void
ns_short_element(BufferedOutputStream &o, std::string_view name)
{
	const auto [ns, rest] = Split(name, '|');
	if (rest.data() == nullptr)
		// TODO what now? is this a bug or bad user input?
		return;

	o.Write('<');
	o.Write("X:");
	o.Write(rest);
	wxml_attribute(o, "xmlns:X", ns);
	wxml_end_short_tag(o);
}

static void
propstat(BufferedOutputStream &o, std::string_view name, std::string_view status)
{
	wxml_open_element(o, "D:propstat");
	wxml_open_element(o, "D:prop");
	ns_short_element(o, name);
	wxml_close_element(o, "D:prop");
	wxml_open_element(o, "D:status");
	o.Write(status);
	wxml_close_element(o, "D:status");
	wxml_close_element(o, "D:propstat");
}

[[gnu::pure]]
static bool
parse_win32_timestamp(const char *s, struct timeval &tv)
{
	struct tm tm;
	if (strptime(s, "%a, %d %b %Y %T %Z", &tm) == nullptr)
		return false;

	tv.tv_sec = timegm(&tm);
	tv.tv_usec = 0;
	return true;
}

bool
PropNameValue::ParseWin32Timestamp(timeval &tv) const
{
	return parse_win32_timestamp(value.c_str(), tv);
}

bool
ProppatchMethod::ParseRequest(was_simple *w)
{
	ExpatParser expat(&data);
	expat.SetElementHandler(start_element, end_element);
	expat.SetCharacterDataHandler(char_data);
	if (!expat.Parse(w)) {
		was_simple_status(w, HTTP_STATUS_BAD_REQUEST);
		return false;
	}

	return true;
}

bool
ProppatchMethod::SendResponse(was_simple *w, std::string_view uri)
{
	if (!was_simple_status(w, HTTP_STATUS_MULTI_STATUS) ||
	    !was_simple_set_header(w, "content-type",
				   "text/xml; charset=\"utf-8\""))
		return false;

	WasOutputStream wos{w};
	BufferedOutputStream bos{wos};

	begin_multistatus(bos);
	wxml_open_element(bos, "D:response");
	href(bos, uri);

	for (auto prop : data.props)
		propstat(bos, prop.name,
			 http_status_to_string(prop.status));

	wxml_close_element(bos, "D:response");
	end_multistatus(bos);

	bos.Flush();
	return true;
}
