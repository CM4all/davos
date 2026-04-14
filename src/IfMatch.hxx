// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

#pragma once

#include "PreconditionResult.hxx"

struct statx;
struct was_simple;

[[gnu::pure]]
PreconditionResult
CheckIfMatch(const struct was_simple &was, const struct statx *st) noexcept;

[[gnu::pure]]
PreconditionResult
CheckIfNoneMatch(const struct was_simple &was, const struct statx *st) noexcept;
