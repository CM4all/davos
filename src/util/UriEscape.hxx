// SPDX-License-Identifier: BSD-2-Clause
// Copyright Max Kellermann <max.kellermann@gmail.com>

#pragma once

#include <string_view>

class LightString;

LightString
UriEscapePath(std::string_view src);

LightString
UriUnescape(std::string_view src);
