// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

/*
 * expat wrapper.
 */

#pragma once

#include <expat.h>

struct was_simple;

class ExpatParser {
	XML_Parser parser;

public:
	ExpatParser(void *userData)
		:parser(XML_ParserCreateNS(nullptr, '|')) {
		XML_SetUserData(parser, userData);
	}

	~ExpatParser() {
		XML_ParserFree(parser);
	}

	void SetElementHandler(XML_StartElementHandler start,
			       XML_EndElementHandler end) {
		XML_SetElementHandler(parser, start, end);
	}

	void SetCharacterDataHandler(XML_CharacterDataHandler charhndl) {
		XML_SetCharacterDataHandler(parser, charhndl);
	}

	bool Parse(was_simple *w);
};
