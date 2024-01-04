/*
 * WebDAV server.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "was/Loop.hxx"
#include "was/WasOutputStream.hxx"
#include "util/UriEscape.hxx"
#include "util/LightString.hxx"
#include "util/ScopeExit.hxx"
#include "util/StringCompare.hxx"

#include <string>
#include <string_view>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef __linux
#include <sys/prctl.h>

#ifndef PR_SET_NO_NEW_PRIVS
#define PR_SET_NO_NEW_PRIVS 38
#endif
#endif

static const char *dav_header;

static std::string_view mountpoint;

template<typename Backend>
static void
handle_options(was_simple *was, const typename Backend::Resource &resource)
{
	const char *allow_new = "OPTIONS,MKCOL,PUT,LOCK";
	const char *allow_file =
		"OPTIONS,GET,HEAD,DELETE,PROPFIND,PROPPATCH,COPY,MOVE,PUT,LOCK,UNLOCK";
	const char *allow_directory =
		"OPTIONS,DELETE,PROPFIND,PROPPATCH,COPY,MOVE,LOCK,UNLOCK";

	const char *allow;
	if (!resource.Exists())
		allow = allow_new;
	else if (resource.IsDirectory())
		allow = allow_directory;
	else
		allow = allow_file;

	was_simple_set_header(was, "allow", allow);

	/* RFC 4918 10.1 */
	was_simple_set_header(was, "dav", dav_header);
}

/**
 * Extract the URI path from an absolute URL.
 */
static const char *
get_uri_path(const char *p)
{
	assert(p != nullptr);

	if (memcmp(p, "http://", 7) == 0)
		p += 7;
	else if (memcmp(p, "https://", 8) == 0)
		p += 8;
	else
		return p;

	const char *slash = strchr(p, '/');
	if (slash == nullptr)
		/* there is no URI path - assume it's just "/" */
		return "/";

	return slash;
}

/**
 * An exception type which gets thrown when the URI is malformed.
 */
struct MalformedUri {};

/**
 * An exception type which gets thrown when the URI is outside of the
 * current mount point.
 */
struct OutsideUri {};

/**
 * Throws #MalformedUri or #OutsideUri on error.
 */
template<class Backend>
static typename Backend::Resource
map_uri(const Backend &backend, const char *uri)
{
	assert(uri != nullptr);

	const LightString unescaped = UriUnescape(uri);
	if (unescaped.IsNull())
		throw MalformedUri();

	uri = unescaped.c_str();

	if (strstr(uri, "/../") != nullptr)
		throw MalformedUri();

	if (StringStartsWith(uri, mountpoint))
		uri += mountpoint.size();
	else if (StringStartsWith(uri, mountpoint.substr(0, mountpoint.size() - 1)) &&
		 uri[mountpoint.size() - 1] == 0)
		/* special case for clients that remove the trailing slash
		   (e.g. Microsoft) */
		uri = "";
	else
		throw OutsideUri();

	/* strip trailing slash */
	std::string uri2(uri);
	if (!uri2.empty() && uri2.back() == '/')
		uri2.pop_back();

	return backend.Map(uri2.c_str());
}

static bool
configure_umask(was_simple *w)
{
	const char *p = was_simple_get_parameter(w, "DAVOS_UMASK");
	if (p == nullptr)
		p = "0022";

	char *endptr;
	unsigned long value = strtoul(p, &endptr, 8);
	if (endptr == p || *endptr != 0 || value == 0 || (value & ~0777) != 0) {
		fprintf(stderr, "Malformed DAVOS_UMASK\n");
		return false;
	}

	static unsigned long old = -1;
	if (value != old) {
		old = value;
		umask(value);
	}

	return true;
}

static constexpr bool
check_mountpoint(std::string_view s) noexcept
{
	return !s.empty() && s.front() == '/' && s.back() == '/';
}

static bool
configure_mapper(was_simple *w)
{
	const char *m = was_simple_get_parameter(w, "DAVOS_MOUNT");
	if (m == nullptr) {
		fprintf(stderr, "No DAVOS_MOUNT\n");
		return false;
	}

	mountpoint = m;
	if (!check_mountpoint(mountpoint)) {
		fprintf(stderr, "Malformed DAVOS_MOUNT\n");
		return false;
	}

	return true;
}

static bool
configure_dav_header(was_simple *w)
{
	dav_header = was_simple_get_parameter(w, "DAVOS_DAV_HEADER");
	if (dav_header == nullptr)
		dav_header = "1";

	return true;
}

