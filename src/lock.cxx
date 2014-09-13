/*
 * Fake LOCK/UNLOCK implementation.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "lock.hxx"
#include "wxml.hxx"
#include "error.hxx"
#include "expat.hxx"

extern "C" {
#include "date.h"

#include <was/simple.h>
}

#include <inline/compiler.h>

#include <string>
#include <forward_list>

#include <string.h>

static bool
begin_prop(Writer &w)
{
    return wxml_declaration(w) &&
        wxml_begin_tag(w, "D:prop") &&
        wxml_attribute(w, "xmlns:D", "DAV:") &&
        wxml_end_tag(w);
}

static bool
end_prop(Writer &w)
{
    return wxml_close_element(w, "D:prop");
}

static bool
locktoken_href(Writer &w, const char *token)
{
    return wxml_open_element(w, "D:locktoken") &&
        href(w, token) &&
        wxml_close_element(w, "D:locktoken");
}

static bool
owner_href(Writer &w, const char *token)
{
    return wxml_open_element(w, "D:owner") &&
        href(w, token) &&
        wxml_close_element(w, "D:owner");
}

static void XMLCALL
start_element(void *userData, const XML_Char *name,
              gcc_unused const XML_Char **atts)
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

    Writer writer(w);
    begin_prop(writer) &&
        wxml_open_element(writer, "D:lockdiscovery") &&
        wxml_open_element(writer, "D:activelock") &&

        wxml_open_element(writer, "D:locktype") &&
        wxml_short_element(writer, "D:write") &&
        wxml_close_element(writer, "D:locktype") &&

        wxml_open_element(writer, "D:lockscope") &&
        wxml_short_element(writer, "D:exclusive") &&
        wxml_close_element(writer, "D:lockscope") &&

        wxml_string_element(writer, "D:depth", "infinity") &&

        locktoken_href(writer, token) &&
        (data.owner_href.empty() ||
         owner_href(writer, data.owner_href.c_str())) &&
        wxml_close_element(writer, "D:activelock") &&
        wxml_close_element(writer, "D:lockdiscovery") &&
        end_prop(writer);
}
