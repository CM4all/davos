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

	LightString():value(nullptr), allocation(nullptr) {}

	explicit LightString(char *_allocation)
		:value(_allocation), allocation(_allocation) {}

	explicit LightString(const char *_value)
		:value(_value), allocation(nullptr) {}

public:
	LightString(std::nullptr_t n):value(n), allocation(n) {}

	LightString(LightString &&src)
		:value(src.value), allocation(src.Steal()) {
	}

	~LightString() {
		delete[] allocation;
	}

	static LightString Donate(char *allocation) {
		return LightString(allocation);
	}

	static LightString Make(const char *value) {
		return LightString(value);
	}

	static LightString Null() {
		return nullptr;
	}

	LightString &operator=(LightString &&src) {
		value = src.value;
		std::swap(allocation, src.allocation);
		return *this;
	}

	bool IsNull() const {
		return value == nullptr;
	}

	const char *c_str() const {
		return value;
	}

	char *Steal() {
		char *result = allocation;
		allocation = nullptr;
		return result;
	}
};
