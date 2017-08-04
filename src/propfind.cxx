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
#include "http/Date.hxx"
#include "util/Compiler.h"

#include <was/simple.h>

#include <string>
#include <forward_list>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>

static std::forward_list<std::string>
ListDirectory(const char *path)
{
    std::forward_list<std::string> result;
    DIR *dir = opendir(path);
    if (gcc_likely(dir != nullptr)) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != nullptr) {
            const char *name = ent->d_name;
            if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0)
                result.emplace_front(name);
        }
    }

    return result;
}

static bool
propfind_file(Writer &writer, std::string &uri, std::string &path,
              const struct stat &st,
              unsigned depth)
{
    if (!open_response_prop(writer, uri.c_str(), "HTTP/1.1 200 OK"))
        return false;

    if (S_ISDIR(st.st_mode)) {
        if (!resourcetype_collection(writer))
            return false;
    } else if (S_ISREG(st.st_mode)) {
        if (!wxml_format_element(writer, "D:getcontentlength", "%llu",
                                 (unsigned long long)st.st_size))
            return false;
    }

    const auto mtime = ToSystemTime(st.st_mtim);

    if (!wxml_string_element(writer, "D:getlastmodified",
                             http_date_format(mtime)))
        return false;

    if (!close_response_prop(writer))
        return false;

    if (depth > 0 && S_ISDIR(st.st_mode)) {
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

            struct stat st2;
            bool success = true;
            if (stat(path.c_str(), &st2) == 0) {
                if (S_ISDIR(st2.st_mode))
                    /* directory URIs should end with a slash */
                    uri.push_back('/');

                success = propfind_file(writer, uri, path, st2, depth);
            }

            uri.erase(uri_length);
            path.erase(path_length);

            if (!success)
                return false;
        }
    }

    return true;
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

    if (!was_simple_status(was, HTTP_STATUS_MULTI_STATUS) ||
        !was_simple_set_header(was, "content-type",
                               "text/xml; charset=\"utf-8\""))
        return;

    Writer writer(was);
    if (!begin_multistatus(writer))
        return;

    std::string uri2(uri);
    std::string path2(resource.GetPath());
    if (!propfind_file(writer, uri2, path2, resource.GetStat(), depth))
        return;

    end_multistatus(writer);
}
