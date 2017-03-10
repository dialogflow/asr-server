// OnlineDecoder.h

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

#ifndef APIAI_DECODER_ONLINEDECODER_H_
#define APIAI_DECODER_ONLINEDECODER_H_

#include "Decoder.h"
#include "online2/online-feature-pipeline.h"
#include "online2/onlinebin-util.h"
#include "online2/online-timing.h"
#include "online2/online-endpoint.h"
#include "fstext/fstext-lib.h"
#include "lat/lattice-functions.h"
#include <list>

namespace apiai {

/**
 * Basic implementation of common code for all Kaldi online decoders
 */
class OnlineDecoder : public Decoder {
public:
	OnlineDecoder();
	virtual ~OnlineDecoder();

	virtual void RegisterOptions(kaldi::OptionsItf &po);
	virtual bool Initialize(kaldi::OptionsItf &po);
	virtual void Decode(Request &request, Response &response);
protected:
	struct DecodedData;

	/**
	 * Process next data chunk
	 */
	virtual bool AcceptWaveform(kaldi::BaseFloat sampling_rate,
                    const kaldi::VectorBase<kaldi::BaseFloat> &waveform,
					const bool do_endpointing) = 0;

	/**
	 * Preparare to decoding
	 */
	virtual void InputStarted() = 0;
	/**
	 * Decoding finished, gets ready to get results
	 */
	virtual void InputFinished() = 0;
	/**
	 * Put result lattice
	 */
	virtual void GetLattice(kaldi::CompactLattice *clat, bool end_of_utterance) = 0;
	/**
	 * Clean all data
	 */
	virtual void CleanUp() = 0;
	/**
	 * Calculate intermediate results
	 */
	virtual kaldi::int32 DecodeIntermediate(int bestCount, std::vector<DecodedData> *result);

	std::string word_syms_rxfilename_;
	kaldi::BaseFloat chunk_length_secs_;
	kaldi::BaseFloat acoustic_scale_;
	kaldi::BaseFloat lm_scale_;


	/**
	 * Max length of record in seconds to be recognised.
	 * All records longer than given value will be truncated. Note: Non-positive value to deactivate.
	 */
	kaldi::BaseFloat max_record_size_seconds_;
	/**
	 * Max interval length in seconds of lattice recognised unchanged. Non-positive value to deactivate
	 */
	kaldi::BaseFloat max_lattice_unchanged_interval_seconds_;

	/** Decoding process timeout given in seconds.
	 * Timeout disabled if value is non-positive
	 */
	kaldi::BaseFloat decoding_timeout_seconds_;

	bool do_endpointing_;

	std::string fst_rxfilename_;
private:
	fst::SymbolTable *word_syms_;

	kaldi::int32 Decode(bool end_of_utterance, int bestCount, std::vector<DecodedData> *result);

	void GetRecognitionResult(DecodedData &input, RecognitionResult *output);
	void GetRecognitionResult(std::vector<DecodedData> &input, std::vector<RecognitionResult> *output);
};

} /* namespace apiai */

#endif /* APIAI_DECODER_ONLINEDECODER_H_ */
