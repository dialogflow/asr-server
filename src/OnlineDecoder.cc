// OnlineDecoder.cc

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

#include "OnlineDecoder.h"
#include "Timing.h"

namespace apiai {

#define PAD_SIZE 400
#define AUDIO_DATA_FREQUENCY 16000
kaldi::BaseFloat padVector[PAD_SIZE];

struct OnlineDecoder::DecodedData {
	kaldi::LatticeWeight weight;
	std::vector<int32> words;
	std::vector<int32> alignment;
	std::vector<kaldi::LatticeWeight> weights;
};

bool wordsEquals(std::vector<int32> &a, std::vector<int32> &b) {
	return (a.size() == b.size()) && (std::equal(a.begin(), a.end(), b.begin()));
}

bool getWeightMeasures(const kaldi::Lattice &fst,
                            std::vector<kaldi::LatticeArc::Weight> *weights_out) {
  typedef kaldi::LatticeArc::Label Label;
  typedef kaldi::LatticeArc::StateId StateId;
  typedef kaldi::LatticeArc::Weight Weight;

  std::vector<Weight> weights;

  StateId cur_state = fst.Start();
  if (cur_state == fst::kNoStateId) {  // empty sequence.
    if (weights_out != NULL) weights_out->clear();
    return true;
  }
  while (1) {
    Weight w = fst.Final(cur_state);
    if (w != Weight::Zero()) {  // is final..

      if (w.Value1() != 0 || w.Value2() != 0) {
    	  weights.push_back(w);
      }
      if (fst.NumArcs(cur_state) != 0) return false;
      if (weights_out != NULL) *weights_out = weights;
      return true;
    } else {
      if (fst.NumArcs(cur_state) != 1) return false;

      fst::ArcIterator<fst::Fst<kaldi::LatticeArc> > iter(fst, cur_state);  // get the only arc.
      const kaldi::LatticeArc &arc = iter.Value();
      if (arc.weight.Value1() != 0 || arc.weight.Value2() != 0) {
    	  weights.push_back(arc.weight);
      }
      cur_state = arc.nextstate;
    }
  }
}


OnlineDecoder::OnlineDecoder() {
	lm_scale_ = 10;
	chunk_length_secs_ = 0.18;
	max_record_size_seconds_ = 0;
	max_lattice_unchanged_interval_seconds_ = 0;
	decoding_timeout_seconds_ = 0;

	word_syms_rxfilename_ = "words.txt";
	fst_rxfilename_ = "HCLG.fst";
}

OnlineDecoder::~OnlineDecoder() {
}


void OnlineDecoder::GetRecognitionResult(DecodedData &input, RecognitionResult *output) {
	  // TODO move parameters to external file
	  output->confidence = std::max(0.0, std::min(1.0, -0.0001466488 * (2.388449*float(input.weight.Value1()) + float(input.weight.Value2())) / (input.words.size() + 1) + 0.956));

	  std::ostringstream outss;

	  for (size_t i = 0; i < input.words.size(); i++) {
		if (i) {
		  outss << " ";
		}
		std::string s = word_syms_->Find(input.words[i]);
		if (s == "") {
		  KALDI_WARN << "Word-id " << input.words[i] <<" not in symbol table.";
		} else {
			outss << s;
		}
	  }
	  output->text = outss.str();
}

void OnlineDecoder::GetRecognitionResult(std::vector<DecodedData> &input, std::vector<RecognitionResult> *output) {
	for (int i = 0; i < input.size(); i++) {
		RecognitionResult result;
		GetRecognitionResult(input.at(i), &result);
		output->push_back(result);
	}
}

void OnlineDecoder::RegisterOptions(kaldi::OptionsItf &po) {
    po.Register("chunk-length", &chunk_length_secs_,
                "Length of chunk size in seconds, that we process.");
    po.Register("word-symbol-table", &word_syms_rxfilename_,
                "Symbol table for words [for debug output]");
    po.Register("fst-in", &fst_rxfilename_, "Path to FST model file");
    po.Register("lm-scale", &lm_scale_, "Scaling factor for LM probabilities. "
				"Note: the ratio acoustic-scale/lm-scale is all that matters.");

    po.Register("max-record-length", &max_record_size_seconds_,
    		"Max length of record in seconds to be recognised. "
			"All records longer than given value will be truncated. Note: Non-positive value to deactivate.");

    po.Register("max-lattice-unchanged-interval", &max_lattice_unchanged_interval_seconds_,
			"Max interval length in seconds of lattice recognised unchanged. Note: Non-positive value to deactivate.");

    po.Register("decoding-timeout", &decoding_timeout_seconds_,
    		"Decoding process timeout given in seconds. Timeout disabled if value is non-positive.");
}

bool OnlineDecoder::Initialize(kaldi::OptionsItf &po) {
	word_syms_ = NULL;
	if (word_syms_rxfilename_ == "") {
		return false;
	}
	if (!(word_syms_ = fst::SymbolTable::ReadText(word_syms_rxfilename_))) {
		KALDI_ERR << "Could not read symbol table from file "
			  << word_syms_rxfilename_;
	}
	return true;
}

void OnlineDecoder::Decode(Request &request, Response &response) {
	try {
		KALDI_ASSERT(request.Frequency() == AUDIO_DATA_FREQUENCY);
		milliseconds_t start_time = getMilliseconds();
		milliseconds_t progress_time = 0;

		KALDI_VLOG(1) << "Started @ " << start_time << " ms";
		InputStarted();

		int intermediate_counter = 1;
		int intermediate_samples_interval = request.IntermediateIntervalMillisec() > 0 ? request.IntermediateIntervalMillisec() * (request.Frequency() / 1000) : 0;
		int max_samples_limit = max_record_size_seconds_ > 0 ? max_record_size_seconds_ * request.Frequency() : 0;

		std::vector<int32> prev_words;
		int samples_per_chunk = int(chunk_length_secs_ * request.Frequency());

		int samp_counter = 0;

		kaldi::SubVector<kaldi::BaseFloat> *wave_part;

		bool do_endpointing = request.DoEndpointing();
		std::string requestInterrupted = Response::NOT_INTERRUPTED;
		int samples_left = (max_samples_limit > 0) ? std::min(max_samples_limit, samples_per_chunk) : samples_per_chunk;
		const bool decoding_timeout_enabled = decoding_timeout_seconds_ > 0;
		const int decoding_timeout_ms = decoding_timeout_enabled ? decoding_timeout_seconds_ * 1000 : 0;

		int time_left_ms = decoding_timeout_ms;
		while ((wave_part = request.NextChunk(samples_left, time_left_ms)) != NULL) {

			samp_counter += wave_part->Dim();

			if (AcceptWaveform(request.Frequency(), *wave_part, do_endpointing) == false && do_endpointing) {
				requestInterrupted = Response::INTERRUPTED_END_OF_SPEECH;
				KALDI_VLOG(1) << "End Point Detected @ " << (getMillisecondsSince(start_time)) << " ms";
				break;
			}
			progress_time = getMillisecondsSince(start_time);

			if (max_samples_limit > 0) {
				if (samp_counter > max_samples_limit) {
					requestInterrupted = Response::INTERRUPTED_DATA_SIZE_LIMIT;
					KALDI_VLOG(1) << "Interrupted by record length @ " << progress_time << " ms";
					break;
				}
				samples_left = std::min(max_samples_limit - samp_counter, samples_per_chunk);
			}

			if ((intermediate_samples_interval > 0) && (samp_counter > (intermediate_samples_interval * intermediate_counter))) {
				intermediate_counter++;
				std::vector<DecodedData> decodeData;
				if (DecodeIntermediate(1, &decodeData) > 0) {
					DecodedData &data = decodeData.at(0);
					if (!wordsEquals(prev_words, data.words)) {
						RecognitionResult recognitionResult;
						GetRecognitionResult(data, &recognitionResult);
						response.SetIntermediateResult(recognitionResult, (samp_counter / (request.Frequency() / 1000)));
						prev_words = data.words;
					}
				} else {
					prev_words.clear();
				}
			}
			if (decoding_timeout_enabled) {
				time_left_ms = decoding_timeout_ms - getMillisecondsSince(start_time);
				if (time_left_ms <= 0) {
					break;
				}
			}
		}
		if (wave_part != NULL && requestInterrupted.size() == 0) {
			if (decoding_timeout_enabled && (decoding_timeout_ms - getMillisecondsSince(start_time) <= 0)) {
				KALDI_VLOG(1) << "Timeout reached @ " << (getMillisecondsSince(start_time)) << " ms";
				requestInterrupted = Response::INTERRUPTED_TIMEOUT;
			} else {
				requestInterrupted = Response::INTERRUPTED_UNEXPECTED;
			}
		}

		if (samp_counter == 0) {
			throw std::runtime_error("Got no data");
		}

		if (samp_counter < PAD_SIZE) {
			KALDI_VLOG(1) << "Input too short, padding with " << (PAD_SIZE - samp_counter) << " zero samples";
			kaldi::SubVector<kaldi::BaseFloat> padding(padVector, PAD_SIZE - samp_counter);
			AcceptWaveform(request.Frequency(), padding, false);
		}

		KALDI_VLOG(1) << "Input finished @ " << getMillisecondsSince(start_time) << " ms (audio length: " << (samp_counter / (request.Frequency() / 1000)) << " ms)";
		InputFinished();

		std::vector<DecodedData> result;

		int32 decoded = Decode(true, request.BestCount(), &result);

		if (decoded == 0) {
			response.SetError("Best-path failed");
			KALDI_WARN << "Best-path failed";
		} else {
			std::vector<RecognitionResult> recognitionResults;
			GetRecognitionResult(result, &recognitionResults);
			response.SetResult(recognitionResults, requestInterrupted, (samp_counter / (request.Frequency() / 1000)));
			KALDI_VLOG(1) << "Recognized @ " << getMillisecondsSince(start_time) << " ms";
		}

		CleanUp();

		KALDI_VLOG(1) << "Decode subroutine done";
	} catch (std::runtime_error &e) {
		response.SetError(e.what());
	}
};

int32 OnlineDecoder::DecodeIntermediate(int bestCount, std::vector<DecodedData> *result) {
	return Decode(false, bestCount, result);
}

int32 OnlineDecoder::Decode(bool end_of_utterance, int bestCount, std::vector<DecodedData> *result) {
	kaldi::CompactLattice clat;
	GetLattice(&clat, end_of_utterance);

	if (clat.NumStates() == 0) {
		return 0;
	}

	if (lm_scale_ != 0) {
		fst::ScaleLattice(fst::LatticeScale(lm_scale_, 1.0), &clat);
	}

	int32 resultsNumber = 0;

	if (bestCount > 1) {
		kaldi::Lattice _lat;
		fst::ConvertLattice(clat, &_lat);
		kaldi::Lattice nbest_lat;
		fst::ShortestPath(_lat, &nbest_lat, bestCount);
		std::vector<kaldi::Lattice> nbest_lats;
		fst::ConvertNbestToVector(nbest_lat, &nbest_lats);
		if (!nbest_lats.empty()) {
		  resultsNumber = static_cast<int32>(nbest_lats.size());
		  for (int32 k = 0; k < resultsNumber; k++) {
			kaldi::Lattice &nbest_lat = nbest_lats[k];

			DecodedData decodeData;
			GetLinearSymbolSequence(nbest_lat, &(decodeData.alignment), &(decodeData.words), &(decodeData.weight));
			getWeightMeasures(nbest_lat, &(decodeData.weights));
			result->push_back(decodeData);
		  }
		}
	} else {
		kaldi::CompactLattice best_path_clat;
		kaldi::CompactLatticeShortestPath(clat, &best_path_clat);

		kaldi::Lattice best_path_lat;
		fst::ConvertLattice(best_path_clat, &best_path_lat);
		DecodedData decodeData;
		GetLinearSymbolSequence(best_path_lat, &(decodeData.alignment), &(decodeData.words), &(decodeData.weight));
		getWeightMeasures(best_path_lat, &(decodeData.weights));
		result->push_back(decodeData);
		resultsNumber = 1;
	}

	return resultsNumber;
}

} /* namespace apiai */
