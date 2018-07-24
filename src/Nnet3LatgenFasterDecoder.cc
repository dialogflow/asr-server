// Nnet3LatgenFasterDecoder.cc

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

#include "Nnet3LatgenFasterDecoder.h"

namespace apiai {

Nnet3LatgenFasterDecoder::Nnet3LatgenFasterDecoder() {
	online_ = true;
	decode_fst_ = NULL;
	trans_model_ = NULL;
	nnet_ = NULL;
    decodable_info_ = NULL;    
	feature_info_ = NULL;
	nnet3_rxfilename_ = "final.mdl";
}

Nnet3LatgenFasterDecoder::~Nnet3LatgenFasterDecoder() {
	delete decode_fst_;
	delete trans_model_;
	delete nnet_;
    delete decodable_info_;   
	delete feature_info_;
}

Nnet3LatgenFasterDecoder *Nnet3LatgenFasterDecoder::Clone() const {
	return new Nnet3LatgenFasterDecoder(*this);
}

void Nnet3LatgenFasterDecoder::RegisterOptions(kaldi::OptionsItf &po) {
	OnlineDecoder::RegisterOptions(po);

	po.Register("nnet-in", &nnet3_rxfilename_,
	                "Path to nnet");
    po.Register("online", &online_,
                "You can set this to false to disable online iVector estimation "
                "and have all the data for each utterance used, even at "
                "utterance start.  This is useful where you just want the best "
                "results and don't care about online operation.  Setting this to "
                "false has the same effect as setting "
                "--use-most-recent-ivector=true and --greedy-ivector-extractor=true "
                "in the file given to --ivector-extraction-config, and "
                "--chunk-length=-1.");

    feature_config_.Register(&po);
    decoder_opts_.Register(&po);
    decodable_opts_.Register(&po);
    endpoint_config_.Register(&po);
}

bool Nnet3LatgenFasterDecoder::Initialize(kaldi::OptionsItf &po) {
	if (!OnlineDecoder::Initialize(po)) {
		return false;
	}

	if (fst_rxfilename_ == "") {
		return false;
	}

	if (nnet3_rxfilename_ == "") {
		return false;
	}

    feature_info_ = new kaldi::OnlineNnet2FeaturePipelineInfo(feature_config_);

    if (!online_) {
      feature_info_->ivector_extractor_info.use_most_recent_ivector = true;
      feature_info_->ivector_extractor_info.greedy_ivector_extractor = true;
      chunk_length_secs_ = -1.0;
    }

    trans_model_ = new kaldi::TransitionModel();
    nnet_ = new kaldi::nnet3::AmNnetSimple();      
    {
      bool binary;
      kaldi::Input ki(nnet3_rxfilename_, &binary);
      trans_model_->Read(ki.Stream(), binary);
      nnet_->Read(ki.Stream(), binary);
    }

    // this object contains precomputed stuff that is used by all decodable
    // objects.  It takes a pointer to nnet_ because if it has iVectors it has
    // to modify the nnet to accept iVectors at intervals.
    decodable_info_ = new kaldi::nnet3::DecodableNnetSimpleLoopedInfo(     
                            decodable_opts_, nnet_);

    decode_fst_ = fst::ReadFstKaldiGeneric(fst_rxfilename_);

    fst::SymbolTable *word_syms = NULL;
    if (word_syms_rxfilename_ != "")
      if (!(word_syms = fst::SymbolTable::ReadText(word_syms_rxfilename_)))
        KALDI_ERR << "Could not read symbol table from file "
                  << word_syms_rxfilename_;

    acoustic_scale_ = decodable_opts_.acoustic_scale;                          

    return true;
}

void Nnet3LatgenFasterDecoder::InputStarted()
{
	adaptation_state_ = new kaldi::OnlineIvectorExtractorAdaptationState(feature_info_->ivector_extractor_info);

	feature_pipeline_ = new kaldi::OnlineNnet2FeaturePipeline (*feature_info_);
	feature_pipeline_->SetAdaptationState(*adaptation_state_);

	decoder_ = new kaldi::SingleUtteranceNnet3Decoder(decoder_opts_,
										*trans_model_,
										*decodable_info_,
										*decode_fst_,
										feature_pipeline_);
}


void Nnet3LatgenFasterDecoder::CleanUp()
{
	delete decoder_;
	delete adaptation_state_;
	delete feature_pipeline_;

	decoder_ = NULL;
	adaptation_state_ = NULL;
	feature_pipeline_ = NULL;
}

bool Nnet3LatgenFasterDecoder::AcceptWaveform(kaldi::BaseFloat sampling_rate,
		const kaldi::VectorBase<kaldi::BaseFloat> &waveform,
		const bool do_endpointing)
{
	feature_pipeline_->AcceptWaveform(sampling_rate, waveform);

	if (do_endpointing && decoder_->EndpointDetected(endpoint_config_)) {
		return false;
	}

	decoder_->AdvanceDecoding();

	return true;
}

void Nnet3LatgenFasterDecoder::InputFinished()
{
	feature_pipeline_->InputFinished();
	decoder_->AdvanceDecoding();
	decoder_->FinalizeDecoding();
}

void Nnet3LatgenFasterDecoder::GetLattice(kaldi::CompactLattice *clat, bool end_of_utterance)
{
	decoder_->GetLattice(end_of_utterance, clat);

	// In an application you might avoid updating the adaptation state if
	// you felt the utterance had low confidence.  See lat/confidence.h
	feature_pipeline_->GetAdaptationState(adaptation_state_);

	if (acoustic_scale_ != 0) {
		ScaleLattice(fst::AcousticLatticeScale(1.0 / acoustic_scale_), clat);
	}
}

} /* namespace apiai */
