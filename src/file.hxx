/*
 * Local file resources.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_FILE_HXX
#define DAVOS_FILE_HXX

#include <string>

#include <assert.h>
#include <sys/stat.h>

class FileResource {
    std::string path;

    int error;

    struct stat st;

public:
    FileResource():error(-1) {}

    explicit FileResource(std::string &&_path);

    bool IsNull() const {
        return error == -1;
    }

    int GetError() const {
        return error;
    }

    bool Exists() const {
        assert(!IsNull());

        return error == 0;
    }

    bool IsFile() const {
        assert(!IsNull());
        assert(Exists());

        return S_ISREG(st.st_mode);
    }

    bool IsDirectory() const {
        assert(!IsNull());
        assert(Exists());

        return S_ISDIR(st.st_mode);
    }

    const char *GetPath() const {
        assert(!IsNull());

        return path.c_str();
    }

    const struct stat &GetStat() const {
        return st;
    }

    off_t GetSize() const {
        return st.st_size;
    }

    time_t GetAccessTime() const {
        return st.st_atime;
    }

    time_t GetModificationTime() const {
        return st.st_mtime;
    }
};

#endif
