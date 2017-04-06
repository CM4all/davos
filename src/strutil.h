/*
 * Common string utilities.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef __BENG_STRUTIL_H
#define __BENG_STRUTIL_H

#include <inline/compiler.h>

#include <stdbool.h>

static gcc_always_inline bool
char_is_digit(char ch)
{
    return ch >= '0' && ch <= '9';
}

#endif
