// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * Loader for /etc/mime.types.
 */

#pragma once

#include <string_view>

[[gnu::pure]]
const char *
LookupMimeTypeByFileName(std::string_view name);

[[gnu::pure]]
const char *
LookupMimeTypeByFilePath(std::string_view path);
