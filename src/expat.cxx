/*
 * expat wrapper.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "expat.hxx"

extern "C" {
#include <was/simple.h>
}

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

bool
ExpatParser::Parse(was_simple *w)
{
    if (!was_simple_has_body(w))
        return false;

    while (true) {
        char buffer[4096];

        enum was_simple_poll_result result = was_simple_input_poll(w, -1);
        switch (result) {
            ssize_t nbytes;

        case WAS_SIMPLE_POLL_SUCCESS:
            nbytes = was_simple_input_read(w, buffer, sizeof(buffer));
            if (nbytes < 0) {
                if (errno == EAGAIN)
                    continue;

                fprintf(stderr, "Error reading HTTP request body: %s\n",
                        strerror(errno));
                return false;
            }

            if (nbytes == 0) {
                fprintf(stderr, "Unexpected end of HTTP request body\n");
                return false;
            }

            if (XML_Parse(parser, buffer, nbytes, false) != XML_STATUS_OK) {
                fprintf(stderr, "XML parser failed on HTTP request body\n");
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
            if (XML_Parse(parser, "", 0, true) != XML_STATUS_OK) {
                fprintf(stderr, "XML parser failed on HTTP request body\n");
                return false;
            }

            return true;

        case WAS_SIMPLE_POLL_CLOSED:
            fprintf(stderr, "Client has closed the HTTP request body.\n");
            return false;
        }
    }
}
