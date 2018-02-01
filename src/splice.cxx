/*
 * Wrapper for libwas and splice().
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "splice.hxx"

extern "C" {
#include <was/simple.h>
}

#include <limits>
#include <algorithm>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

bool
splice_from_was(was_simple *w, int out_fd)
{
    const int in_fd = was_simple_input_fd(w);

    while (true) {
        enum was_simple_poll_result result = was_simple_input_poll(w, -1);
        switch (result) {
        case WAS_SIMPLE_POLL_SUCCESS:
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

        auto nbytes = splice(in_fd, nullptr, out_fd, nullptr,
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
    }
}

bool
splice_to_was(was_simple *w, int in_fd, uint64_t remaining)
{
    if (!was_simple_set_length(w, remaining))
        return false;

    if (remaining == 0)
        return true;

    const int out_fd = was_simple_output_fd(w);
    while (remaining > 0) {
        constexpr uint64_t max = std::numeric_limits<size_t>::max();
        size_t length = std::min(remaining, max);
        ssize_t nbytes = splice(in_fd, nullptr, out_fd, nullptr,
                                length,
                                SPLICE_F_MOVE|SPLICE_F_MORE);
        if (nbytes <= 0) {
            if (nbytes < 0)
                fprintf(stderr, "splice() failed: %s\n", strerror(errno));
            return false;
        }

        if (!was_simple_sent(w, nbytes))
            return false;

        remaining -= nbytes;
    }

    return true;
}
