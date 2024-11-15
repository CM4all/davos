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

bool
CheckIfMatch(const struct was_simple &was, const struct statx *st) noexcept
{
	const char *p = was_simple_get_header(&was, "if-match");
	if (p == nullptr || strcmp(p, "*") == 0)
		return true;

	return st != nullptr && http_list_contains(p, MakeETag(*st).c_str());
}

bool
CheckIfNoneMatch(const struct was_simple &was, const struct statx *st) noexcept
{
	const char *p = was_simple_get_header(&was, "if-none-match");
	if (p == nullptr)
		return true;

	if (st == nullptr)
		return true;

	if (strcmp(p, "*") == 0)
		return false;

	return !http_list_contains(p, MakeETag(*st).c_str());
}
