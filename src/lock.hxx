// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * Fake LOCK/UNLOCK implementation.
 */

#pragma once

#include <string>

struct was_simple;

struct LockParserData {
	enum State {
		ROOT,
		OWNER,
		OWNER_HREF,
	} state;

	std::string owner_href;

	LockParserData():state(ROOT) {}
};

class LockMethod {
	LockParserData data;

public:
	bool ParseRequest(was_simple *w);
	void Run(was_simple *w, bool created);
};
