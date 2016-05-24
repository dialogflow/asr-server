// Response.h

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

#ifndef RESPONSE_H_
#define RESPONSE_H_

#include <string>
#include <vector>

namespace apiai {

/**
 * Recognition results holder
 */
struct RecognitionResult {
	/**
	 * Confidence value given in percents
	 */
	float confidence;
	/**
	 * Recognition result text
	 */
	std::string text;
};

/**
 * Interface for recognition data collector
 */
class Response {
public:
	virtual ~Response() {};

	/** Get content type MIME string */
	virtual const std::string &GetContentType() = 0;

	/** Set final results */
	virtual void SetResult(std::vector<RecognitionResult> &data, int timeMarkMs) = 0;
	/** Set final result.
	 * Value of interrupted flag is set to true if recognition process was interrupted before
	 * all given data been read.
	 */
	virtual void SetResult(std::vector<RecognitionResult> &data, const std::string &interrupted, int timeMarkMs) = 0;
	/** Set intermediate result */
	virtual void SetIntermediateResult(RecognitionResult &decodedData, int timeMarkMs) = 0;
	/** Set error value */
	virtual void SetError(const std::string &message) = 0;

	static const std::string NOT_INTERRUPTED;
	static const std::string INTERRUPTED_UNEXPECTED;
	static const std::string INTERRUPTED_END_OF_SPEECH;
	static const std::string INTERRUPTED_DATA_SIZE_LIMIT;
	static const std::string INTERRUPTED_TIMEOUT;
};

} /* namespace apiai */

#endif /* RESPONSE_H_ */
