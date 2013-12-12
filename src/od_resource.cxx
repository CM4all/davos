/*
 * Class for OnlineDrive resources.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "od_resource.hxx"

std::pair<od_resource *, const char *>
OnlineDriveResource::GetParent(GError **error_r) const
{
    assert(!IsNull());

    if (uri.empty())
        return { nullptr, nullptr };

    auto slash = uri.find_last_of('/');
    if (slash == uri.npos)
        return { od_resource_dup(root), uri.c_str() };

    const std::string parent_uri(uri.data(), slash);
    return { od_resource_lookup(root, parent_uri.c_str(), error_r),
            uri.c_str() + slash + 1 };
}
