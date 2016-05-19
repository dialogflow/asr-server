// ResponseJsonWriter.h

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

#ifndef APIAI_DECODER_STIRESPONSEWRITER_H_
#define APIAI_DECODER_STIRESPONSEWRITER_H_

#include "Response.h"
#include <sstream>

namespace apiai {

/**
 * Writes recognition data to output stream as JSON serialized objects
 */
class ResponseJsonWriter : public Response {
public:
	ResponseJsonWriter(std::ostream *osb) : out_(osb) {}
	virtual ~ResponseJsonWriter() {};

	virtual void SetResult(std::vector<RecognitionResult> &data, int timeMarkMs);
	virtual void SetResult(std::vector<RecognitionResult> &data, bool interrupted, int timeMarkMs);
	virtual void SetIntermediateResult(RecognitionResult &decodedData, int timeMarkMs);
	virtual void SetError(const std::string &message);
private:
	void Write(std::ostringstream &outss, RecognitionResult &data);
	std::ostream *out_;
};

} /* namespace apiai */

#endif /* APIAI_DECODER_STIRESPONSEWRITER_H_ */
