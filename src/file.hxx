// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * Local file resources.
 */

#pragma once

#include "time/StatxCast.hxx"

#include <string>
#include <chrono>

#include <assert.h>
#include <sys/stat.h>

class FileResource {
	std::string path;

	int error;

	struct statx st;

public:
	explicit FileResource(std::string &&_path) noexcept;

	int GetError() const noexcept {
		return error;
	}

	bool Exists() const noexcept {
		return error == 0;
	}

	bool IsFile() const noexcept {
		assert(Exists());

		return S_ISREG(st.stx_mode);
	}

	bool IsDirectory() const noexcept {
		assert(Exists());

		return S_ISDIR(st.stx_mode);
	}

	const char *GetPath() const noexcept {
		return path.c_str();
	}

	const struct statx &GetStat() const noexcept {
		return st;
	}

	const struct statx *GetStatIfExists() const noexcept {
		return Exists() ? &GetStat() : nullptr;
	}

	off_t GetSize() const noexcept {
		return st.stx_size;
	}

	std::chrono::system_clock::time_point GetAccessTime() const noexcept {
		return ToSystemTimePoint(st.stx_atime);
	}

	std::chrono::system_clock::time_point GetModificationTime() const noexcept {
		return ToSystemTimePoint(st.stx_mtime);
	}

	/**
	 * Create the file with O_EXCL.
	 *
	 * @return 0 on success, an errno value on error
	 */
	int CreateExclusive() const noexcept;
};
