/*
 * Request handler for PUT.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "put.hxx"
#include "IfMatch.hxx"
#include "was.hxx"
#include "error.hxx"
#include "file.hxx"
#include "splice.hxx"
#include "io/FileWriter.hxx"
#include "util/PrintException.hxx"

extern "C" {
#include <was/simple.h>
}

#include <exception>

static void
HandleIfMatch(struct was_simple &was, const struct stat *st)
{
    if (!CheckIfMatch(was, st)) {
        was_simple_status(&was, HTTP_STATUS_PRECONDITION_FAILED);
        throw WasBreak();
    }
}

static void
HandleIfNoneMatch(struct was_simple &was, const struct stat *st)
{
    if (!CheckIfNoneMatch(was, st)) {
        was_simple_status(&was, HTTP_STATUS_PRECONDITION_FAILED);
        throw WasBreak();
    }
}

void
handle_put(was_simple *w, const FileResource &resource)
{
    assert(was_simple_has_body(w));

    HandleIfMatch(*w, resource.GetStatIfExists());
    HandleIfNoneMatch(*w, resource.GetStatIfExists());

    try {
        FileWriter fw(resource.GetPath());

        int64_t remaining = was_simple_input_remaining(w);
        if (remaining > 0)
            fw.Allocate(remaining);

        if (!splice_from_was(w, fw.GetFileDescriptor().Get())) {
            was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
            return;
        }

        fw.Commit();
    } catch (const std::exception &e) {
        PrintException(e);
        was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }

    was_simple_status(w, HTTP_STATUS_CREATED);
}
