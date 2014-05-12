/*
 * Loader for /etc/mime.types.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "mime_types.hxx"

#include <string.h>

static constexpr inline int
char_is_whitespace(char ch)
{
    return ((unsigned char)ch) <= 0x20;
}

static char *
end_of_word(char *p)
{
    while (!char_is_whitespace(*p))
        ++p;
    return p;
}

static char *
skip_space(char *p)
{
    while (*p != 0 && char_is_whitespace(*p))
        ++p;
    return p;
}

static std::string
LookupMimeTypeByExtension(const char *ext)
{
    FILE *file = fopen("/etc/mime.types", "r");
    if (file == nullptr)
        return std::string();

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
        while (*(start = skip_space(p)) != 0) {
            p = end_of_word(start);
            if (*p != 0)
                /* terminate filename extension if it isn't already at
                   EOL */
                *p++ = 0;

            if (strcasecmp(start, ext) == 0) {
                fclose(file);
                return line;
            }
        }
    }

    fclose(file);
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