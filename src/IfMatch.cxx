// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

#include "IfMatch.hxx"
#include "ETag.hxx"
#include "file.hxx"
#include "http/List.hxx"

#include <was/simple.h>

#include <string.h>
#include <sys/stat.h>

PreconditionResult
CheckIfMatch(const struct was_simple &was, const struct statx *st) noexcept
{
	const char *p = was_simple_get_header(&was, "if-match");
	if (p == nullptr)
		return PreconditionResult::NONE;

	if (st == nullptr)
		return PreconditionResult::FAILURE;

	if (strcmp(p, "*") == 0)
		return PreconditionResult::SUCCESS;

	return http_list_contains(p, MakeETag(*st).c_str())
		? PreconditionResult::SUCCESS
		: PreconditionResult::FAILURE;
}

PreconditionResult
CheckIfNoneMatch(const struct was_simple &was, const struct statx *st) noexcept
{
	const char *p = was_simple_get_header(&was, "if-none-match");
	if (p == nullptr)
		return PreconditionResult::NONE;

	if (st == nullptr)
		return PreconditionResult::SUCCESS;

	if (strcmp(p, "*") == 0)
		return PreconditionResult::FAILURE;

	return http_list_contains(p, MakeETag(*st).c_str())
		? PreconditionResult::FAILURE
		: PreconditionResult::SUCCESS;
}
