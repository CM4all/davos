/*
 * WebDAV server.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

extern "C" {
#include <was/simple.h>
}

#include <inline/compiler.h>

#include <glib.h> // TODO: eliminate GLib

#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static const char *mountpoint;
static size_t mountpoint_length;

template<typename Backend>
static void
handle_options(was_simple *was, const typename Backend::Resource &resource)
{
    const char *allow_new = "OPTIONS,MKCOL,PUT,LOCK";
    const char *allow_file =
        "OPTIONS,GET,HEAD,DELETE,PROPFIND,PROPPATCH,COPY,MOVE,PUT,LOCK,UNLOCK";
    const char *allow_directory =
        "OPTIONS,DELETE,PROPFIND,PROPPATCH,COPY,MOVE,PUT,LOCK,UNLOCK";

    const char *allow;
    if (!resource.Exists())
        allow = allow_new;
    else if (resource.IsDirectory())
        allow = allow_directory;
    else
        allow = allow_file;

    was_simple_set_header(was, "allow", allow);
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

    char *unescaped = g_uri_unescape_string(uri, "/");
    if (unescaped == nullptr)
        return typename Backend::Resource();

    const std::string unescaped2(unescaped);
    g_free(unescaped);

    uri = unescaped2.c_str();

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

    /* strip trailing slash */
    std::string uri2(uri);
    if (!uri2.empty() && uri2.back() == '/')
        uri2.pop_back();

    return backend.Map(uri2.c_str());
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

    static unsigned long old = -1;
    if (value != old) {
        old = value;
        umask(value);
    }

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

gcc_pure
static bool
HasTrailingSlash(const char *uri)
{
    const size_t length = strlen(uri);
    assert(length > 0);
    return uri[length - 1] == '/';
}

template<typename Backend>
static void
run2(Backend &backend, was_simple *was, const char *uri)
{
    const auto resource = map_uri(backend, uri);
    if (resource.IsNull()) {
        was_simple_status(was, HTTP_STATUS_NOT_FOUND);
        return;
    }

    const http_method_t method = was_simple_get_method(was);

    if (HasTrailingSlash(uri)) {
        if (method == HTTP_METHOD_PUT) {
            /* a trailing slash is not allowed for (new) regular
               file */
            was_simple_status(was, HTTP_STATUS_METHOD_NOT_ALLOWED);
            return;
        } else if (resource.Exists() && !resource.IsDirectory()) {
            /* trailing slash is only allowed (and obligatory) for
               directories (collections) */
            was_simple_status(was, HTTP_STATUS_NOT_FOUND);
            return;
        }
    }

    switch (method) {
    case HTTP_METHOD_OPTIONS:
        if (!was_simple_input_close(was))
            return;

        handle_options<Backend>(was, resource);
        break;

    case HTTP_METHOD_HEAD:
        if (!was_simple_input_close(was))
            return;

        backend.HandleHead(was, resource);
        break;

    case HTTP_METHOD_GET:
        if (!was_simple_input_close(was))
            return;

        backend.HandleGet(was, resource);
        break;

    case HTTP_METHOD_PUT:
        if (!was_simple_has_body(was)) {
            was_simple_status(was, HTTP_STATUS_BAD_REQUEST);
            return;
        }

        backend.HandlePut(was, resource);
        break;

    case HTTP_METHOD_DELETE:
        if (!was_simple_input_close(was))
            return;

        backend.HandleDelete(was, resource);
        break;

    case HTTP_METHOD_PROPFIND:
        backend.HandlePropfind(was, uri, resource);
        break;

    case HTTP_METHOD_PROPPATCH:
        backend.HandleProppatch(was, uri, resource);
        break;

    case HTTP_METHOD_MKCOL:
        if (!was_simple_input_close(was))
            return;

        backend.HandleMkcol(was, resource);
        break;

    case HTTP_METHOD_COPY: {
        if (!was_simple_input_close(was))
            return;

        const char *p = was_simple_get_header(was, "destination");
        if (p == nullptr) {
            was_simple_status(was, HTTP_STATUS_BAD_REQUEST);
            return;
        }

        p = get_uri_path(p);

        const auto destination = map_uri(backend, p);
        if (destination.IsNull()) {
            /* can't copy the file out of its site */
            was_simple_status(was, HTTP_STATUS_FORBIDDEN);
            return;
        }

        backend.HandleCopy(was, resource, destination);
    }
        break;

    case HTTP_METHOD_MOVE: {
        if (!was_simple_input_close(was))
            return;

        const char *p = was_simple_get_header(was, "destination");
        if (p == nullptr) {
            was_simple_status(was, HTTP_STATUS_BAD_REQUEST);
            return;
        }

        p = get_uri_path(p);

        const auto destination = map_uri(backend, p);
        if (destination.IsNull()) {
            /* can't move the file out of its site */
            was_simple_status(was, HTTP_STATUS_FORBIDDEN);
            return;
        }

        backend.HandleMove(was, resource, destination);
    }
        break;

    case HTTP_METHOD_LOCK:
        backend.HandleLock(was, resource);
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
run(Backend &backend, was_simple *was, const char *uri)
{
    if (!configure(backend, was)) {
        was_simple_status(was, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }

    run2(backend, was, uri);
    backend.TearDown();
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
