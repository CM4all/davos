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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

static bool
begin_prop(was_simple *w)
{
    return wxml_declaration(w) &&
        wxml_begin_tag(w, "D:prop") &&
        wxml_attribute(w, "xmlns:D", "DAV:") &&
        wxml_end_tag(w);
}

static bool
end_prop(was_simple *w)
{
    return wxml_close_element(w, "D:prop");
}

static bool
href(was_simple *w, const char *uri)
{
    return wxml_string_element(w, "D:href", uri);
}

static bool
locktoken_href(was_simple *w, const char *token)
{
    return wxml_open_element(w, "D:locktoken") &&
        href(w, token) &&
        wxml_close_element(w, "D:locktoken");
}

static bool
owner_href(was_simple *w, const char *token)
{
    return wxml_open_element(w, "D:owner") &&
        href(w, token) &&
        wxml_close_element(w, "D:owner");
}

struct LockParserData {
    enum State {
        ROOT,
        OWNER,
        OWNER_HREF,
    } state;

    std::string owner_href;

    LockParserData():state(ROOT) {}
};

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

void
handle_lock(was_simple *w, const char *path)
{
    if (was_simple_get_header(w, "if") != nullptr)
        /* lock refresh, no XML request body */
        return;

    LockParserData data;

    {
        ExpatParser expat(&data);
        expat.SetElementHandler(start_element, end_element);
        expat.SetCharacterDataHandler(char_data);
        if (!expat.Parse(w)) {
            was_simple_status(w, HTTP_STATUS_BAD_REQUEST);
            return;
        }
    }

    http_status_t status = HTTP_STATUS_OK;

    struct stat st;
    if (stat(path, &st) < 0) {
        int e = errno;
        if (e == ENOENT) {
            /* RFC4918 9.10.4: "A successful LOCK method MUST result
               in the creation of an empty resource that is locked
               (and that is not a collection) when a resource did not
               previously exist at that URL". */
            int fd = open(path, O_CREAT|O_EXCL|O_WRONLY|O_NOCTTY, 0666);
            if (fd < 0) {
                e = errno;
                if (e == EEXIST || e == EISDIR) {
                    /* the file/directory has been created by somebody
                       else meanwhile */
                } else if (e == ENOENT || e == ENOTDIR) {
                    was_simple_status(w, HTTP_STATUS_CONFLICT);
                    return;
                } else {
                    errno_respones(w, e);
                    return;
                }
            } else {
                close(fd);
                status = HTTP_STATUS_CREATED;
            }
        } else if (e == ENOTDIR) {
            was_simple_status(w, HTTP_STATUS_CONFLICT);
            return;
        } else {
            errno_respones(w, e);
            return;
        }
    }

    const char *token = "opaquelocktoken:dummy";
    const char *token2 = "<opaquelocktoken:dummy>";

    was_simple_status(w, status) &&
        was_simple_set_header(w, "content-type",
                              "text/xml; charset=\"utf-8\"") &&
        was_simple_set_header(w, "lock-token", token2) &&
        begin_prop(w) &&
        wxml_open_element(w, "D:lockdiscovery") &&
        wxml_open_element(w, "D:activelock") &&

        wxml_open_element(w, "D:locktype") &&
        wxml_short_element(w, "D:write") &&
        wxml_close_element(w, "D:locktype") &&

        wxml_open_element(w, "D:lockscope") &&
        wxml_short_element(w, "D:exclusive") &&
        wxml_close_element(w, "D:lockscope") &&

        wxml_string_element(w, "D:depth", "infinity") &&

        locktoken_href(w, token) &&
        (data.owner_href.empty() || owner_href(w, data.owner_href.c_str())) &&
        wxml_close_element(w, "D:activelock") &&
        wxml_close_element(w, "D:lockdiscovery") &&
        end_prop(w);
}
