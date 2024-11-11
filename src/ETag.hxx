// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

#pragma once

#include "util/StringBuffer.hxx"

struct statx;

[[gnu::pure]]
StringBuffer<64>
MakeETag(const struct statx &st) noexcept;
