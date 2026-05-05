// SPDX-License-Identifier: BSD-2-Clause
// Copyright 2015 Max Kellermann <max.kellermann@gmail.com>

#pragma once

#include <cstddef> // for std::nullptr_t
#include <utility>

/**
 * A string pointer whose memory may or may not be managed by this
 * class.
 */
class LightString {
	const char *value;
	char *allocation;

	constexpr LightString() noexcept:value(nullptr), allocation(nullptr) {}

	explicit constexpr LightString(char *_allocation) noexcept
		:value(_allocation), allocation(_allocation) {}

	explicit constexpr LightString(const char *_value) noexcept
		:value(_value), allocation(nullptr) {}

public:
	constexpr LightString(std::nullptr_t n) noexcept:value(n), allocation(n) {}

	constexpr LightString(LightString &&src) noexcept
		:value(src.value), allocation(std::exchange(src.allocation, nullptr)) {}

	constexpr ~LightString() noexcept {
		delete[] allocation;
	}

	static constexpr LightString Donate(char *allocation) noexcept {
		return LightString(allocation);
	}

	static constexpr LightString Make(const char *value) noexcept {
		return LightString(value);
	}

	static constexpr LightString Null() noexcept {
		return nullptr;
	}

	constexpr LightString &operator=(LightString &&src) noexcept {
		value = src.value;
		std::swap(allocation, src.allocation);
		return *this;
	}

	constexpr bool IsNull() const noexcept {
		return value == nullptr;
	}

	constexpr const char *c_str() const noexcept {
		return value;
	}
};
