/*
 * WebDAV server.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "error.hxx"
#include "file.hxx"
#include "propfind.hxx"

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

static void
options(was_simple *was, const char *path)
{
    const char *allow_new = "OPTIONS,MKCOL,PUT,LOCK";
    const char *allow_file =
        "OPTIONS,GET,HEAD,DELETE,PROPFIND,PROPPATCH,COPY,MOVE,PUT,LOCK,UNLOCK";
    const char *allow_directory =
        "OPTIONS,DELETE,PROPFIND,PROPPATCH,COPY,MOVE,PUT,LOCK,UNLOCK";

    const char *allow;
    struct stat st;
    if (stat(path, &st) < 0)
        allow = allow_new;
    else if (S_ISDIR(st.st_mode))
        allow = allow_directory;
    else
        allow = allow_file;

    was_simple_set_header(was, "allow", allow);
}

static std::string
map_uri(const char *uri)
{
    assert(uri != nullptr);

    if (memcmp(uri, "/dav/", 5) != 0 ||
        strstr(uri, "/../") != nullptr)
        return std::string();

    std::string path(document_root);
    path.push_back('/');
    path.append(uri + 5);
    return path;
}

static bool
configure_umask(was_simple *w)
{
    const char *p = was_simple_get_parameter(w, "DAVOS_UMASK");
    if (p == nullptr)
        p = "0755";

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
        options(was, path.c_str());
        break;

    case HTTP_METHOD_GET:
        get(was, path.c_str());
        break;

    case HTTP_METHOD_HEAD:
        head(was, path.c_str());
        break;

    case HTTP_METHOD_PROPFIND:
        propfind(was, uri, path.c_str());
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
