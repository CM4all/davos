/*
 * Local file resources.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_FILE_HXX
#define DAVOS_FILE_HXX

#include "Chrono.hxx"

#include <string>
#include <chrono>

#include <assert.h>
#include <sys/stat.h>

class FileResource {
	std::string path;

	int error;

	struct stat st;

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

		return S_ISREG(st.st_mode);
	}

	bool IsDirectory() const noexcept {
		assert(Exists());

		return S_ISDIR(st.st_mode);
	}

	const char *GetPath() const noexcept {
		return path.c_str();
	}

	const struct stat &GetStat() const noexcept {
		return st;
	}

	const struct stat *GetStatIfExists() const noexcept {
		return Exists() ? &GetStat() : nullptr;
	}

	off_t GetSize() const noexcept {
		return st.st_size;
	}

	std::chrono::system_clock::time_point GetAccessTime() const noexcept {
		return ToSystemTime(st.st_atim);
	}

	std::chrono::system_clock::time_point GetModificationTime() const noexcept {
		return ToSystemTime(st.st_mtim);
	}

	/**
	 * Create the file with O_EXCL.
	 *
	 * @return 0 on success, an errno value on error
	 */
	int CreateExclusive() const noexcept;
};

#endif
