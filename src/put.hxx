// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * Request handler for PUT.
 */

#pragma once

struct was_simple;
class FileResource;

void
handle_put(was_simple *was, const FileResource &resource);
