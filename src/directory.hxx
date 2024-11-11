// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * Request handler for directories (e.g. MKCOL).
 */

#pragma once

struct was_simple;
class FileResource;

void
handle_mkcol(was_simple *was, const FileResource &resource);
