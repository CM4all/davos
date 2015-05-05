/*
 * Escape and unescape in URI style ('%20').
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_URI_ESCAPE_HXX
#define DAVOS_URI_ESCAPE_HXX

#include <inline/compiler.h>

#include <string>

void
AppendUriEscape(std::string &dest, const char *src);

#endif
