/*
 * Request handler for local files.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "IfMatch.hxx"
#include "ETag.hxx"
#include "file.hxx"
#include "http/List.hxx"

#include <was/simple.h>

#include <string.h>

bool
CheckIfMatch(const struct was_simple &was, const struct stat *st) noexcept
{
	const char *p = was_simple_get_header(&was, "if-match");
	if (p == nullptr || strcmp(p, "*") == 0)
		return true;

	return st != nullptr && http_list_contains(p, MakeETag(*st).c_str());
}

bool
CheckIfNoneMatch(const struct was_simple &was, const struct stat *st) noexcept
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
