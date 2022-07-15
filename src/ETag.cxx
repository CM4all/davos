/*
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "ETag.hxx"
#include "util/Base32.hxx"

StringBuffer<32>
MakeETag(const struct stat &st) noexcept
{
	StringBuffer<32> result;

	char *p = result.data();
	*p++ = '"';

	p = FormatIntBase32(p, st.st_dev);

	*p++ = '-';

	p = FormatIntBase32(p, st.st_ino);

	*p++ = '-';

	p = FormatIntBase32(p, st.st_mtime);

	*p++ = '"';
	*p = 0;

	return result;
}
