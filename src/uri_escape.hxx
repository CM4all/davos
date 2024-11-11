// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * Escape and unescape in URI style ('%20').
 */

#pragma once

#include "util/Compiler.h"

#include <string>

void
AppendUriEscape(std::string &dest, const char *src);
