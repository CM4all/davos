/*
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "ETag.hxx"
#include "util/HexFormat.h"

StringBuffer<32>
MakeETag(const struct stat &st) noexcept
{
    StringBuffer<32> result;

    char *p = result.data();
    *p++ = '"';

    p += format_uint32_hex(p, (uint32_t)st.st_dev);

    *p++ = '-';

    p += format_uint32_hex(p, (uint32_t)st.st_ino);

    *p++ = '-';

    p += format_uint32_hex(p, (uint32_t)st.st_mtime);

    *p++ = '"';
    *p = 0;

    return result;
}
