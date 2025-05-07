// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

#pragma once

#include <chrono>

#include <sys/time.h>

[[gnu::const]]
static inline struct timeval
ToTimeval(std::chrono::system_clock::time_point p)
{
	struct timeval tv;
	tv.tv_sec = std::chrono::system_clock::to_time_t(p);

	const auto remainder = p - std::chrono::system_clock::from_time_t(tv.tv_sec);
	tv.tv_usec = std::chrono::duration_cast<std::chrono::microseconds>(remainder).count();
	return tv;
}
