// QueryStringParser.h

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

#ifndef SRC_QUERYSTRINGPARSER_H_
#define SRC_QUERYSTRINGPARSER_H_

#include <string>

namespace apiai {

/**
 * Query string parser.
 * Iterates through name-value pairs of standard URI query string
 */
class QueryStringParser {
public:
	/** Initialize parse with given query string */
	QueryStringParser(const char *query);
	/** Initialize parse with given query string */
	QueryStringParser(const std::string &query);
	virtual ~QueryStringParser();

	/** Returns true if there is more unhandled name-value pairs */
	bool HasNext() const { return has_next_; }
	/**
	 * Get next name-value pair.
	 * Returns false if there is no more pairs
	 */
	bool Next(std::string *name, std::string *value);
private:
	void Init();
	bool SeekNext(std::string::iterator &from);

	std::string query_;
	bool has_next_;

	std::string::iterator name_begin_;
	std::string::iterator name_end_;
	std::string::iterator value_begin_;
	std::string::iterator value_end_;
};

} /* namespace apiai */

#endif /* SRC_QUERYSTRINGPARSER_H_ */
