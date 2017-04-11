/*
 * Loader for /etc/mime.types.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "mime_types.hxx"
#include "util/CharUtil.hxx"
#include "util/StringUtil.hxx"
#include "util/ScopeExit.hxx"

#include <string.h>

static char *
end_of_word(char *p)
{
    while (!IsWhitespaceOrNull(*p))
        ++p;
    return p;
}

static std::string
LookupMimeTypeByExtension(const char *ext)
{
    FILE *file = fopen("/etc/mime.types", "r");
    if (file == nullptr)
        return std::string();

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

            if (strcasecmp(start, ext) == 0)
                return line;
        }
    }

    return std::string();
}

std::string
LookupMimeTypeByFileName(const char *name)
{
    const char *dot = strrchr(name + 1, '.');
    if (dot == nullptr || dot[1] == 0)
        return std::string();

    return LookupMimeTypeByExtension(dot + 1);
}

std::string
LookupMimeTypeByFilePath(const char *path)
{
    const char *slash = strrchr(path, '/');
    if (slash != nullptr)
        path = slash + 1;

    return LookupMimeTypeByFileName(path);
}
