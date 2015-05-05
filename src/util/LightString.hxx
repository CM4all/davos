/*
 * Copyright (C) 2015 Max Kellermann <max@duempel.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef LIGHT_STRING_HXX
#define LIGHT_STRING_HXX

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

#endif
