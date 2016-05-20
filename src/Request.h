// Request.h

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

#ifndef SRC_REQUEST_H_
#define SRC_REQUEST_H_

#include "base/kaldi-types.h"
#include "matrix/kaldi-vector.h"

namespace apiai {

/**
 * Request data holding interface
 */
class Request {
public:
	virtual ~Request() {};

	/** Get number of samples per second of audio data */
	virtual kaldi::int32 Frequency(void) const = 0;

	/** Get max number of expected result variants */
	virtual kaldi::int32 BestCount(void) const = 0;
	/** Get milliseconds interval between intermediate results.
	 *  If non-positive given then no intermediate results would be calculated */
	virtual kaldi::int32 IntermediateIntervalMillisec(void) const = 0;

	/** Get end-of-speech points detection flag. */
	virtual bool DoEndpointing(void) const = 0;

	/**
	 * Get next chunk of audio data samples.
	 * Max number of samples specified by samples_count value
	 */
	virtual kaldi::SubVector<kaldi::BaseFloat> *NextChunk(kaldi::int32 samples_count) = 0;
	/**
	 * Get next chunk of audio data samples.
	 * Max number of samples specified by samples_count value.
	 * Read timeout specified by timeout_ms.
	 */
	virtual kaldi::SubVector<kaldi::BaseFloat> *NextChunk(kaldi::int32 samples_count, kaldi::int32 timeout_ms) = 0;
};

} /* namespace apiai */

#endif /* SRC_REQUEST_H_ */
