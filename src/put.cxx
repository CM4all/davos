/*
 * Request handler for PUT.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "put.hxx"
#include "error.hxx"
#include "file.hxx"
#include "splice.hxx"

extern "C" {
#include <was/simple.h>
}

#include <string>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

static std::string
make_tmp_path(const char *path)
{
    // TODO
    std::string s(path);
    s.append(".tmp");
    return s;
}

void
handle_put(was_simple *w, const FileResource &resource)
{
    assert(was_simple_has_body(w));

    const std::string tmp_path = make_tmp_path(resource.GetPath());

    int file_fd = open(tmp_path.c_str(), O_CREAT|O_EXCL|O_WRONLY, 0666);
    if (file_fd < 0) {
        fprintf(stderr, "Failed to create %s: %s\n",
                tmp_path.c_str(), strerror(errno));
        was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }

    if (!splice_from_was(w, file_fd)) {
        close(file_fd);
        unlink(tmp_path.c_str());
        was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }

    if (close(file_fd) < 0) {
        fprintf(stderr, "Failed to write %s: %s\n",
                tmp_path.c_str(), strerror(errno));
        unlink(tmp_path.c_str());
        was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }

    if (rename(tmp_path.c_str(), resource.GetPath()) < 0) {
        fprintf(stderr, "Failed to commit %s: %s\n",
                resource.GetPath(), strerror(errno));
        unlink(tmp_path.c_str());
        was_simple_status(w, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return;
    }

    was_simple_status(w, HTTP_STATUS_CREATED);
}
