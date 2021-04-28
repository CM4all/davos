/*
 * Request handler for local files.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#pragma once

#include "util/Compiler.h"

#include <sys/stat.h>

struct was_simple;

/**
 * @return false if there is an "if-match" header and it does not
 * match the file's ETag
 */
gcc_pure
bool
CheckIfMatch(const struct was_simple &was, const struct stat *st) noexcept;

/**
 * @return false if there is an "if-none-match" header and it matches
 * the file's ETag
 */
gcc_pure
bool
CheckIfNoneMatch(const struct was_simple &was, const struct stat *st) noexcept;
