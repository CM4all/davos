/*
 * PROPPATCH implementation.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "proppatch.hxx"
#include "wxml.hxx"
#include "expat.hxx"
#include "error.hxx"

extern "C" {
#include <was/simple.h>
}

#include <string.h>
#include <sys/time.h>

static void XMLCALL
start_element(void *userData, const XML_Char *name,
              gcc_unused const XML_Char **atts)
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

static bool
ns_short_element(was_simple *w, const char *name)
{
    const char *pipe = strchr(name, '|');
    if (pipe == nullptr)
        return false;

    const std::string ns(name, pipe);
    name = pipe + 1;

    return was_simple_puts(w, "<") && was_simple_puts(w, "X:") &&
        was_simple_puts(w, name) &&
        wxml_attribute(w, "xmlns:X", ns.c_str()) &&
        wxml_end_short_tag(w);
}

static bool
propstat(was_simple *w, const char *name, const char *status)
{
    return wxml_open_element(w, "D:propstat") &&
        wxml_open_element(w, "D:prop") &&
        ns_short_element(w, name) &&
        wxml_close_element(w, "D:prop") &&
        wxml_open_element(w, "D:status") &&
        was_simple_puts(w, status) &&
        wxml_close_element(w, "D:status") &&
        wxml_close_element(w, "D:propstat");
}

gcc_pure
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
ProppatchMethod::SendResponse(was_simple *w, const char *uri)
{
    if (!was_simple_status(w, HTTP_STATUS_MULTI_STATUS) ||
        !was_simple_set_header(w, "content-type",
                               "text/xml; charset=\"utf-8\"") ||
        !begin_multistatus(w) ||
        !wxml_open_element(w, "D:response") ||
        !href(w, uri))
        return false;

    for (auto prop : data.props)
        if (!propstat(w, prop.name.c_str(),
                      http_status_to_string(prop.status)))
            return false;

    return wxml_close_element(w, "D:response") && end_multistatus(w);
}
