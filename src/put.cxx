/*
 * Request handler for PUT.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "put.hxx"
#include "error.hxx"
#include "file.hxx"

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

static bool
splice_from_was(was_simple *w, int out_fd)
{
    const int in_fd = was_simple_input_fd(w);

    while (true) {
        enum was_simple_poll_result result = was_simple_input_poll(w, -1);
        switch (result) {
            ssize_t nbytes;

        case WAS_SIMPLE_POLL_SUCCESS:
            nbytes = splice(in_fd, nullptr, out_fd, nullptr,
                            1 << 30,
                            SPLICE_F_MOVE|SPLICE_F_NONBLOCK|SPLICE_F_MORE);
            if (nbytes < 0) {
                if (errno == EAGAIN)
                    continue;

                fprintf(stderr, "Error copying HTTP request body: %s\n",
                        strerror(errno));
                return false;
            }

            if (!was_simple_received(w, nbytes)) {
                fprintf(stderr, "Error receiving HTTP request body\n");
                return false;
            }

            break;

        case WAS_SIMPLE_POLL_ERROR:
            fprintf(stderr, "Error reading HTTP request body.\n");
            return false;

        case WAS_SIMPLE_POLL_TIMEOUT:
            fprintf(stderr, "Timeout reading HTTP request body.\n");
            return false;

        case WAS_SIMPLE_POLL_END:
            return true;

        case WAS_SIMPLE_POLL_CLOSED:
            fprintf(stderr, "Client has closed the PUT request body.\n");
            return false;
        }
    }
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
