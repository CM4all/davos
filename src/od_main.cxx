/*
 * WebDAV server.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "od_resource.hxx"
#include "frontend.hxx"
#include "wxml.hxx"
#include "splice.hxx"

extern "C" {
#include "date.h"

#include <inline/compiler.h>
#include <od/setup.h>
#include <od/site.h>
#include <od/resource.h>
#include <od/stat.h>
#include <od/list.h>
#include <od/stream.h>
#include <od/create.h>
}

#include <string>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

class OnlineDriveBackend {
    od_setup *const setup;
    od_site *site;

public:
    typedef OnlineDriveResource Resource;

    OnlineDriveBackend(od_setup *_setup)
        :setup(_setup), site(nullptr) {}
    ~OnlineDriveBackend() {
        od_setup_free(setup);
    }

    bool Setup(was_simple *w);

    void TearDown() {
        assert(site != nullptr);

        od_site_free(site);
        site = nullptr;
    }

    gcc_pure
    Resource Map(const char *uri) const {
        od_resource *root = od_site_get_root(site);

        return OnlineDriveResource(root, uri,
                                   od_resource_lookup(root, uri, nullptr));
    }

    void HandleHead(was_simple *w, const Resource &resource);
    void HandleGet(was_simple *w, const Resource &resource);
    void HandlePut(was_simple *w, const Resource &resource);
    void HandleDelete(was_simple *w, const Resource &resource);

    void HandlePropfind(was_simple *w, const char *uri,
                        const Resource &resource);

    void HandleProppatch(was_simple *w, const char *uri,
                         const Resource &resource) {
        (void)w;
        (void)uri;
        (void)resource;
        //handle_proppatch(w, uri, path.c_str());
    }

    void HandleMkcol(was_simple *w, const Resource &resource);
    void HandleCopy(was_simple *w, const Resource &src, const Resource &dest);
    void HandleMove(was_simple *w, const Resource &src, const Resource &dest);

    void HandleLock(was_simple *w, const Resource &resource) {
        (void)w;
        (void)resource;
        //handle_lock(w, path.c_str());
    }
};

bool
OnlineDriveBackend::Setup(was_simple *w)
{
    assert(site == nullptr);

    const char *site_id = was_simple_get_parameter(w, "DAVOS_SITE");
    if (site_id == nullptr) {
        fprintf(stderr, "No DAVOS_SITE\n");
        return false;
    }

    GError *error = nullptr;
    site = od_setup_open(setup, site_id, &error);
    if (site == nullptr) {
        fprintf(stderr, "%s\n", error->message);
        g_error_free(error);
        return false;
    }

    return true;
}

static bool
static_response_headers(was_simple *w, const OnlineDriveResource &resource,
                        const od_stat &st)
{
    assert(resource.Exists());

    const char *content_type = st.content_type;
    if (content_type == nullptr)
        content_type = "application/octet-stream";
    if (!was_simple_set_header(w, "content-type", content_type))
        return false;

    if (st.mtime > 0 &&
        !was_simple_set_header(w, "last-modified",
                               http_date_format(st.mtime)))
        return false;

    const char *id = resource.GetId();
    if (id != nullptr) {
        std::string etag;
        etag.reserve(strlen(id) + 2);
        etag.push_back('"');
        etag.append(id);
        etag.push_back('"');
        if (!was_simple_set_header(w, "etag", etag.c_str()))
            return false;
    }

    return true;
}

void
OnlineDriveBackend::HandleHead(was_simple *w, const Resource &resource)
{
    if (!resource.Exists()) {
        was_simple_status(w, HTTP_STATUS_NOT_FOUND);
        return;
    }

    if (!resource.IsFile()) {
        was_simple_status(w, HTTP_STATUS_METHOD_NOT_ALLOWED);
        return;
    }

    const od_stat &st = resource.GetStat();

    if (!static_response_headers(w, resource, st))
        return;

    if (st.size != OD_SIZE_UNKNOWN) {
        char buffer[64];
        sprintf(buffer, "%llu", (unsigned long long)st.size);
        if (!was_simple_set_header(w, "content-length", buffer))
            return;
    }
}

void
OnlineDriveBackend::HandleGet(was_simple *w, const Resource &resource)
{
    // TODO: range, if-modified-since, if-match, ...

    if (!resource.Exists()) {
        was_simple_status(w, HTTP_STATUS_NOT_FOUND);
        return;
    }

    if (!resource.IsFile()) {
        was_simple_status(w, HTTP_STATUS_METHOD_NOT_ALLOWED);
        return;
    }

    const od_stat &st = resource.GetStat();

    if (!static_response_headers(w, resource, st))
        return;

    const char *path = resource.GetLocalPath();
    if (st.size != OD_SIZE_UNKNOWN && path != nullptr) {
        int fd = open(path, O_RDONLY|O_NOCTTY);
        if (fd < 0) {
            was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
            return;
        }

        splice_to_was(w, fd, st.size);
        close(fd);
    } else {
        GError *error = nullptr;
        od_stream *stream = resource.OpenStream(&error);
        if (stream == nullptr) {
            fprintf(stderr, "%s\n", error->message);
            g_error_free(error);
            return;
        }

        if (st.size != OD_SIZE_UNKNOWN &&
            !was_simple_set_length(w, st.size))
            return;

        char buffer[8192];
        while (true) {
            size_t nbytes = od_stream_read(stream, buffer, sizeof(buffer),
                                           &error);
            if (nbytes == (size_t)-1) {
                fprintf(stderr, "%s\n", error->message);
                g_error_free(error);
                break;
            }

            if (nbytes == 0)
                break;

            if (!was_simple_write(w, buffer, nbytes))
                break;
        }

        od_stream_free(stream);
    }
}

void
OnlineDriveBackend::HandlePut(was_simple *w, const Resource &resource)
{
    assert(was_simple_has_body(w));

    if (resource.Exists() && !resource.IsFile()) {
        was_simple_status(w, HTTP_STATUS_CONFLICT);
        return;
    }

    GError *error = nullptr;
    auto parent = resource.GetParent(&error);
    if (parent.first == nullptr) {
        // TODO: improve error handling
        if (error != nullptr) {
            fprintf(stderr, "%s\n", error->message);
            g_error_free(error);
        }

        was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }

    od_resource_create *c = od_resource_create_begin(site, &error);
    if (c == nullptr) {
        od_resource_free(parent.first);
        fprintf(stderr, "%s\n", error->message);
        g_error_free(error);
        was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }

    bool success =
        od_resource_create_set_location(c, parent.first, parent.second,
                                        &error);
    od_resource_free(parent.first);
    if (!success) {
        od_resource_create_abort(c);
        fprintf(stderr, "%s\n", error->message);
        g_error_free(error);
        was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }

    // TODO: support slow mode
    int fd = od_resource_create_open(c, &error);
    if (fd < 0) {
        od_resource_create_abort(c);
        fprintf(stderr, "%s\n", error->message);
        g_error_free(error);
        was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }

    if (!splice_from_was(w, fd)) {
        od_resource_create_abort(c);
        was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }

    od_resource *new_resource = od_resource_create_commit(c, &error);
    if (new_resource == nullptr) {
        fprintf(stderr, "%s\n", error->message);
        g_error_free(error);
        was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }

    od_resource_free(new_resource);

    was_simple_status(w, HTTP_STATUS_CREATED);
}

void
OnlineDriveBackend::HandleDelete(was_simple *w, const Resource &resource)
{
    if (!resource.Exists()) {
        was_simple_status(w, HTTP_STATUS_NOT_FOUND);
        return;
    }

    // TODO: recursive delete for non-empty directories

    GError *error = nullptr;
    if (!resource.Delete(&error)) {
        fprintf(stderr, "%s\n", error->message);
        g_error_free(error);
        was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }
}

static bool
PropfindResource(was_simple *w, std::string &uri,
                 const OnlineDriveResource &resource,
                 unsigned depth)
{
    assert(resource.Exists());

    if (!open_response_prop(w, uri.c_str(), "HTTP/1.1 200 OK"))
        return false;

    const od_stat &st = resource.GetStat();

    if (resource.IsDirectory()) {
        if (!resourcetype_collection(w))
            return false;
    } else if (resource.IsFile()) {

        if (st.size != OD_SIZE_UNKNOWN &&
            !wxml_format_element(w, "D:getcontentlength", "%llu",
                                 (unsigned long long)st.size))
            return false;
    }

    if (st.mtime > 0 &&
        !wxml_string_element(w, "D:getlastmodified",
                             http_date_format(st.mtime)))
        return false;

    if (!close_response_prop(w))
        return false;

    if (depth > 0 && resource.IsDirectory()) {
        --depth;

        GError *error = nullptr;
        od_resource_list *list = resource.GetChildren(&error);
        if (list != nullptr) {
            uri.push_back('/');
            const auto uri_length = uri.length();

            od_resource *child;
            while ((child = od_resource_list_next(list, &error)) != nullptr) {
                OnlineDriveResource child2(resource, child);

                uri.resize(uri_length);
                uri.append(child2.GetName());

                if (!PropfindResource(w, uri, child2, depth)) {
                    od_resource_list_free(list);
                    return false;
                }
            }

            od_resource_list_free(list);
        }

        if (error != nullptr) {
            fprintf(stderr, "Failed to list children of %s: %s\n",
                    uri.c_str(), error->message);
            g_error_free(error);
        }
    }

    return true;
}

void
OnlineDriveBackend::HandlePropfind(was_simple *w, const char *_uri,
                                   const Resource &resource)
{
    if (!resource.Exists()) {
        was_simple_status(w, HTTP_STATUS_NOT_FOUND);
        return;
    }

    unsigned depth = 0;
    const char *depth_string = was_simple_get_header(w, "depth");
    if (depth_string != nullptr)
        depth = strtoul(depth_string, nullptr, 10);

    if (!was_simple_status(w, HTTP_STATUS_MULTI_STATUS) ||
        !was_simple_set_header(w, "content-type",
                               "text/xml; charset=\"utf-8\"") ||
        !begin_multistatus(w))
        return;

    std::string uri(_uri);
    if (!PropfindResource(w, uri, resource, depth))
        return;

    end_multistatus(w);
}

void
OnlineDriveBackend::HandleMkcol(was_simple *w, const Resource &resource)
{
    if (resource.Exists()) {
        was_simple_status(w, HTTP_STATUS_CONFLICT);
        return;
    }

    GError *error = nullptr;
    auto parent = resource.GetParent(&error);
    if (parent.first == nullptr) {
        // TODO: improve error handling
        if (error != nullptr) {
            fprintf(stderr, "%s\n", error->message);
            g_error_free(error);
        }

        was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }

    od_resource *new_directory =
        od_resource_mkdir(parent.first, parent.second, &error);
    od_resource_free(parent.first);
    if (new_directory == nullptr) {
        fprintf(stderr, "%s\n", error->message);
        g_error_free(error);
        was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }

    od_resource_free(new_directory);

    was_simple_status(w, HTTP_STATUS_CREATED);
}

void
OnlineDriveBackend::HandleCopy(was_simple *w, const Resource &src,
                               const Resource &dest)
{
    if (!src.Exists()) {
        was_simple_status(w, HTTP_STATUS_NOT_FOUND);
        return;
    }

    // TODO: overwrite?
    if (dest.Exists()) {
        was_simple_status(w, HTTP_STATUS_CONFLICT);
        return;
    }

    GError *error = nullptr;
    auto parent = dest.GetParent(&error);
    if (parent.first == nullptr) {
        // TODO: improve error handling
        if (error != nullptr) {
            fprintf(stderr, "%s\n", error->message);
            g_error_free(error);
        }

        was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }

    od_resource *new_resource = src.Copy(parent.first, parent.second, &error);
    od_resource_free(parent.first);
    if (new_resource == nullptr) {
        fprintf(stderr, "%s\n", error->message);
        g_error_free(error);
        was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }

    od_resource_free(new_resource);
}

void
OnlineDriveBackend::HandleMove(was_simple *w, const Resource &src,
                               const Resource &dest)
{
    if (!src.Exists()) {
        was_simple_status(w, HTTP_STATUS_NOT_FOUND);
        return;
    }

    // TODO: overwrite?
    if (dest.Exists()) {
        was_simple_status(w, HTTP_STATUS_CONFLICT);
        return;
    }

    GError *error = nullptr;
    auto parent = dest.GetParent(&error);
    if (parent.first == nullptr) {
        // TODO: improve error handling
        if (error != nullptr) {
            fprintf(stderr, "%s\n", error->message);
            g_error_free(error);
        }

        was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }

    bool success = src.Move(parent.first, parent.second, &error);
    od_resource_free(parent.first);
    if (!success) {
        fprintf(stderr, "%s\n", error->message);
        g_error_free(error);
        was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }
}

static od_setup *
Load(const char *path, const char *group_name, GError **error_r)
{
    GKeyFile *key_file = g_key_file_new();
    if (!g_key_file_load_from_file(key_file, path, G_KEY_FILE_NONE, error_r)) {
        g_key_file_free(key_file);
        return nullptr;
    }

    od_setup *setup = od_setup_new(key_file, group_name, error_r);
    g_key_file_free(key_file);
    return setup;
}

static od_setup *
Initialize(const char *path, const char *group_name)
{
    GError *error = nullptr;
    od_setup *setup = Load(path, group_name, &error);
    if (setup == nullptr) {
        fprintf(stderr, "Failed to load %s: %s\n", path, error->message);
        g_error_free(error);
    }

    return setup;
}

int
main(int argc, const char *const*argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s PATH GROUP\n", argv[0]);
        return EXIT_FAILURE;
    }

    od_setup *setup = Initialize(argv[1], argv[2]);
    if (setup == nullptr)
        return EXIT_FAILURE;

    OnlineDriveBackend backend(setup);
    run(backend);
    return EXIT_SUCCESS;
}
