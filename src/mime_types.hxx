// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * Loader for /etc/mime.types.
 */

#pragma once

[[gnu::pure]]
const char *
LookupMimeTypeByFileName(const char *name);

[[gnu::pure]]
const char *
LookupMimeTypeByFilePath(const char *path);
