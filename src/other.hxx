// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * Request handlers for "other" commands.
 */

#pragma once

struct was_simple;
class FileResource;

void
handle_delete(was_simple *was, const FileResource &resource);

void
handle_copy(was_simple *w, const FileResource &src, const FileResource &dest);

void
handle_move(was_simple *w, const FileResource &src, const FileResource &dest);
