// SPDX-License-Identifier: BSD-2-Clause
// Copyright Max Kellermann <max.kellermann@gmail.com>

#include "util/UriEscape.hxx"
#include "util/LightString.hxx"

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
		const auto result = UriEscapePath(i);
		ASSERT_FALSE(result.IsNull());
		const std::string_view sv{result};
		EXPECT_EQ(sv.data(), i);
		EXPECT_EQ(sv.size(), strlen(i));
	}

	for (auto i : pairs) {
		const auto result = UriEscapePath(i.raw);
		ASSERT_FALSE(result.IsNull());
		const std::string_view sv{result};
		EXPECT_EQ(sv, i.escaped);
	}
}

TEST(UriEscapeTest, Unescape)
{
	for (auto i : literals) {
		const auto result = UriUnescape(i);
		ASSERT_FALSE(result.IsNull());
		const std::string_view sv{result};
		EXPECT_EQ(sv.data(), i);
		EXPECT_EQ(sv.size(), strlen(i));
	}

	for (auto i : malformed) {
		auto result = UriUnescape(i);
		assert(result.IsNull());
	}

	for (auto i : pairs) {
		const auto result = UriUnescape(i.escaped);
		assert(!result.IsNull());
		const std::string_view sv{result};
		EXPECT_EQ(sv, i.raw);
	}
}
