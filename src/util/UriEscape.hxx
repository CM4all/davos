// SPDX-License-Identifier: BSD-2-Clause
// Copyright Max Kellermann <max.kellermann@gmail.com>

#pragma once

class LightString;

LightString
UriEscapePath(const char *src);

LightString
UriUnescape(const char *src);
