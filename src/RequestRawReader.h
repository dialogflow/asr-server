// RequestRawReader.h

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

#ifndef APIAI_DECODER_STIREQUESTREADER_H_
#define APIAI_DECODER_STIREQUESTREADER_H_

#include "Request.h"
#include <stdio.h>
#include <istream>

#define NBEST_MIN 1
#define NBEST_MAX 10

#define INTERMEDIATE_MIN 500

namespace apiai {

/**
 * Provides access to PCM data from input stream.
 * Assumed that PCM is signed mono, 16 bits, 16 KHz
 */
class RequestRawReader : public Request {
public:
	RequestRawReader(std::istream *is)
	{
		fail_ = false;
		current_chunk_ = NULL;

		is_ = is;
		frequency_ = 16000;
		bytes_per_sample_ = 16 / 8;
		channels_ = 1;
		channel_index_ = 0;

		bestCount_ = 1;
		intermediateMillisecondsInterval_ = 0;
		doEndpointing_ = false;
	}

	virtual ~RequestRawReader() {
		delete current_chunk_;
	}

	virtual kaldi::int32 Frequency(void) const { return frequency_; }

	/** Get errors flag */
	bool HasErrors(void) { return fail_ || is_->fail(); }
	/** Get last error message */
	const std::string &LastErrorMessage(void) const { return last_error_message_; }

	virtual kaldi::int32 BestCount(void) const { return bestCount_; }
	virtual kaldi::int32 IntermediateIntervalMillisec(void) const { return intermediateMillisecondsInterval_; }
	virtual bool DoEndpointing(void) const { return doEndpointing_; }

	/** Set number of suggested recognition result variants */
	void BestCount(kaldi::int32 value) { bestCount_ = std::max(NBEST_MIN, std::min(NBEST_MAX, value)); }
	/** Set intermediate results interval in milliseconds */
	void IntermediateIntervalMillisec(kaldi::int32 value) {
		intermediateMillisecondsInterval_ = value > 0 ? std::max(value, INTERMEDIATE_MIN) : 0;
	}
	/** Set end-of-speech points detection flag. */
	void DoEndpointing(bool value) { doEndpointing_ = value; }

	virtual kaldi::SubVector<kaldi::BaseFloat> *NextChunk(kaldi::int32 samples_count);
	virtual kaldi::SubVector<kaldi::BaseFloat> *NextChunk(kaldi::int32 samples_count, kaldi::int32 timeout_ms);
private:
	bool fail_;
	kaldi::int32 frequency_;
	kaldi::int32 bytes_per_sample_;
	kaldi::int32 channels_;
	kaldi::int32 channel_index_;

	kaldi::int32 bestCount_;
	kaldi::int32 intermediateMillisecondsInterval_;
	bool doEndpointing_;

	std::istream *is_;
	std::vector<kaldi::BaseFloat> buffer_;
	std::string last_error_message_;
	kaldi::SubVector<kaldi::BaseFloat> *current_chunk_;
};

} /* namespace apiai */

#endif /* APIAI_DECODER_STIREQUESTREADER_H_ */
