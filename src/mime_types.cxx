/*
 * Loader for /etc/mime.types.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "mime_types.hxx"
#include "util/CharUtil.hxx"
#include "util/StringStrip.hxx"
#include "util/ScopeExit.hxx"

#include <map>
#include <string>
#include <algorithm>

#include <string.h>

static std::map<std::string, std::string> mime_types;
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
LookupMimeTypeByExtension(const char *ext)
{
	LoadMimeTypesFile();

	const size_t length = strlen(ext);
	char lc_ext[32];
	if (length >= sizeof(lc_ext))
		return nullptr;

	std::transform(ext, ext + length, lc_ext, ToLowerASCII);
	lc_ext[length] = 0;

	auto i = mime_types.find(lc_ext);
	if (i == mime_types.end())
		return nullptr;

	return i->second.c_str();
}

const char *
LookupMimeTypeByFileName(const char *name)
{
	const char *dot = strrchr(name + 1, '.');
	if (dot == nullptr || dot[1] == 0)
		return nullptr;

	return LookupMimeTypeByExtension(dot + 1);
}

const char *
LookupMimeTypeByFilePath(const char *path)
{
	const char *slash = strrchr(path, '/');
	if (slash != nullptr)
		path = slash + 1;

	return LookupMimeTypeByFileName(path);
}
