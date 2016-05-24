// ResponseJsonWriter.cc

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

#ifndef RESPONSEMULTIPARTJSONWRITER_H_
#define RESPONSEMULTIPARTJSONWRITER_H_

#include "ResponseJsonWriter.h"

namespace apiai {

class ResponseMultipartJsonWriter: public ResponseJsonWriter {
public:
	ResponseMultipartJsonWriter(std::ostream *osb);
	virtual ~ResponseMultipartJsonWriter();

	virtual const std::string &GetContentType() { return content_type_; }
protected:
	virtual void SendJson(std::string json, bool final);
private:
	std::string boundary_token_;
	std::string content_type_;
	bool data_sent_;
	static const std::string MIME_MULTIPART;
};

} /* namespace apiai */

#endif /* RESPONSEMULTIPARTJSONWRITER_H_ */
