/*
 * WebDAV protocol utilities.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "util.hxx"

extern "C" {
#include <was/simple.h>
}

#include <string.h>

gcc_pure
bool
get_boolean_header(was_simple *w, const char *name, bool default_value)
{
    const char *overwrite = was_simple_get_header(w, name);
    if (overwrite == nullptr)
        return default_value;

    return default_value
        ? overwrite[0] != 'f' && overwrite[0] != 'F'
        : overwrite[0] == 't' || overwrite[0] == 'T';
}
