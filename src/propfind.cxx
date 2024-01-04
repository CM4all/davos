/*
 * PROPFIND implementation.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "propfind.hxx"
#include "uri_escape.hxx"
#include "wxml.hxx"
#include "error.hxx"
#include "file.hxx"
#include "Chrono.hxx"
#include "was/WasOutputStream.hxx"
#include "http/Date.hxx"
#include "io/DirectoryReader.hxx"
#include "util/Compiler.h"

#include <was/simple.h>

#include <string>
#include <forward_list>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>

static std::size_t MAX_FILES = 2000;
static unsigned MAX_DEPTH = 3;

static std::forward_list<std::string>
ListDirectory(const char *path)
try {
	std::forward_list<std::string> result;
	std::size_t n = 0;
	DirectoryReader r{path};
	while (const char *name = r.Read()) {
		if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
			continue;

		result.emplace_front(name);
		if (++n >= MAX_FILES)
			break;
	}

	return result;
} catch (...) {
	return {};
}

static void
propfind_file(BufferedOutputStream &o, std::string &uri, std::string &path,
	      const struct statx &st,
	      unsigned depth)
{
	open_response_prop(o, uri, "HTTP/1.1 200 OK");

	if (S_ISDIR(st.stx_mode)) {
		resourcetype_collection(o);
	} else if (S_ISREG(st.stx_mode)) {
		wxml_fmt_element(o, "D:getcontentlength", "{}", st.stx_size);
	}

	const auto mtime = ToSystemTime(st.stx_mtime);

	wxml_string_element(o, "D:getlastmodified", http_date_format(mtime));

	close_response_prop(o);

	if (depth > 0 && S_ISDIR(st.stx_mode)) {
		--depth;

		if (uri.back() != '/')
			/* directory URIs should end with a slash - but we don't
			   enforce that everywhere; fix up the URI if the
			   user-supplied URI doesn't */
			uri.push_back('/');

		const auto uri_length = uri.length();

		const auto &children = ListDirectory(path.c_str());

		path.push_back('/');
		const auto path_length = path.length();

		for (const std::string &name : children) {
			AppendUriEscape(uri, name.c_str());
			path.append(name);

			struct statx st2;
			if (statx(-1, path.c_str(), AT_STATX_SYNC_AS_STAT,
				  STATX_TYPE|STATX_MTIME|STATX_SIZE,
				  &st2) == 0) {
				if (S_ISDIR(st2.stx_mode))
					/* directory URIs should end with a slash */
					uri.push_back('/');

				propfind_file(o, uri, path, st2, depth);
			}

			uri.erase(uri_length);
			path.erase(path_length);
		}
	}
}

void
handle_propfind(was_simple *was, const char *uri, const FileResource &resource)
{
	if (!resource.Exists()) {
		errno_response(was, resource.GetError());
		return;
	}

	unsigned depth = 0;
	const char *depth_string = was_simple_get_header(was, "depth");
	if (depth_string != nullptr)
		depth = strtoul(depth_string, nullptr, 10);
	if (depth > MAX_DEPTH)
		depth = MAX_DEPTH;

	if (!was_simple_status(was, HTTP_STATUS_MULTI_STATUS) ||
	    !was_simple_set_header(was, "content-type",
				   "text/xml; charset=\"utf-8\""))
		return;

	WasOutputStream wos{was};
	BufferedOutputStream bos{wos};

	begin_multistatus(bos);

	std::string uri2(uri);
	std::string path2(resource.GetPath());
	propfind_file(bos, uri2, path2, resource.GetStat(), depth);
	end_multistatus(bos);

	bos.Flush();
}
