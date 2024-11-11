// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

#pragma once

struct statx;
struct was_simple;

/**
 * @return false if there is an "if-match" header and it does not
 * match the file's ETag
 */
[[gnu::pure]]
bool
CheckIfMatch(const struct was_simple &was, const struct statx *st) noexcept;

/**
 * @return false if there is an "if-none-match" header and it matches
 * the file's ETag
 */
[[gnu::pure]]
bool
CheckIfNoneMatch(const struct was_simple &was, const struct statx *st) noexcept;
