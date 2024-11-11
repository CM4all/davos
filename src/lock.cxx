// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * Fake LOCK/UNLOCK implementation.
 */

#include "lock.hxx"
#include "wxml.hxx"
#include "error.hxx"
#include "expat.hxx"
#include "was/WasOutputStream.hxx"

#include <was/simple.h>

#include <string>
#include <forward_list>

#include <string.h>

static void
begin_prop(BufferedOutputStream &o)
{
	wxml_declaration(o);
	wxml_begin_tag(o, "D:prop");
	wxml_attribute(o, "xmlns:D", "DAV:");
	wxml_end_tag(o);
}

static void
end_prop(BufferedOutputStream &o)
{
	wxml_close_element(o, "D:prop");
}

static void
locktoken_href(BufferedOutputStream &o, std::string_view token)
{
	wxml_open_element(o, "D:locktoken");
	href(o, token);
	wxml_close_element(o, "D:locktoken");
}

static void
owner_href(BufferedOutputStream &o, std::string_view token)
{
	wxml_open_element(o, "D:owner");
	href(o, token);
	wxml_close_element(o, "D:owner");
}

static void XMLCALL
start_element(void *userData, const XML_Char *name,
	      [[maybe_unused]] const XML_Char **atts)
{
	LockParserData &data = *(LockParserData *)userData;

	switch (data.state) {
	case LockParserData::ROOT:
		if (strcmp(name, "DAV:|owner") == 0)
			data.state = LockParserData::OWNER;
		break;

	case LockParserData::OWNER:
		if (strcmp(name, "DAV:|href") == 0)
			data.state = LockParserData::OWNER_HREF;
		break;

	case LockParserData::OWNER_HREF:
		break;
	}
}

static void XMLCALL
end_element(void *userData, const XML_Char *name)
{
	LockParserData &data = *(LockParserData *)userData;

	switch (data.state) {
	case LockParserData::ROOT:
		break;

	case LockParserData::OWNER:
		if (strcmp(name, "DAV:|owner") == 0)
			data.state = LockParserData::ROOT;
		break;

	case LockParserData::OWNER_HREF:
		if (strcmp(name, "DAV:|href") == 0)
			data.state = LockParserData::OWNER;
		break;
	}
}

static void XMLCALL
char_data(void *userData, const XML_Char *s, int len)
{
	LockParserData &data = *(LockParserData *)userData;

	switch (data.state) {
	case LockParserData::ROOT:
	case LockParserData::OWNER:
		break;

	case LockParserData::OWNER_HREF:
		data.owner_href.assign(s, len);
		break;
	}
}

bool
LockMethod::ParseRequest(was_simple *w)
{
	if (was_simple_get_header(w, "if") != nullptr)
		/* lock refresh, no XML request body */
		return false;

	ExpatParser expat(&data);
	expat.SetElementHandler(start_element, end_element);
	expat.SetCharacterDataHandler(char_data);
	if (!expat.Parse(w)) {
		was_simple_status(w, HTTP_STATUS_BAD_REQUEST);
		return false;
	}

	return true;
}

void
LockMethod::Run(was_simple *w, bool created)
{
	http_status_t status = created ? HTTP_STATUS_CREATED : HTTP_STATUS_OK;

	const char *token = "opaquelocktoken:dummy";
	const char *token2 = "<opaquelocktoken:dummy>";

	if (!was_simple_status(w, status) ||
	    !was_simple_set_header(w, "content-type",
				   "text/xml; charset=\"utf-8\"") ||
	    !was_simple_set_header(w, "lock-token", token2))
		return;

	WasOutputStream wos{w};
	BufferedOutputStream bos{wos};

	begin_prop(bos);
	wxml_open_element(bos, "D:lockdiscovery");
	wxml_open_element(bos, "D:activelock");

	wxml_open_element(bos, "D:locktype");
	wxml_short_element(bos, "D:write");
	wxml_close_element(bos, "D:locktype");

	wxml_open_element(bos, "D:lockscope");
	wxml_short_element(bos, "D:exclusive");
	wxml_close_element(bos, "D:lockscope");

	wxml_string_element(bos, "D:depth", "infinity");

	locktoken_href(bos, token);
	if (!data.owner_href.empty())
		owner_href(bos, data.owner_href);
	wxml_close_element(bos, "D:activelock");
	wxml_close_element(bos, "D:lockdiscovery");
	end_prop(bos);

	bos.Flush();
}
