/*
 * Copyright 2015-2022 Max Kellermann <max@duempel.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "UriEscape.hxx"
#include "util/CharUtil.hxx"
#include "util/HexFormat.hxx"
#include "uri/Chars.hxx"
#include "uri/Unescape.hxx"
#include "LightString.hxx"

#include <cstring>

/**
 * @see RFC 3986 2.2
 */
constexpr
static inline bool
IsUriSubcomponentDelimiter(char ch)
{
	return ch == '!' || ch == '$' || ch == '&' || ch == '\'' ||
		ch == '(' || ch == ')' ||
		ch == '*' || ch == '+' ||
		ch == ',' || ch == ';' || ch == '=';
}

/**
 * @see RFC 3986 2.3
 */
constexpr
static inline bool
IsUriPathUnreserved(char ch)
{
	return IsUriUnreservedChar(ch) || IsUriSubcomponentDelimiter(ch) ||
		ch == ':' || ch == '@' || ch == '/';
}

static size_t
CountEscapePath(const char *p)
{
	size_t n = 0;
	while (*p)
		if (!IsUriPathUnreserved(*p++))
			++n;
	return n;
}

static char *
UriEscapeByte(char *p, uint8_t value)
{
	*p++ = '%';
	p = HexFormatUint8Fixed(p, value);
	return p;
}

static char *
UriEscapePath(char *dest, const char *src)
{

	while (*src) {
		const char ch = *src++;

		if (IsUriPathUnreserved(ch))
			*dest++ = ch;
		else
			dest = UriEscapeByte(dest, ch);
	}

	return dest;
}

LightString
UriEscapePath(const char *src)
{
	size_t n_escape = CountEscapePath(src);
	if (n_escape == 0)
		return LightString::Make(src);

	char *dest = new char[strlen(src) + n_escape * 2 + 1];
	*UriEscapePath(dest, src) = 0;
	return LightString::Donate(dest);
}

LightString
UriUnescape(const char *_src)
{
	const std::string_view src{_src};
	if (src.find('%') == src.npos)
		/* no escape, no change required, return the existing
		   pointer without allocating a copy */
		return LightString::Make(_src);

	/* worst-case allocation */
	char *dest = new char[src.size() + 1];

	char *end = UriUnescape(dest, src);
	if (end == nullptr) {
		delete[] dest;
		return nullptr;
	}

	*end = 0;

	return LightString::Donate(dest);
}
