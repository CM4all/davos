/*
 * WebDAV server.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "frontend.hxx"
#include "error.hxx"
#include "directory.hxx"
#include "get.hxx"
#include "put.hxx"
#include "propfind.hxx"
#include "proppatch.hxx"
#include "lock.hxx"
#include "other.hxx"
#include "file.hxx"

#include <inline/compiler.h>

#include <stdio.h>
#include <stdlib.h>

class SimpleBackend {
    const char *document_root;

public:
    typedef FileResource Resource;

    bool Setup(was_simple *w);

    gcc_pure
    Resource Map(const char *uri) const;

    void HandleHead(was_simple *w, const Resource &resource) {
        handle_head(w, resource.GetPath());
    }

    void HandleGet(was_simple *w, const Resource &resource) {
        handle_get(w, resource.GetPath());
    }

    void HandlePut(was_simple *w, const Resource &resource) {
        handle_put(w, resource.GetPath());
    }

    void HandleDelete(was_simple *w, const Resource &resource) {
        handle_delete(w, resource);
    }

    void HandlePropfind(was_simple *w, const char *uri,
                        const Resource &resource) {
        handle_propfind(w, uri, resource);
    }

    void HandleProppatch(was_simple *w, const char *uri,
                         const Resource &resource) {
        handle_proppatch(w, uri, resource);
    }

    void HandleMkcol(was_simple *w, const Resource &resource) {
        handle_mkcol(w, resource.GetPath());
    }

    void HandleCopy(was_simple *w, const Resource &src, const Resource &dest) {
        handle_copy(w, src, dest);
    }

    void HandleMove(was_simple *w, const Resource &src, const Resource &dest) {
        handle_move(w, src, dest);
    }

    void HandleLock(was_simple *w, const Resource &resource) {
        handle_lock(w, resource.GetPath());
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
