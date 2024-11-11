// SPDX-License-Identifier: BSD-2-Clause
// Copyright Max Kellermann <max.kellermann@gmail.com>

#include "UriEscape.hxx"
#include "util/CharUtil.hxx"
#include "util/HexFormat.hxx"
#include "uri/Chars.hxx"
#include "uri/Unescape.hxx"
#include "LightString.hxx"

#include <cstring>

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
