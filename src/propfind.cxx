/*
 * PROPFIND implementation.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "propfind.hxx"
#include "wxml.hxx"
#include "error.hxx"
#include "file.hxx"

extern "C" {
#include "date.h"

#include <was/simple.h>
}

#include <inline/compiler.h>

#include <string>
#include <forward_list>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>

static bool
begin_multistatus(was_simple *w)
{
    return wxml_declaration(w) &&
        wxml_begin_tag(w, "D:multistatus") &&
        wxml_attribute(w, "xmlns:D", "DAV:") &&
        wxml_end_tag(w);
}

static bool
end_multistatus(was_simple *w)
{
    return wxml_close_element(w, "D:multistatus");
}

static bool
href(was_simple *w, const char *uri)
{
    return wxml_open_element(w, "D:href") &&
        was_simple_puts(w, uri) &&
        wxml_close_element(w, "D:href");
}

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
propfind_file(was_simple *was, std::string &uri, std::string &path,
              const struct stat &st,
              unsigned depth)
{
    bool success = wxml_open_element(was, "D:response") &&
        href(was, uri.c_str()) &&
        wxml_open_element(was, "D:propstat") &&
        wxml_string_element(was, "D:status", "HTTP/1.1 200 OK") &&
        wxml_open_element(was, "D:prop");
    if (!success)
        return false;

    if (S_ISDIR(st.st_mode)) {
        success = wxml_open_element(was, "D:resourcetype") &&
            wxml_short_element(was, "D:collection") &&
            wxml_close_element(was, "D:resourcetype");
        if (!success)
            return false;
    } else if (S_ISREG(st.st_mode)) {
        success = wxml_open_element(was, "D:getcontentlength") &&
            was_simple_printf(was, "%llu", (unsigned long long)st.st_size) &&
            wxml_close_element(was, "D:getcontentlength");
        if (!success)
            return false;
    }

    success = wxml_open_element(was, "D:getlastmodified") &&
        was_simple_puts(was, http_date_format(st.st_mtime)) &&
        wxml_close_element(was, "D:getlastmodified");
    if (!success)
            return false;

    success = wxml_close_element(was, "D:prop") &&
        wxml_close_element(was, "D:propstat") &&
        wxml_close_element(was, "D:response");
    if (!success)
        return false;

    const auto uri_length = uri.length();
    const auto path_length = path.length();

    if (depth > 0 && S_ISDIR(st.st_mode)) {
        --depth;

        const auto &children = ListDirectory(path.c_str());
        for (const std::string &name : children) {
            uri.push_back('/');
            uri.append(name);
            path.push_back('/');
            path.append(name);

            struct stat st2;
            success = true;
            if (stat(path.c_str(), &st2) == 0) {
                success = propfind_file(was, uri, path, st2, depth);
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
        errno_respones(was, resource.GetError());
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

    if (!begin_multistatus(was))
        return;

    std::string uri2(uri);
    std::string path2(resource.GetPath());
    if (!propfind_file(was, uri2, path2, resource.GetStat(), depth))
        return;

    end_multistatus(was);
}
