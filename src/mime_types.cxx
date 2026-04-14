// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * Loader for /etc/mime.types.
 */

#include "mime_types.hxx"
#include "util/CharUtil.hxx"
#include "util/StringStrip.hxx"
#include "util/StringSplit.hxx"
#include "util/ScopeExit.hxx"

#include <map>
#include <string>
#include <algorithm>

#include <string.h>

static std::map<std::string, std::string, std::less<>> mime_types;
static bool mime_types_loaded = false;

static char *
end_of_word(char *p)
{
	while (!IsWhitespaceOrNull(*p))
		++p;
	return p;
}

static void
LoadMimeTypesFile()
{
	if (mime_types_loaded)
		return;

	mime_types_loaded = true;

	FILE *file = fopen("/etc/mime.types", "r");
	if (file == nullptr)
		return;

	AtScopeExit(file) { fclose(file); };

	char line[256];
	while (fgets(line, sizeof(line), file) != nullptr) {
		if (line[0] == '#') /* # is comment */
			continue;

		/* first column = mime type */

		char *p = end_of_word(line);
		if (p == line || *p == 0)
			continue;

		*p++ = 0; /* terminate first column */

		/* iterate through all following columns */

		char *start;
		while (*(start = StripLeft(p)) != 0) {
			p = end_of_word(start);
			if (*p != 0)
				/* terminate filename extension if it isn't already at
				   EOL */
				*p++ = 0;

			if (p > start)
				mime_types.emplace(start, line);
		}
	}
}

static const char *
LookupMimeTypeByExtension(std::string_view ext)
{
	LoadMimeTypesFile();

	char lc_ext[32];
	if (ext.size() >= sizeof(lc_ext))
		return nullptr;

	std::transform(ext.begin(), ext.end(), lc_ext, ToLowerASCII);
	lc_ext[ext.size()] = 0;

	auto i = mime_types.find(lc_ext);
	if (i == mime_types.end())
		return nullptr;

	return i->second.c_str();
}

const char *
LookupMimeTypeByFileName(std::string_view name)
{
	const auto [_, suffix] = SplitLast(name, '.');
	if (suffix.empty())
		return nullptr;

	return LookupMimeTypeByExtension(suffix);
}

const char *
LookupMimeTypeByFilePath(std::string_view path)
{
	const auto [_, base] = SplitLast(path, '/');
	if (base.data() != nullptr)
		path = base;

	return LookupMimeTypeByFileName(path);
}
