// RequestRawReader.cc

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

#include "RequestRawReader.h"

#include "Timing.h"
#include <algorithm>

namespace apiai {

const milliseconds_t data_wait_interval_ms = 500;

kaldi::SubVector<kaldi::BaseFloat> *RequestRawReader::NextChunk(kaldi::int32 samples_count)
{
	return NextChunk(samples_count, 0);
}

kaldi::SubVector<kaldi::BaseFloat> *RequestRawReader::NextChunk(kaldi::int32 samples_count, kaldi::int32 timeout_ms) {
	// TODO: timeout_ms is not supported because libfcgi do not provides "readsome" functionality
	if (samples_count <= 0) {
		return NULL;
	}

	if (fail_) {
		return NULL;
	}

	int frame_size = bytes_per_sample_ * channels_;
	kaldi::int32 chunk_size = samples_count * frame_size;

	int offset = channel_index_ * bytes_per_sample_;

	std::vector<char> audioData(chunk_size);

	int bytes_read = 0;

	is_->read(audioData.data(), chunk_size);

	bytes_read = is_->gcount();

	if (is_->gcount() == 0) {
		fail_ = true;
		last_error_message_ == "Failed to read any data";
		return NULL;
	}

	buffer_.clear();
	for (int index = 0; index < bytes_read; index += frame_size) {
		kaldi::int16 value = *reinterpret_cast<kaldi::int16*>(audioData.data() + index + offset);
		kaldi::BaseFloat fvalue = kaldi::BaseFloat(value);
		buffer_.push_back(fvalue);
	}

	if (current_chunk_ && (current_chunk_->Dim() != buffer_.size())) {
		delete current_chunk_;
		current_chunk_ = NULL;
	}

	if (!current_chunk_) {
		current_chunk_ = new kaldi::SubVector<kaldi::BaseFloat>(buffer_.data(), buffer_.size());
	} else {
		KALDI_ASSERT(buffer_.size() == current_chunk_->Dim());
		std::copy(buffer_.begin(), buffer_.end(), current_chunk_->Data());
	}

	return current_chunk_;
}

} /* namespace apiai */
