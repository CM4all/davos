/*
 * author: Max Kellermann <mk@cm4all.com>
 */

#pragma once

#include "util/StringBuffer.hxx"

#include <sys/stat.h>

[[gnu::pure]]
StringBuffer<32>
MakeETag(const struct stat &st) noexcept;
