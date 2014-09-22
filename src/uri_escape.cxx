/*
 * Escape and unescape in URI style ('%20').
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "uri_escape.hxx"

#include <glib.h>

void
AppendUriEscape(std::string &dest, const char *src)
{
    char *escaped =
        g_uri_escape_string(src, G_URI_RESERVED_CHARS_ALLOWED_IN_PATH, true);
    if (escaped == nullptr)
        return;

    dest.append(escaped);
    g_free(escaped);
}
