// QueryStringParser.cc

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
// WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache 2 License for the specific language governing permissions and
// limitations under the License.

#include "QueryStringParser.h"

namespace apiai {

enum ParseState {
	NameRead, ValueRead, Done
};

QueryStringParser::QueryStringParser(const char *query) {
	query_.assign(query);
	Init();
}

QueryStringParser::QueryStringParser(const std::string &query) {
	query_ = query;
	Init();
}

QueryStringParser::~QueryStringParser() {

}

void QueryStringParser::Init() {


	if (query_.size() == 0 || query_ == "?") {
		has_next_ = false;
		return;
	}

	std::string::iterator index = query_.begin();

	if ((*index) == '?') {
		++index;
	}

	has_next_ = SeekNext(index);
}

bool QueryStringParser::SeekNext(std::string::iterator &from) {
	std::string::iterator index = from;

	ParseState state = NameRead;
	name_begin_ = index;
	value_begin_ = value_end_ = query_.end();
	for (;(state != Done) && (index < query_.end()); index++) {
		switch (*index) {
		case '=':
			switch (state) {
			case NameRead:
				state = ValueRead;
				name_end_ = index;
				value_begin_ = value_end_ = index + 1;
				break;
			case ValueRead:
				// Do nothing
				break;
			case Done:
				// Do nothing
				break;
			}

			break;
		case '&':
			switch (state) {
			case NameRead:
				name_end_ = index;
				break;
			case ValueRead:
				value_end_ = index;
				break;
			case Done:
				// Do nothing
				break;
			}
			state = Done;
			break;
		default:
			break;
		};
	}

	switch (state) {
	case NameRead:
		name_end_ = index;;
		break;
	case ValueRead:
		value_end_ = index;
		break;
	case Done:
		// Do nothing
		break;
	}

	bool result = index != from;

	return result;
}

bool QueryStringParser::Next(std::string *name, std::string *value) {
	if (!has_next_) {
		return false;
	}

	name->assign(name_begin_, name_end_);
	value->assign(value_begin_, value_end_);

	std::string::iterator index = value_end_;
	++index;
	has_next_ = SeekNext(index);
	return true;
}

} /* namespace apiai */
