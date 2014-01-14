/*
 * Class for OnlineDrive resources.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_OD_RESOURCE_HXX
#define DAVOS_OD_RESOURCE_HXX

extern "C" {
#include <inline/compiler.h>
#include <od/resource.h>
}

#include <string>

#include <assert.h>

class OnlineDriveResource {
    od_resource *root;
    std::string uri;

    od_resource *resource;

public:
    OnlineDriveResource()
        :root(nullptr) {}

    OnlineDriveResource(od_resource *_root,
                        const char *_uri,
                        od_resource *_resource)
        :root(_root), uri(_uri), resource(_resource) {}

    /**
     * Construct a child resource.
     */
    OnlineDriveResource(const OnlineDriveResource &other,
                        od_resource *_resource)
        :root(od_resource_dup(other.root)),
         uri(other.uri),
         resource(_resource) {
        if (!uri.empty())
            uri.push_back('/');
        uri.append(od_resource_get_name(resource));
    }

    OnlineDriveResource(OnlineDriveResource &&other)
        :root(other.root), uri(std::move(other.uri)),
         resource(other.resource) {
        other.root = other.resource = nullptr;
    }

    ~OnlineDriveResource() {
        if (IsNull())
            return;

        if (resource != nullptr)
            od_resource_free(resource);
        od_resource_free(root);
    }

    bool IsNull() const {
        return root == nullptr;
    }

    bool Exists() const {
        assert(!IsNull());

        return resource != nullptr;
    }

    od_resource *Get() const {
        assert(Exists());

        return resource;
    }

    bool IsFile() const {
        assert(Exists());

        return od_resource_get_type(resource) == OD_TYPE_FILE;
    }

    bool IsDirectory() const {
        assert(Exists());

        return od_resource_get_type(resource) == OD_TYPE_DIRECTORY;
    }

    gcc_pure
    const char *GetId() const {
        return od_resource_get_id(resource);
    }

    gcc_pure
    const char *GetName() const {
        return od_resource_get_name(resource);
    }

    gcc_pure
    const char *GetURI() const {
        return uri.c_str();
    }

    gcc_pure
    const od_stat &GetStat() const {
        assert(Exists());

        return *od_resource_stat(resource);
    }

    bool SetStat(const od_stat &st, GError **error_r) const {
        assert(Exists());

        return od_resource_update(resource, &st, error_r);
    }

    od_resource_list *GetChildren(GError **error_r) const {
        assert(Exists());

        return od_resource_get_children(resource, error_r);
    }

    std::pair<od_resource *, const char *> GetParent(GError **error_r) const;

    od_resource *Copy(od_resource *parent, const char *name, bool replace,
                      GError **error_r) const {
        assert(Exists());

        return od_resource_copy(resource, parent, name, replace, error_r);
    }

    bool Move(od_resource *parent, const char *name, bool replace,
              GError **error_r) const {
        assert(Exists());

        return od_resource_move(resource, parent, name, replace, error_r);
    }

    bool Delete(GError **error_r) const {
        assert(Exists());

        return od_resource_delete(resource, false, error_r);
    }

    gcc_pure
    const char *GetLocalPath() const {
        assert(Exists());

        return od_resource_get_local_path(resource);
    }

    od_stream *OpenStream(GError **error_r) const {
        assert(Exists());

        return od_resource_stream_open(resource, error_r);
    }
};

#endif
