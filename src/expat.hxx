/*
 * expat wrapper.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_EXPAT_HXX
#define DAVOS_EXPAT_HXX

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

#endif
