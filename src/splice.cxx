/*
 * Wrapper for libwas and splice().
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "splice.hxx"

extern "C" {
#include <was/simple.h>
}

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
