// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

#include "ETag.hxx"
#include "util/Base32.hxx"

#include <sys/stat.h>

StringBuffer<64>
MakeETag(const struct statx &st) noexcept
{
	StringBuffer<64> result;

	char *p = result.data();
	*p++ = '"';

	p = FormatIntBase32(p, st.stx_dev_major);
	p = FormatIntBase32(p, st.stx_dev_minor);

	*p++ = '-';

	p = FormatIntBase32(p, st.stx_ino);

	*p++ = '-';

	p = FormatIntBase32(p, st.stx_mtime.tv_sec);

	*p++ = '-';

	p = FormatIntBase32(p, st.stx_mtime.tv_nsec);

	*p++ = '"';
	*p = 0;

	return result;
}
