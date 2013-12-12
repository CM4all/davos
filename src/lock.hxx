/*
 * Fake LOCK/UNLOCK implementation.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_LOCK_HXX
#define DAVOS_LOCK_HXX

#include <string>

struct was_simple;

struct LockParserData {
    enum State {
        ROOT,
        OWNER,
        OWNER_HREF,
    } state;

    std::string owner_href;

    LockParserData():state(ROOT) {}
};

class LockMethod {
    LockParserData data;

public:
    bool ParseRequest(was_simple *w);
    void Run(was_simple *w, bool created);
};

#endif
