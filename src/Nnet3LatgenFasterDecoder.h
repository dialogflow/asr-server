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
#include "online2/online-nnet2-feature-pipeline.h"

namespace apiai {

class Nnet3LatgenFasterDecoder: public OnlineDecoder {
public:
	Nnet3LatgenFasterDecoder();
	virtual ~Nnet3LatgenFasterDecoder();

	virtual Nnet3LatgenFasterDecoder *Clone() const;
	virtual void RegisterOptions(kaldi::OptionsItf &po);
	virtual bool Initialize(kaldi::OptionsItf &po);
protected:
	virtual bool AcceptWaveform(kaldi::BaseFloat sampling_rate,
			const kaldi::VectorBase<kaldi::BaseFloat> &waveform,
			const bool do_endpointing);
	virtual void InputStarted();
	virtual void InputFinished();
	virtual void GetLattice(kaldi::CompactLattice *clat, bool end_of_utterance);
	virtual void CleanUp();
private:
	std::string nnet3_rxfilename_;

    bool online_;
    kaldi::OnlineEndpointConfig endpoint_config_;

    // feature_config includes configuration for the iVector adaptation,
    // as well as the basic features.
    kaldi::OnlineNnet2FeaturePipelineConfig feature_config_;
    kaldi::nnet3::NnetSimpleLoopedComputationOptions decodable_opts_;  
    kaldi::LatticeFasterDecoderConfig decoder_opts_;                   

    kaldi::OnlineNnet2FeaturePipelineInfo *feature_info_;
    fst::Fst<fst::StdArc> *decode_fst_;
    kaldi::TransitionModel *trans_model_;
    kaldi::nnet3::AmNnetSimple *nnet_;
    kaldi::nnet3::DecodableNnetSimpleLoopedInfo *decodable_info_;     

    kaldi::OnlineIvectorExtractorAdaptationState *adaptation_state_;
    kaldi::OnlineNnet2FeaturePipeline *feature_pipeline_;
    kaldi::SingleUtteranceNnet3Decoder *decoder_;
};

} /* namespace apiai */

#endif /* APIAI_DECODER_NNET3LATGENFASTERDECODER_H_ */
