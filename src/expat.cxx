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
		ssize_t nbytes = was_simple_read(w, buffer, sizeof(buffer));
		if (nbytes < 0) {
			fprintf(stderr, "Error reading HTTP request body\n");
			return false;
		}

		if (nbytes == 0) {
			if (XML_Parse(parser, "", 0, true) != XML_STATUS_OK) {
				fprintf(stderr, "XML parser failed on HTTP request body\n");
				return false;
			}

			return true;
		}

		if (XML_Parse(parser, buffer, nbytes, false) != XML_STATUS_OK) {
			fprintf(stderr, "XML parser failed on HTTP request body\n");
			return false;
		}
	}
}
