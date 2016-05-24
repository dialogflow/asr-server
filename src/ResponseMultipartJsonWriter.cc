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

#include "ResponseMultipartJsonWriter.h"

namespace apiai {

const std::string ResponseMultipartJsonWriter::MIME_MULTIPART = "multipart/x-mixed-replace";

ResponseMultipartJsonWriter::~ResponseMultipartJsonWriter() {

}

ResponseMultipartJsonWriter::ResponseMultipartJsonWriter(std::ostream *osb)
	: ResponseJsonWriter(osb)
{
	boundary_token_ = "ResponseBoundary";
	content_type_ = MIME_MULTIPART + ";boundary=" + boundary_token_;
	data_sent_ = false;
}

void ResponseMultipartJsonWriter::SendJson(std::string json, bool final) {
	if (! data_sent_) {
		*out() << "\r\n--" << boundary_token_ << "\r\n";
		data_sent_ = true;
	}

	*out() << "Content-Disposition: form-data; name=\""
		<< (final ? "result" : "partial")
		<< "\"\r\n"
		<< "Content-type: " << ResponseJsonWriter::GetContentType() << "\r\n"
		<< "\r\n";

	ResponseJsonWriter::SendJson(json, final);

	*out() << "\r\n"
		<< "--" << boundary_token_
		<< (final ? "--" : "")
		<< "\r\n";

	out()->flush();
}

} /* namespace apiai */
