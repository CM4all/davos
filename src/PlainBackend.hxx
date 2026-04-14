// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

#pragma once

#include "file.hxx"
#include "directory.hxx"
#include "get.hxx"
#include "put.hxx"
#include "propfind.hxx"
#include "proppatch.hxx"
#include "lock.hxx"
#include "other.hxx"

struct was_simple;

class PlainBackend {
	const char *document_root;

public:
	typedef FileResource Resource;

	bool Setup(was_simple *w) noexcept;
	void TearDown() noexcept {}

	[[gnu::pure]]
	Resource Map(std::string_view uri) const noexcept;

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
