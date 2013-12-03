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

static const char *document_root;

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

static std::string
map_uri(const char *uri)
{
    assert(uri != nullptr);

    if (strstr(uri, "/../") != nullptr)
        return std::string();

    if (memcmp(uri, mountpoint, mountpoint_length) == 0)
        uri += mountpoint_length;
    else if (memcmp(uri, mountpoint, mountpoint_length - 1) == 0 &&
             uri[mountpoint_length - 1] == 0)
        /* special case for clients that remove the trailing slash
           (e.g. Microsoft) */
        uri = "";
    else
        return std::string();

    std::string path(document_root);

    if (*uri != 0) {
        path.push_back('/');
        path.append(uri);
    }

    return path;
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

    document_root = was_simple_get_parameter(w, "DAVOS_DOCUMENT_ROOT");
    if (document_root == nullptr) {
        fprintf(stderr, "No DAVOS_DOCUMENT_ROOT\n");
        return false;
    }

    return true;
}

static bool
configure(was_simple *w)
{
    return configure_umask(w) && configure_mapper(w);
}

static void
run(was_simple *was, const char *uri)
{
    if (!configure(was)) {
        was_simple_status(was, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }

    std::string path = map_uri(uri);
    if (path.empty()) {
        was_simple_status(was, HTTP_STATUS_NOT_FOUND);
        return;
    }

    const http_method_t method = was_simple_get_method(was);
    switch (method) {
    case HTTP_METHOD_OPTIONS:
        handle_options(was, path.c_str());
        break;

    case HTTP_METHOD_HEAD:
        head(was, path.c_str());
        break;

    case HTTP_METHOD_GET:
        get(was, path.c_str());
        break;

    case HTTP_METHOD_PUT:
        put(was, path.c_str());
        break;

    case HTTP_METHOD_DELETE:
        handle_delete(was, path.c_str());
        break;

    case HTTP_METHOD_PROPFIND:
        propfind(was, uri, path.c_str());
        break;

    case HTTP_METHOD_MKCOL:
        mkcol(was, path.c_str());
        break;

    case HTTP_METHOD_COPY: {
        const char *p = was_simple_get_header(was, "destination");
        if (p == nullptr) {
            was_simple_status(was, HTTP_STATUS_BAD_REQUEST);
            return;
        }

        p = get_uri_path(p);

        std::string destination = map_uri(p);
        if (destination.empty()) {
            /* can't copy the file out of its site */
            was_simple_status(was, HTTP_STATUS_FORBIDDEN);
            return;
        }

        handle_copy(was, path.c_str(), destination.c_str());
    }
        break;

    case HTTP_METHOD_MOVE: {
        const char *p = was_simple_get_header(was, "destination");
        if (p == nullptr) {
            was_simple_status(was, HTTP_STATUS_BAD_REQUEST);
            return;
        }

        p = get_uri_path(p);

        std::string destination = map_uri(p);
        if (destination.empty()) {
            /* can't move the file out of its site */
            was_simple_status(was, HTTP_STATUS_FORBIDDEN);
            return;
        }

        handle_move(was, path.c_str(), destination.c_str());
    }
        break;

    default:
        was_simple_status(was, HTTP_STATUS_METHOD_NOT_ALLOWED);
   }
}

int
main(int argc, const char *const*argv)
{
    (void)argc;
    (void)argv;

    was_simple *was = was_simple_new();
    const char *uri;

    while ((uri = was_simple_accept(was)) != nullptr)
        run(was, uri);

    was_simple_free(was);
    return EXIT_SUCCESS;
}
