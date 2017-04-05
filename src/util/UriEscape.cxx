/*
 * Copyright (C) 2015 Max Kellermann <max@duempel.org>
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
#include "LightString.hxx"

#include <algorithm>

#include <stdint.h>
#include <string.h>

/**
 * @see RFC 3986 2.3
 */
constexpr
static inline bool
IsUriUnreserved(char ch)
{
	return IsAlphaNumericASCII(ch) ||
		ch == '-' || ch == '.' || ch == '_' || ch == '~';
}

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
	return IsUriUnreserved(ch) || IsUriSubcomponentDelimiter(ch) ||
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
FormatHexByte(char *p, uint8_t value)
{
	static constexpr char hex_digits[] = "0123456789abcdef";

	*p++ = hex_digits[value >> 4];
	*p++ = hex_digits[value & 0xf];
	return p;
}

static char *
UriEscapeByte(char *p, uint8_t value)
{
	*p++ = '%';
	p = FormatHexByte(p, value);
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

static int
ParseHexDigit(char ch)
{
	if (IsDigitASCII(ch))
		return ch - '0';
	else if (ch >= 'a' && ch <= 'f')
		return ch - 'a' + 0xa;
	else if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 0xa;
	else
		return -1;
}

static char *
UriUnescape(char *dest, const char *src, const char *const end)
{
	while (true) {
		const char *p = std::find(src, end, '%');
		dest = std::copy(src, p, dest);
		if (p == end)
			return dest;

		src = p;
		if (src + 2 >= end)
			/* percent sign at the end of string */
			return nullptr;

		const int digit1 = ParseHexDigit(src[1]);
		const int digit2 = ParseHexDigit(src[2]);
		if (digit1 == -1 || digit2 == -1)
			/* invalid hex digits */
			return nullptr;

		const char ch = (char)((digit1 << 4) | digit2);
		if (ch == 0)
			/* no %00 hack allowed! */
			return nullptr;

		*dest++ = ch;
		src += 3;
	}

	dest = std::copy(src, end, dest);
	return dest;
}

LightString
UriUnescape(const char *src)
{
	const size_t length = strlen(src);
	if (memchr(src, '%', length) == nullptr)
		/* no escape, no change required, return the existing
		   pointer without allocating a copy */
		return LightString::Make(src);

	/* worst-case allocation */
	char *dest = new char[length + 1];

	char *end = UriUnescape(dest, src, src + length);
	if (end == nullptr) {
		delete[] dest;
		return nullptr;
	}

	*end = 0;

	return LightString::Donate(dest);
}
