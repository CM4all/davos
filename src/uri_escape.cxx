/*
 * Escape and unescape in URI style ('%20').
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "uri_escape.hxx"
#include "util/UriEscape.hxx"
#include "util/LightString.hxx"

void
AppendUriEscape(std::string &dest, const char *src)
{
    const auto escaped = UriEscapePath(src);
    dest.append(escaped.c_str());
}
