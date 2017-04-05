/*
 * Request handler for PUT.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "put.hxx"
#include "error.hxx"
#include "file.hxx"
#include "splice.hxx"
#include "io/FileWriter.hxx"
#include "util/PrintException.hxx"

extern "C" {
#include <was/simple.h>
}

#include <exception>

void
handle_put(was_simple *w, const FileResource &resource)
{
    assert(was_simple_has_body(w));

    try {
        FileWriter fw(resource.GetPath());

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
