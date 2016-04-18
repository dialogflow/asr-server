// Decoder.h

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

#ifndef APIAI_DECODER_DECODER_H_
#define APIAI_DECODER_DECODER_H_

#include "Request.h"
#include "Response.h"
#include "util/parse-options.h"

namespace apiai {

/**
 * ASR decoder basic interface
 */
class Decoder {
public:
	virtual ~Decoder() {};

	/** Create decoder clone */
	virtual Decoder *Clone() const = 0;
	/** Register options which can be defined via command line arguments */
	virtual void RegisterOptions(kaldi::OptionsItf &po) = 0;
	/** Initialize decoder */
	virtual bool Initialize(kaldi::OptionsItf &po) = 0;
	/** Perform decoding routine */
	virtual void Decode(Request &request, Response &response) = 0;
};

} /* namespace apiai */

#endif /* APIAI_DECODER_DECODER_H_ */
