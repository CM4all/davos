// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

#include "PlainBackend.hxx"
#include "Chrono.hxx"
#include "error.hxx"
#include "http/Date.hxx"

#include <was/simple.h>

bool
PlainBackend::Setup(was_simple *w) noexcept
{
	document_root = was_simple_get_parameter(w, "DAVOS_DOCUMENT_ROOT");
	if (document_root == nullptr) {
		fprintf(stderr, "No DAVOS_DOCUMENT_ROOT\n");
		return false;
	}

	return true;
}

PlainBackend::Resource
PlainBackend::Map(std::string_view uri) const noexcept
{
	std::string path(document_root);

	if (!uri.empty()) {
		path.push_back('/');
		path.append(uri);
	}

	return Resource(std::move(path));
}

void
PlainBackend::HandleProppatch(was_simple *w, const char *uri,
			       Resource &resource)
{
	if (!resource.Exists()) {
		errno_response(w, resource.GetError());
		return;
	}

	ProppatchMethod method;
	if (!method.ParseRequest(w))
		return;

	struct timeval times[2] = {
		ToTimeval(resource.GetAccessTime()),
		ToTimeval(resource.GetModificationTime()),
	};

	bool times_enabled = false;
	http_status_t times_status = HTTP_STATUS_NOT_FOUND;

	for (auto &prop : method.GetProps()) {
		if (prop.IsWin32LastAccessTime()) {
			if (!prop.ParseWin32Timestamp(times[0])) {
				prop.status = HTTP_STATUS_BAD_REQUEST;
				continue;
			}

			times_enabled = true;
		} else if (prop.IsWin32LastModifiedTime()) {
			if (!prop.ParseWin32Timestamp(times[1])) {
				prop.status = HTTP_STATUS_BAD_REQUEST;
				continue;
			}

			times_enabled = true;
		} else if (prop.IsGetLastModified()) {
			const auto t = http_date_parse(prop.value.c_str());
			if (t < std::chrono::system_clock::time_point()) {
				prop.status = HTTP_STATUS_BAD_REQUEST;
				continue;
			}

			times[1].tv_sec = std::chrono::system_clock::to_time_t(t);
			times[1].tv_usec = 0;
			times_enabled = true;
		}
	}

	if (times_enabled)
		times_status = utimes(resource.GetPath(), times) == 0
			? HTTP_STATUS_OK
			: errno_status(errno);

	for (auto &prop : method.GetProps()) {
		if ((prop.IsGetLastModified() ||
		     prop.IsWin32LastAccessTime() || prop.IsWin32LastModifiedTime()) &&
		    prop.status == HTTP_STATUS_NOT_FOUND)
			prop.status = times_status;
	}

	method.SendResponse(w, uri);
}

void
PlainBackend::HandleLock(was_simple *w, Resource &resource)
{
	LockMethod method;
	if (!method.ParseRequest(w))
		return;

	bool created = false;

	if (!resource.Exists()) {
		int e = resource.GetError();
		if (e == ENOENT) {
			/* RFC4918 9.10.4: "A successful LOCK method MUST result
			   in the creation of an empty resource that is locked
			   (and that is not a collection) when a resource did not
			   previously exist at that URL". */
			e = resource.CreateExclusive();
			if (e != 0) {
				if (e == EEXIST || e == EISDIR) {
					/* the file/directory has been created by somebody
					   else meanwhile */
				} else if (e == ENOENT || e == ENOTDIR) {
					was_simple_status(w, HTTP_STATUS_CONFLICT);
					return;
				} else {
					errno_response(w, e);
					return;
				}
			} else {
				created = true;
			}
		} else if (e == ENOTDIR) {
			was_simple_status(w, HTTP_STATUS_CONFLICT);
			return;
		} else {
			errno_response(w, e);
			return;
		}
	}

	method.Run(w, created);
}