template<typename Backend>
static bool
configure(Backend &backend, was_simple *w)
{
	return configure_umask(w) && configure_mapper(w) &&
		configure_dav_header(w) &&
		backend.Setup(w);
}

[[gnu::pure]]
static bool
HasTrailingSlash(const char *uri)
{
	const size_t length = strlen(uri);
	assert(length > 0);
	return uri[length - 1] == '/';
}

template<typename Backend>
static void
run2(Backend &backend, was_simple *was, const char *uri)
try {
	auto resource = map_uri(backend, uri);

	const http_method_t method = was_simple_get_method(was);

	if (HasTrailingSlash(uri)) {
		if (method == HTTP_METHOD_PUT) {
			/* a trailing slash is not allowed for (new) regular
			   file */
			was_simple_status(was, HTTP_STATUS_METHOD_NOT_ALLOWED);
			return;
		} else if (resource.Exists() && !resource.IsDirectory()) {
			/* trailing slash is only allowed (and obligatory) for
			   directories (collections) */
			was_simple_status(was, HTTP_STATUS_NOT_FOUND);
			return;
		}
	}

	switch (method) {
	case HTTP_METHOD_OPTIONS:
		if (!was_simple_input_close(was))
			return;

		handle_options<Backend>(was, resource);
		break;

	case HTTP_METHOD_HEAD:
		if (!was_simple_input_close(was))
			return;

		backend.HandleHead(was, resource);
		break;

	case HTTP_METHOD_GET:
		if (!was_simple_input_close(was))
			return;

		backend.HandleGet(was, resource);
		break;

	case HTTP_METHOD_PUT:
		if (!was_simple_has_body(was)) {
			was_simple_status(was, HTTP_STATUS_BAD_REQUEST);
			return;
		}

		backend.HandlePut(was, resource);
		break;

	case HTTP_METHOD_DELETE:
		if (!was_simple_input_close(was))
			return;

		backend.HandleDelete(was, resource);
		break;

	case HTTP_METHOD_PROPFIND:
		/* TODO: parse request body */
		if (!was_simple_input_close(was))
			return;

		backend.HandlePropfind(was, uri, resource);
		break;

	case HTTP_METHOD_PROPPATCH:
		backend.HandleProppatch(was, uri, resource);
		break;

	case HTTP_METHOD_MKCOL:
		if (!was_simple_input_close(was))
			return;

		backend.HandleMkcol(was, resource);
		break;

	case HTTP_METHOD_COPY: {
		if (!was_simple_input_close(was))
			return;

		const char *p = was_simple_get_header(was, "destination");
		if (p == nullptr) {
			was_simple_status(was, HTTP_STATUS_BAD_REQUEST);
			return;
		}

		p = get_uri_path(p);

		auto destination = map_uri(backend, p);
		backend.HandleCopy(was, resource, destination);
	}
		break;

	case HTTP_METHOD_MOVE: {
		if (!was_simple_input_close(was))
			return;

		const char *p = was_simple_get_header(was, "destination");
		if (p == nullptr) {
			was_simple_status(was, HTTP_STATUS_BAD_REQUEST);
			return;
		}

		p = get_uri_path(p);

		auto destination = map_uri(backend, p);
		backend.HandleMove(was, resource, destination);
	}
		break;

	case HTTP_METHOD_LOCK:
		backend.HandleLock(was, resource);
		break;

	case HTTP_METHOD_UNLOCK:
		/* no-op */
		break;

	default:
		was_simple_status(was, HTTP_STATUS_METHOD_NOT_ALLOWED);
	}
} catch (WasOutputStream::WriteFailed) {
} catch (MalformedUri) {
	was_simple_status(was, HTTP_STATUS_BAD_REQUEST);
} catch (OutsideUri) {
	/* can't copy/move the file out of its site */
	was_simple_status(was, HTTP_STATUS_FORBIDDEN);
}

template<typename Backend>
static void
run(Backend &backend, was_simple *was, const char *uri)
{
	if (!configure(backend, was)) {
		was_simple_status(was, HTTP_STATUS_INTERNAL_SERVER_ERROR);
		return;
	}

	AtScopeExit(&backend) {
		backend.TearDown();
	};

	run2(backend, was, uri);
}

template<typename Backend>
static void
run(Backend &backend)
{
#ifdef __linux
	prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
#endif

	WasLoop([&backend](struct was_simple *was, const char *uri){
		run(backend, was, uri);
	});
}
