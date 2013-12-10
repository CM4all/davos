/*
 * WebDAV server.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "frontend.hxx"
#include "error.hxx"
#include "directory.hxx"
#include "file.hxx"
#include "put.hxx"
#include "propfind.hxx"
#include "proppatch.hxx"
#include "lock.hxx"
#include "other.hxx"

#include <inline/compiler.h>

#include <string>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

class SimpleBackend {
    const char *document_root;

public:
    struct Resource {
        std::string path;

        int error;

        struct stat st;

        Resource():error(-1) {}

        explicit Resource(std::string &&_path)
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

    bool Setup(was_simple *w);

    gcc_pure
    Resource Map(const char *uri) const;

    void HandleOptions(was_simple *w, const Resource &resource) {
        handle_options(w, resource.path.c_str());
    }

    void HandleHead(was_simple *w, const Resource &resource) {
        handle_head(w, resource.path.c_str());
    }

    void HandleGet(was_simple *w, const Resource &resource) {
        handle_get(w, resource.path.c_str());
    }

    void HandlePut(was_simple *w, const Resource &resource) {
        handle_put(w, resource.path.c_str());
    }

    void HandleDelete(was_simple *w, const Resource &resource) {
        handle_delete(w, resource.path.c_str());
    }

    void HandlePropfind(was_simple *w, const char *uri,
                        const Resource &resource) {
        handle_propfind(w, uri, resource.path.c_str());
    }

    void HandleProppatch(was_simple *w, const char *uri,
                         const Resource &resource) {
        handle_proppatch(w, uri, resource.path.c_str());
    }

    void HandleMkcol(was_simple *w, const Resource &resource) {
        handle_mkcol(w, resource.path.c_str());
    }

    void HandleCopy(was_simple *w, const Resource &src, const Resource &dest) {
        handle_copy(w, src.path.c_str(), dest.path.c_str());
    }

    void HandleMove(was_simple *w, const Resource &src, const Resource &dest) {
        handle_move(w, src.path.c_str(), dest.path.c_str());
    }

    void HandleLock(was_simple *w, const Resource &resource) {
        handle_lock(w, resource.path.c_str());
    }
};

inline bool
SimpleBackend::Setup(was_simple *w)
{
    document_root = was_simple_get_parameter(w, "DAVOS_DOCUMENT_ROOT");
    if (document_root == nullptr) {
        fprintf(stderr, "No DAVOS_DOCUMENT_ROOT\n");
        return false;
    }

    return true;
}

inline SimpleBackend::Resource
SimpleBackend::Map(const char *uri) const
{
    std::string path(document_root);

    if (*uri != 0) {
        path.push_back('/');
        path.append(uri);
    }

    return Resource(std::move(path));
}

int
main(int argc, const char *const*argv)
{
    (void)argc;
    (void)argv;

    SimpleBackend backend;
    run(backend);
    return EXIT_SUCCESS;
}
