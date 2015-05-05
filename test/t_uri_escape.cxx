#include "util/UriEscape.hxx"
#include "util/LightString.hxx"

#include <inline/compiler.h>

#include <assert.h>
#include <string.h>

static const char *const literals[] = {
    "",
    "/",
    "/foo/;a=b&c=d",
};

static const char *const malformed[] = {
    "%",
    "%2",
    "%gg",
    "%00",
};

struct TestPair {
    const char *raw, *escaped;
};

static constexpr TestPair pairs[] = {
    { "%", "%25" },
    { "foo%bar", "foo%25bar" },
    { "1%2%3%4", "1%252%253%254" },
    { "%2%3%", "%252%253%25" },
    { "%%3%", "%25%253%25" },
    { "\xff", "%ff" },
};

static void
TestUriEscapePath()
{
    for (auto i : literals) {
        auto result = UriEscapePath(i);
        assert(result.c_str() == i);
    }

    for (auto i : pairs) {
        auto result = UriEscapePath(i.raw);
        assert(!result.IsNull());
        assert(strcmp(result.c_str(), i.escaped) == 0);
    }
}

static void
TestUriUnescape()
{
    for (auto i : literals) {
        auto result = UriUnescape(i);
        assert(result.c_str() == i);
    }

    for (auto i : malformed) {
        auto result = UriUnescape(i);
        assert(result.IsNull());
    }

    for (auto i : pairs) {
        auto result = UriUnescape(i.escaped);
        assert(!result.IsNull());
        assert(strcmp(result.c_str(), i.raw) == 0);
    }
}

int
main(gcc_unused int argc, gcc_unused char **argv)
{
    TestUriEscapePath();
    TestUriUnescape();
    return 0;
}
