/*
 * author: Max Kellermann <mk@cm4all.com>
 */

#pragma once

#include "util/StringBuffer.hxx"

struct statx;

[[gnu::pure]]
StringBuffer<64>
MakeETag(const struct statx &st) noexcept;
