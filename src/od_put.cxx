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
#include <od/error.h>
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

static bool
PutDataSlow(struct od_resource_create *c, struct was_simple *w)
{
    while (true) {
        auto p = was_simple_input_poll(w, -1);
        switch (p) {
        case WAS_SIMPLE_POLL_SUCCESS:
            break;

        case WAS_SIMPLE_POLL_END:
            return true;

        case WAS_SIMPLE_POLL_ERROR:
        case WAS_SIMPLE_POLL_TIMEOUT:
        case WAS_SIMPLE_POLL_CLOSED:
            return false;
        }

        char buffer[8192];
        ssize_t nbytes = was_simple_read(w, buffer, sizeof(buffer));
        if (gcc_unlikely(nbytes <= 0)) {
            if (nbytes == -1)
                fprintf(stderr, "Failed to read from WAS pipe: %s\n",
                        strerror(errno));

            return nbytes == 0;
        }

        GError *error = nullptr;
        if (!od_resource_create_write(c, buffer, nbytes, &error)) {
            fprintf(stderr, "%s\n", error->message);
            g_error_free(error);
            return false;
        }
    }
}

static bool
PutData(struct od_resource_create *c, struct was_simple *w)
{
    GError *error = nullptr;
    int fd = od_resource_create_open(c, &error);
    if (fd < 0) {
        if (error->domain == od_error_domain() &&
            error->code == OD_ERROR_NOT_IMPLEMENTED)
            /* libod data module doesn't support file descriptor -
               fall back to "slow" transfer using was_simple_read()
               and od_resource_create_write() */
            return PutDataSlow(c, w);

        fprintf(stderr, "%s\n", error->message);
        g_error_free(error);
        return false;
    }

    return splice_from_was(w, fd);
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

    if (!PutData(c, w)) {
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
