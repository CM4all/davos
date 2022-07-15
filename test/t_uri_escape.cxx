#include "util/UriEscape.hxx"
#include "util/LightString.hxx"
#include "util/Compiler.h"

#include <gtest/gtest.h>

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

TEST(UriEscapeTest, EscapePath)
{
	for (auto i : literals) {
		auto result = UriEscapePath(i);
		ASSERT_EQ(result.c_str(), i);
	}

	for (auto i : pairs) {
		auto result = UriEscapePath(i.raw);
		ASSERT_FALSE(result.IsNull());
		ASSERT_STREQ(result.c_str(), i.escaped);
	}
}

TEST(UriEscapeTest, Unescape)
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
