// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * PROPFIND implementation.
 */

#pragma once

struct was_simple;
class FileResource;

void
handle_propfind(was_simple *was, const char *uri,
		const FileResource &resource);
