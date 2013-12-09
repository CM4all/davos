/*
 * WebDAV server.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "error.hxx"
#include "directory.hxx"
#include "file.hxx"
#include "put.hxx"
#include "propfind.hxx"
#include "proppatch.hxx"
#include "lock.hxx"
#include "other.hxx"

extern "C" {
#include <was/simple.h>
}

#include <inline/compiler.h>

#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static const char *mountpoint;
static size_t mountpoint_length;

class SimpleBackend {
    const char *document_root;

public:
    typedef std::string Resource;

    bool Setup(was_simple *w);

    gcc_pure
    Resource Map(const char *uri) const;

    void HandleOptions(was_simple *w, const Resource &path) {
        handle_options(w, path.c_str());
    }

    void HandleHead(was_simple *w, const Resource &path) {
        handle_head(w, path.c_str());
    }

    void HandleGet(was_simple *w, const Resource &path) {
        handle_get(w, path.c_str());
    }

    void HandlePut(was_simple *w, const Resource &path) {
        handle_put(w, path.c_str());
    }

    void HandleDelete(was_simple *w, const Resource &path) {
        handle_delete(w, path.c_str());
    }

    void HandlePropfind(was_simple *w, const char *uri, const Resource &path) {
        handle_propfind(w, uri, path.c_str());
    }

    void HandleProppatch(was_simple *w, const char *uri,
                         const Resource &path) {
        handle_proppatch(w, uri, path.c_str());
    }

    void HandleMkcol(was_simple *w, const Resource &path) {
        handle_mkcol(w, path.c_str());
    }

    void HandleCopy(was_simple *w, const Resource &src, const Resource &dest) {
        handle_copy(w, src.c_str(), dest.c_str());
    }

    void HandleMove(was_simple *w, const Resource &src, const Resource &dest) {
        handle_move(w, src.c_str(), dest.c_str());
    }

    void HandleLock(was_simple *w, const Resource &path) {
        handle_lock(w, path.c_str());
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
    Resource path(document_root);

    if (*uri != 0) {
        path.push_back('/');
        path.append(uri);
    }

    return path;
}

/**
 * Extract the URI path from an absolute URL.
 */
static const char *
get_uri_path(const char *p)
{
    assert(p != nullptr);

    if (memcmp(p, "http://", 7) == 0)
        p += 7;
    else if (memcmp(p, "https://", 8) == 0)
        p += 8;
    else
        return p;

    const char *slash = strchr(p, '/');
    if (slash == nullptr)
        /* there is no URI path - assume it's just "/" */
        return "/";

    return slash;
}

template<class Backend>
static typename Backend::Resource
map_uri(const Backend &backend, const char *uri)
{
    assert(uri != nullptr);

    if (strstr(uri, "/../") != nullptr)
        return typename Backend::Resource();

    if (memcmp(uri, mountpoint, mountpoint_length) == 0)
        uri += mountpoint_length;
    else if (memcmp(uri, mountpoint, mountpoint_length - 1) == 0 &&
             uri[mountpoint_length - 1] == 0)
        /* special case for clients that remove the trailing slash
           (e.g. Microsoft) */
        uri = "";
    else
        return typename Backend::Resource();

    return backend.Map(uri);
}

static bool
configure_umask(was_simple *w)
{
    const char *p = was_simple_get_parameter(w, "DAVOS_UMASK");
    if (p == nullptr)
        p = "0022";

    char *endptr;
    unsigned long value = strtoul(p, &endptr, 8);
    if (endptr == p || *endptr != 0 || value == 0 || (value & ~0777) != 0) {
        fprintf(stderr, "Malformed DAVOS_UMASK\n");
        return false;
    }

    umask(value);
    return true;
}

gcc_pure
static bool
check_mountpoint(const char *p, size_t length)
{
    assert(p != nullptr);

    return p[0] == '/' && p[length - 1] == '/';
}

static bool
configure_mapper(was_simple *w)
{
    mountpoint = was_simple_get_parameter(w, "DAVOS_MOUNT");
    if (mountpoint == nullptr) {
        fprintf(stderr, "No DAVOS_MOUNT\n");
        return false;
    }

    mountpoint_length = strlen(mountpoint);
    if (!check_mountpoint(mountpoint, mountpoint_length)) {
        fprintf(stderr, "Malformed DAVOS_MOUNT\n");
        return false;
    }

    return true;
}

template<typename Backend>
static bool
configure(Backend &backend, was_simple *w)
{
    return configure_umask(w) && configure_mapper(w) &&
        backend.Setup(w);
}

template<typename Backend>
static void
run(Backend &backend, was_simple *was, const char *uri)
{
    if (!configure(backend, was)) {
        was_simple_status(was, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }

    const auto path = map_uri(backend, uri);
    if (path.empty()) {
        was_simple_status(was, HTTP_STATUS_NOT_FOUND);
        return;
    }

    const http_method_t method = was_simple_get_method(was);
    switch (method) {
    case HTTP_METHOD_OPTIONS:
        backend.HandleOptions(was, path);
        break;

    case HTTP_METHOD_HEAD:
        backend.HandleHead(was, path);
        break;

    case HTTP_METHOD_GET:
        backend.HandleGet(was, path);
        break;

    case HTTP_METHOD_PUT:
        backend.HandlePut(was, path);
        break;

    case HTTP_METHOD_DELETE:
        backend.HandleDelete(was, path);
        break;

    case HTTP_METHOD_PROPFIND:
        backend.HandlePropfind(was, uri, path);
        break;

    case HTTP_METHOD_PROPPATCH:
        backend.HandleProppatch(was, uri, path);
        break;

    case HTTP_METHOD_MKCOL:
        backend.HandleMkcol(was, path);
        break;

    case HTTP_METHOD_COPY: {
        const char *p = was_simple_get_header(was, "destination");
        if (p == nullptr) {
            was_simple_status(was, HTTP_STATUS_BAD_REQUEST);
            return;
        }

        p = get_uri_path(p);

        const auto destination = map_uri(backend, p);
        if (destination.empty()) {
            /* can't copy the file out of its site */
            was_simple_status(was, HTTP_STATUS_FORBIDDEN);
            return;
        }

        backend.HandleCopy(was, path, destination);
    }
        break;

    case HTTP_METHOD_MOVE: {
        const char *p = was_simple_get_header(was, "destination");
        if (p == nullptr) {
            was_simple_status(was, HTTP_STATUS_BAD_REQUEST);
            return;
        }

        p = get_uri_path(p);

        const auto destination = map_uri(backend, p);
        if (destination.empty()) {
            /* can't move the file out of its site */
            was_simple_status(was, HTTP_STATUS_FORBIDDEN);
            return;
        }

        backend.HandleMove(was, path, destination);
    }
        break;

    case HTTP_METHOD_LOCK:
        backend.HandleLock(was, path);
        break;

    case HTTP_METHOD_UNLOCK:
        /* no-op */
        break;

    default:
        was_simple_status(was, HTTP_STATUS_METHOD_NOT_ALLOWED);
   }
}

template<typename Backend>
static void
run(Backend &backend)
{
    was_simple *was = was_simple_new();
    const char *uri;

    while ((uri = was_simple_accept(was)) != nullptr)
        run(backend, was, uri);

    was_simple_free(was);
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
