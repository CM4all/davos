// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * PROPPATCH implementation.
 */

#pragma once

extern "C" {
#include <http/status.h>
}

#include <string>
#include <list>

struct was_simple;
struct timeval;

struct PropNameValue {
	std::string name;
	std::string value;
	http_status_t status;

	PropNameValue(const char *_name)
		:name(_name),
		 status(HTTP_STATUS_NOT_FOUND) {}

	[[gnu::pure]]
	bool IsGetLastModified() const {
		return name == "DAV:|getlastmodified";
	}

	[[gnu::pure]]
	bool IsWin32LastAccessTime() const {
		return name == "urn:schemas-microsoft-com:|Win32LastAccessTime";
	}

	[[gnu::pure]]
	bool IsWin32LastModifiedTime() const {
		return name == "urn:schemas-microsoft-com:|Win32LastModifiedTime";
	}

	bool ParseWin32Timestamp(timeval &tv) const;
};

struct ProppatchParserData {
	enum State {
		ROOT,
		PROP,
		PROP_NAME,
	} state;

	std::list<PropNameValue> props;

	ProppatchParserData():state(ROOT) {}
};

class ProppatchMethod {
	ProppatchParserData data;

public:
	bool ParseRequest(was_simple *w);

	std::list<PropNameValue> &GetProps() {
		return data.props;
	}

	bool SendResponse(was_simple *w, std::string_view uri);
};
