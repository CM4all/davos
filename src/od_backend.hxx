/*
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_OD_BACKEND_HXX
#define DAVOS_OD_BACKEND_HXX

#include <inline/compiler.h>

struct was_simple;
struct od_setup;
struct od_site;
class OnlineDriveResource;

class OnlineDriveBackend {
    od_setup *const setup;
    od_site *site;

public:
    typedef OnlineDriveResource Resource;

    OnlineDriveBackend(od_setup *_setup)
        :setup(_setup), site(nullptr) {}
    ~OnlineDriveBackend();

    bool Setup(was_simple *w);
    void TearDown();

    gcc_pure
    Resource Map(const char *uri) const;

    void HandleHead(was_simple *w, const Resource &resource);
    void HandleGet(was_simple *w, const Resource &resource);
    void HandlePut(was_simple *w, const Resource &resource);
    void HandleDelete(was_simple *w, const Resource &resource);

    void HandlePropfind(was_simple *w, const char *uri,
                        const Resource &resource);
    void HandleProppatch(was_simple *w, const char *uri,
                         const Resource &resource);

    void HandleMkcol(was_simple *w, const Resource &resource);
    void HandleCopy(was_simple *w, const Resource &src, const Resource &dest);
    void HandleMove(was_simple *w, const Resource &src, const Resource &dest);
    void HandleLock(was_simple *w, const Resource &resource);
};

#endif
