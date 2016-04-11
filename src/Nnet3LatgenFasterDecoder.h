// Nnet3LatgenFasterDecoder.h

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

#ifndef APIAI_DECODER_NNET3LATGENFASTERDECODER_H_
#define APIAI_DECODER_NNET3LATGENFASTERDECODER_H_

#include "OnlineDecoder.h"
#include "online2/online-nnet3-decoding.h"

namespace apiai {

class Nnet3LatgenFasterDecoder: public OnlineDecoder {
public:
	Nnet3LatgenFasterDecoder();
	~Nnet3LatgenFasterDecoder();

	void RegisterOptions(kaldi::OptionsItf &po);
	bool Initialize(kaldi::OptionsItf &po);
protected:
	bool AcceptWaveform(kaldi::BaseFloat sampling_rate,
			const kaldi::VectorBase<kaldi::BaseFloat> &waveform);
	void InputStarted();
	void InputFinished();
	void GetLattice(kaldi::CompactLattice *clat);
	void CleanUp();

	int32 DecodeIntermediate(int bestCount, vector<DecodedData> *result);
private:
	std::string nnet3_rxfilename_;

    bool online_;
    kaldi::OnlineEndpointConfig endpoint_config_;

    // feature_config includes configuration for the iVector adaptation,
    // as well as the basic features.
    kaldi::OnlineNnet2FeaturePipelineConfig feature_config_;
    kaldi::OnlineNnet3DecodingConfig nnet3_decoding_config_;

    kaldi::OnlineNnet2FeaturePipelineInfo *feature_info_;
    fst::Fst<fst::StdArc> *decode_fst_;
    kaldi::TransitionModel *trans_model_;
    kaldi::nnet3::AmNnetSimple *nnet_;


    kaldi::OnlineIvectorExtractorAdaptationState *adaptation_state_;
    kaldi::OnlineNnet2FeaturePipeline *feature_pipeline_;
    kaldi::SingleUtteranceNnet3Decoder *decoder_;
};

} /* namespace apiai */

#endif /* APIAI_DECODER_NNET3LATGENFASTERDECODER_H_ */
