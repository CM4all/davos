/*
 * OnlineDrive PUT handler.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "od_backend.hxx"
#include "od_resource.hxx"
#include "mime_types.hxx"
#include "splice.hxx"

extern "C" {
#include <was/simple.h>
#include <od/resource.h>
#include <od/create.h>
#include <od/stat.h>
}

#include <assert.h>
#include <stdio.h>
#include <string.h>

static bool
SendStat(od_resource_create *c, const char *name, GError **error_r)
{
    od_stat st;
    memset(&st, 0, sizeof(st));

    const std::string content_type = LookupMimeTypeByFileName(name);
    st.content_type = content_type.empty()
        ? "application/octet-stream"
        : content_type.c_str();

    st.size = OD_SIZE_UNKNOWN;

    return od_resource_create_set_stat(c, &st, error_r);
}

void
OnlineDriveBackend::HandlePut(was_simple *w, Resource &resource)
{
    assert(was_simple_has_body(w));

    if (resource.Exists() && !resource.IsFile()) {
        was_simple_status(w, HTTP_STATUS_CONFLICT);
        return;
    }

    GError *error = nullptr;
    auto c = resource.CreateBegin(&error);
    if (c == nullptr) {
        fprintf(stderr, "%s\n", error->message);
        g_error_free(error);
        was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }

    if (!SendStat(c, resource.GetName2(), &error)) {
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
