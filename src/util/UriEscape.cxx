// SPDX-License-Identifier: BSD-2-Clause
// Copyright Max Kellermann <max.kellermann@gmail.com>

#include "UriEscape.hxx"
#include "util/CharUtil.hxx"
#include "util/HexFormat.hxx"
#include "uri/Chars.hxx"
#include "uri/Unescape.hxx"
#include "LightString.hxx"

/**
 * @see RFC 3986 2.3
 */
static constexpr bool
IsUriPathUnreserved(char ch)
{
	return IsUriUnreservedChar(ch) || IsUriSubcomponentDelimiter(ch) ||
		ch == ':' || ch == '@' || ch == '/';
}

static constexpr size_t
CountEscapePath(const std::string_view s) noexcept
{
	size_t n = 0;
	for (const char ch : s)
		if (!IsUriPathUnreserved(ch))
			++n;
	return n;
}

static constexpr char *
UriEscapeByte(char *p, uint8_t value)
{
	*p++ = '%';
	p = HexFormatUint8Fixed(p, value);
	return p;
}

static constexpr char *
UriEscapePath(char *dest, const std::string_view src) noexcept
{
	for (const char ch : src) {
		if (IsUriPathUnreserved(ch))
			*dest++ = ch;
		else
			dest = UriEscapeByte(dest, ch);
	}

	return dest;
}

LightString
UriEscapePath(const std::string_view src)
{
	size_t n_escape = CountEscapePath(src);
	if (n_escape == 0)
		return LightString::Make(src);

	char *dest = new char[src.size() + n_escape * 2];
	char *end = UriEscapePath(dest, src);
	return LightString::Donate(dest, end - dest);
}

LightString
UriUnescape(const std::string_view src)
{
	if (src.find('%') == src.npos)
		/* no escape, no change required, return the existing
		   pointer without allocating a copy */
		return LightString::Make(src);

	/* worst-case allocation */
	char *dest = new char[src.size()];

	char *end = UriUnescape(dest, src);
	if (end == nullptr) {
		delete[] dest;
		return nullptr;
	}

	return LightString::Donate(dest, end - dest);
}
