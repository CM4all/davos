/*
 * WebDAV protocol utilities.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_UTIL_HXX
#define DAVOS_UTIL_HXX

#include <inline/compiler.h>

struct was_simple;

gcc_pure
bool
get_boolean_header(was_simple *w, const char *name, bool default_value);

gcc_pure
static inline bool
get_overwrite_header(was_simple *w)
{
    return get_boolean_header(w, "overwrite", true);
}

#endif
