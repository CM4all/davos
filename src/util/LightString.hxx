// SPDX-License-Identifier: BSD-2-Clause
// Copyright 2015 Max Kellermann <max.kellermann@gmail.com>

#pragma once

#include "util/TagStructs.hxx"

#include <cstddef> // for std::nullptr_t
#include <string_view>
#include <utility>

/**
 * A string pointer whose memory may or may not be managed by this
 * class.
 */
class LightString {
	std::string_view value;
	char *allocation;

	constexpr LightString() noexcept:allocation(nullptr) {}

	explicit constexpr LightString(AdoptTag, char *_allocation, std::size_t size) noexcept
		:value(_allocation, size), allocation(_allocation) {}

	explicit constexpr LightString(std::string_view _value) noexcept
		:value(_value), allocation(nullptr) {}

public:
	constexpr LightString(std::nullptr_t n) noexcept:allocation(n) {}

	constexpr LightString(LightString &&src) noexcept
		:value(src.value), allocation(std::exchange(src.allocation, nullptr)) {}

	constexpr ~LightString() noexcept {
		delete[] allocation;
	}

	static constexpr LightString Donate(char *allocation, std::size_t size) noexcept {
		return LightString{AdoptTag{}, allocation, size};
	}

	static constexpr LightString Make(std::string_view value) noexcept {
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
		return value.data() == nullptr;
	}

	constexpr operator std::string_view() const noexcept {
		return value;
	}
};
