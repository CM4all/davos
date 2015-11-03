/*
 * Class for OnlineDrive resources.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "od_resource.hxx"

extern "C" {
#include <od/create.h>
#include <od/error.h>
}

struct od_resource_create *
OnlineDriveResource::CreateBegin(GError **error_r)
{
    auto parent = GetParent(error_r);
    if (parent.first == nullptr) {
        assert(error_r == nullptr || *error_r != nullptr);
        return nullptr;
    }

    auto *c = od_resource_create_begin(od_resource_get_site(parent.first),
                                       error_r);
    if (c == nullptr) {
        od_resource_free(parent.first);
        return nullptr;
    }

    bool success = od_resource_create_set_location(c, parent.first,
                                                   parent.second, true,
                                                   error_r);
    od_resource_free(parent.first);
    if (!success) {
        od_resource_create_abort(c);
        return nullptr;
    }

    return c;
}

bool
OnlineDriveResource::CreateEmpty(GError **error_r)
{
    assert(!Exists());

    auto *c = CreateBegin(error_r);
    if (c == nullptr)
        return false;

    auto *r = od_resource_create_commit(c, error_r);
    if (r == nullptr)
        return false;

    resource = r;
    return true;
}

