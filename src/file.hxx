/*
 * Local file resources.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_FILE_HXX
#define DAVOS_FILE_HXX

#include <string>
#include <chrono>

#include <assert.h>
#include <sys/stat.h>

class FileResource {
    std::string path;

    int error;

    struct stat st;

public:
    explicit FileResource(std::string &&_path);

    int GetError() const {
        return error;
    }

    bool Exists() const {
        return error == 0;
    }

    bool IsFile() const {
        assert(Exists());

        return S_ISREG(st.st_mode);
    }

    bool IsDirectory() const {
        assert(Exists());

        return S_ISDIR(st.st_mode);
    }

    const char *GetPath() const {
        return path.c_str();
    }

    const struct stat &GetStat() const {
        return st;
    }

    off_t GetSize() const {
        return st.st_size;
    }

    std::chrono::system_clock::time_point GetAccessTime() const {
        return std::chrono::system_clock::from_time_t(st.st_atime);
    }

    std::chrono::system_clock::time_point GetModificationTime() const {
        return std::chrono::system_clock::from_time_t(st.st_mtime);
    }

    /**
     * Create the file with O_EXCL.
     *
     * @return 0 on success, an errno value on error
     */
    int CreateExclusive() const;
};

#endif
