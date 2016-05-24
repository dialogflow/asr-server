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

#ifndef RESPONSEJSONWRITER_H_
#define RESPONSEJSONWRITER_H_

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

	virtual const std::string &GetContentType() { return MIME_APPLICATION_JAVA; }

	virtual void SetResult(std::vector<RecognitionResult> &data, int timeMarkMs);
	virtual void SetResult(std::vector<RecognitionResult> &data, const std::string &interrupted, int timeMarkMs);
	virtual void SetIntermediateResult(RecognitionResult &decodedData, int timeMarkMs);
	virtual void SetError(const std::string &message);
protected:
	std::ostream *out() { return out_; }

	virtual void SendJson(std::string json, bool final);
private:
	void Write(std::ostringstream &outss, RecognitionResult &data);
	std::ostream *out_;

	static const std::string MIME_APPLICATION_JAVA;
};

} /* namespace apiai */

#endif /* RESPONSEJSONWRITER_H_ */
