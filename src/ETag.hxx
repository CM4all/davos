/*
 * author: Max Kellermann <mk@cm4all.com>
 */

#pragma once

#include "util/StringBuffer.hxx"
#include "util/Compiler.h"

#include <sys/stat.h>

gcc_pure
StringBuffer<32>
MakeETag(const struct stat &st) noexcept;
