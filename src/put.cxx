// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * Request handler for PUT.
 */

#include "put.hxx"
#include "IfMatch.hxx"
#include "error.hxx"
#include "file.hxx"
#include "was/ExceptionResponse.hxx"
#include "was/Splice.hxx"
#include "io/FileWriter.hxx"
#include "util/PrintException.hxx"

extern "C" {
#include <was/simple.h>
}

#include <exception>

static void
HandleIfMatch(struct was_simple &was, const struct statx *st)
{
	if (!CheckIfMatch(was, st)) {
		was_simple_status(&was, HTTP_STATUS_PRECONDITION_FAILED);
		throw Was::EndResponse{};
	}
}

static void
HandleIfNoneMatch(struct was_simple &was, const struct statx *st)
{
	if (!CheckIfNoneMatch(was, st)) {
		was_simple_status(&was, HTTP_STATUS_PRECONDITION_FAILED);
		throw Was::EndResponse{};
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

		if (int64_t remaining = was_simple_input_remaining(w);
		    remaining > 0)
			fw.Allocate(remaining);

		if (!SpliceFromWas(w, fw.GetFileDescriptor())) {
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
