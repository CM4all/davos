// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

#include "frontend.hxx"
#include "error.hxx"
#include "directory.hxx"
#include "get.hxx"
#include "put.hxx"
#include "propfind.hxx"
#include "proppatch.hxx"
#include "lock.hxx"
#include "other.hxx"
#include "file.hxx"
#include "PivotRoot.hxx"
#include "IsolatePath.hxx"
#include "Chrono.hxx"
#include "http/Date.hxx"
#include "util/PrintException.hxx"

#include <cerrno>
#include <stdexcept>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

class SimpleBackend {
	const char *document_root;

public:
	typedef FileResource Resource;

	bool Setup(was_simple *w);
	void TearDown() {}

	[[gnu::pure]]
	Resource Map(const char *uri) const;

	void HandleHead(was_simple *w, const Resource &resource) {
		handle_head(w, resource);
	}

	void HandleGet(was_simple *w, const Resource &resource) {
		handle_get(w, resource);
	}

	void HandlePut(was_simple *w, Resource &resource) {
		handle_put(w, resource);
	}

	void HandleDelete(was_simple *w, Resource &resource) {
		handle_delete(w, resource);
	}

	void HandlePropfind(was_simple *w, const char *uri,
			    const Resource &resource) {
		handle_propfind(w, uri, resource);
	}

	void HandleProppatch(was_simple *w, const char *uri,
			     Resource &resource);

	void HandleMkcol(was_simple *w, Resource &resource) {
		handle_mkcol(w, resource);
	}

	void HandleCopy(was_simple *w, const Resource &src, Resource &dest) {
		handle_copy(w, src, dest);
	}

	void HandleMove(was_simple *w, Resource &src, Resource &dest) {
		handle_move(w, src, dest);
	}

	void HandleLock(was_simple *w, Resource &resource);
};

inline bool
SimpleBackend::Setup(was_simple *w)
{
	document_root = was_simple_get_parameter(w, "DAVOS_DOCUMENT_ROOT");
	if (document_root == nullptr) {
		fprintf(stderr, "No DAVOS_DOCUMENT_ROOT\n");
		return false;
	}

	return true;
}

inline SimpleBackend::Resource
SimpleBackend::Map(const char *uri) const
{
	std::string path(document_root);

	if (*uri != 0) {
		path.push_back('/');
		path.append(uri);
	}

	return Resource(std::move(path));
}

void
SimpleBackend::HandleProppatch(was_simple *w, const char *uri,
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
SimpleBackend::HandleLock(was_simple *w, Resource &resource)
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

static void
MaybePivotRoot()
{
	const char *new_root = getenv("DAVOS_PIVOT_ROOT");
	if (new_root == nullptr)
		return;

	const char *put_old = getenv("DAVOS_PIVOT_ROOT_OLD");
	if (put_old == nullptr)
		throw std::runtime_error("DAVOS_PIVOT_ROOT_OLD missing");

	if (*put_old == '/' || *put_old == 0)
		throw std::runtime_error("DAVOS_PIVOT_ROOT_OLD must be a relative path");

	PivotRoot(new_root, put_old);
}

static void
MaybeIsolatePath()
{
	const char *path = getenv("DAVOS_ISOLATE_PATH");
	if (path == nullptr)
		return;

	if (*path != '/')
		throw std::runtime_error("DAVOS_ISOLATE_PATH must be an absolute path");

	IsolatePath(path);
}

int
main(int, const char *const*)
try {
	MaybePivotRoot();
	MaybeIsolatePath();

	SimpleBackend backend;
	run(backend);
	return EXIT_SUCCESS;
} catch (...) {
	PrintException(std::current_exception());
	return EXIT_FAILURE;
}
