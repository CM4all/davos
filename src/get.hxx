// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * Request handler for local files.
 */

#pragma once

struct was_simple;
class FileResource;

void
handle_get(was_simple *was, const FileResource &resource);

void
handle_head(was_simple *was, const FileResource &resource);
