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
#include <errno.h>

struct FileResource {
    std::string path;

    int error;

    struct stat st;

    FileResource():error(-1) {}

    explicit FileResource(std::string &&_path)
        :path(std::move(_path)), error(0) {
        if (stat(path.c_str(), &st) < 0)
            error = errno;
    }

    bool IsNull() const {
        return error == -1;
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
};

#endif
