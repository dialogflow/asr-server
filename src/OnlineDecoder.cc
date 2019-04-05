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

#include <iomanip>
#include "OnlineDecoder.h"
#include "json.hpp"
#include "Timing.h"

using namespace kaldi;
using json = nlohmann::json;

namespace apiai {

#define PAD_SIZE 400
#define AUDIO_DATA_FREQUENCY 16000
kaldi::BaseFloat padVector[PAD_SIZE];

struct OnlineDecoder::DecodedData {
	kaldi::LatticeWeight weight;
	std::vector<int32> words;
	std::vector<int32> alignment;
	std::vector<kaldi::LatticeWeight> weights;
  std::string text, ref_conf;
  float conf = 0.0;
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
	lm_scale_ = 1;
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
  //output->confidence = std::max(0.0, std::min(1.0, -0.0001466488 * (2.388449*float(input.weight.Value1()) + float(input.weight.Value2())) / (input.words.size() + 1) + 0.956));
  output->confidence = input.conf;
	  /*std::ostringstream outss;

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
	  output->text = outss.str();*/
      output->ref_conf = input.ref_conf;
      output->text = input.text;
      //KALDI_LOG << "text: " << input.text;
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
    po.Register("default-graph", &fst_rxfilename_, "Path to FST model file");
    po.Register("lm-scale", &lm_scale_, "Scaling factor for LM probabilities. "
				"Note: the ratio acoustic-scale/lm-scale is all that matters.");

    po.Register("max-record-length", &max_record_size_seconds_,
    		"Max length of record in seconds to be recognised. "
			"All records longer than given value will be truncated. Note: Non-positive value to deactivate.");

    po.Register("max-lattice-unchanged-interval", &max_lattice_unchanged_interval_seconds_,
			"Max interval length in seconds of lattice recognised unchanged. Note: Non-positive value to deactivate.");

    po.Register("decoding-timeout", &decoding_timeout_seconds_,
    		"Decoding process timeout given in seconds. Timeout disabled if value is non-positive.");

     /// amira
    po.Register("graph-list", &graph_list_file_,
                "Path to text file which lists all the decoding graphs");
    po.Register("trans-list", &trans_list_file_,
                "Path to text file which lists all the ref transcriptions");
    wbi_opts_.Register(&po);
    po.Register("word-boundary-info", &word_boundary_rxfilename_,
                "Word boundary info file");
    po.Register("default-word-symbol-table", &default_word_syms_rxfilename_,
                "The default symbol table for words.");

}

bool OnlineDecoder::Initialize(kaldi::OptionsItf &po) {


    /// amira
    KALDI_ASSERT(decode_graphs_ == NULL);
    decode_graphs_ = new
        std::unordered_map<std::string, fst::Fst<fst::StdArc>* >();
    ref_texts_ = new std::unordered_map<std::string, std::vector<int32> >();
    if (!graph_list_file_.empty()) {
      KALDI_LOG << "Graph list file provided. Will load all the graphs and "
                << "ref transcriptions...";
      KALDI_ASSERT(!trans_list_file_.empty());
      SequentialTableReader<fst::VectorFstHolder> fst_reader(graph_list_file_);
      RandomAccessInt32VectorReader transcript_reader(trans_list_file_);
      int num_success = 0, num_fail = 0;
      for (; !fst_reader.Done(); fst_reader.Next()) {
        std::string utt = fst_reader.Key();
        if (!transcript_reader.HasKey(utt)) {
          KALDI_LOG << "Warning: No transcription was found for phrase-id "
                    << utt;
          num_fail++;
          continue;
        }
        fst::Fst<fst::StdArc> *graph = new fst::VectorFst<fst::StdArc>(
            fst_reader.Value());
        decode_graphs_->insert({utt, graph});
        ref_texts_->insert({utt, transcript_reader.Value(utt)});
        num_success++;
      }
      //for (; !transcript_reader.Done(); transcript_reader.Next()) {
      //  std::string key = transcript_reader.Key();
      //  const std::vector<int32> &transcript = transcript_reader.Value();
      //}
      KALDI_LOG << "Loaded " << num_success << " graphs and ref transcripts. "
                << "Failed for " << num_fail;
    }
    if (!word_boundary_rxfilename_.empty())
      word_boundary_info_ = new WordBoundaryInfo(wbi_opts_,
                                                 word_boundary_rxfilename_);
    else
      KALDI_WARN << "Word boundary info needed for confidences...";

	word_syms_ = NULL;
	if (word_syms_rxfilename_ == "") {
		return false;
	}
	if (!(word_syms_ = fst::SymbolTable::ReadText(word_syms_rxfilename_))) {
		KALDI_ERR << "Could not read symbol table from file "
			  << word_syms_rxfilename_;
	}

    if (default_word_syms_rxfilename_.empty()) {
      KALDI_WARN << "No default word list provided. Default graph decoding won't work...";

    } else if (!(default_word_syms_ = fst::SymbolTable::ReadText(default_word_syms_rxfilename_))) {
      KALDI_ERR << "Could not read the default symbol table from file "
                << default_word_syms_rxfilename_;
    }

    return true;
}

void OnlineDecoder::Decode(Request &request, Response &response) {
	try {
      req_ = &request;
		KALDI_ASSERT(request.Frequency() == AUDIO_DATA_FREQUENCY);
		milliseconds_t start_time = getMilliseconds();
		milliseconds_t progress_time = 0;

		KALDI_VLOG(1) << "Started @ " << start_time << " ms";
        KALDI_LOG << "Request.phrase-id: " << request.phrase_id;
		InputStarted();

		int intermediate_counter = 1;
		int intermediate_samples_interval = request.IntermediateIntervalMillisec() > 0 ? request.IntermediateIntervalMillisec() * (request.Frequency() / 1000) : 0;
		int intermediate_frames_interval = request.IntermediateIntervalMillisec() > 0 ?
            request.IntermediateIntervalMillisec() / 1000 * frame_shift_ : 0;
        int prev_frames = 0;
		int max_samples_limit = max_record_size_seconds_ > 0 ? max_record_size_seconds_ * request.Frequency() : 0;

		std::vector<int32> prev_words;
        std::string prev_interim;
		int samples_per_chunk = int(chunk_length_secs_ * request.Frequency());

		int samp_counter = 0;

		kaldi::SubVector<kaldi::BaseFloat> *wave_part;

		bool do_endpointing = !request.DoEndpointing();
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
              json result = GetTranscription(false, request.BestCount());
              if (req_ && !req_->phrase_id.empty() &&
                  decode_graphs_->find(req_->phrase_id) != decode_graphs_->end())
                result["phrase-id"] = req_->phrase_id;
              else
                result["phrase-id"] = "<N/A>";
              result["model"] = nnet3_rxfilename_;
              if (result.find("text") != result.end() &&
                  prev_interim != result["text"]) {
                response.SendJson(result.dump(), true);
                if (result.find("text") != result.end())
                  prev_interim = result["text"];
                //} else {
                //  prev_interim = "";
                //  prev_words.clear();
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

		//std::vector<DecodedData> result;

		//int32 decoded = Decode(true, request.BestCount(), &result);

		//if (decoded == 0) {
		//	response.SetError("Best-path failed");
		//	KALDI_WARN << "Best-path failed";
		//} else {
		//	std::vector<RecognitionResult> recognitionResults;
		//	GetRecognitionResult(result, &recognitionResults);
            json result = GetTranscription(true, request.BestCount());
            if (req_ && !req_->phrase_id.empty() &&
                decode_graphs_->find(req_->phrase_id) != decode_graphs_->end())
              result["phrase-id"] = req_->phrase_id;
            else
              result["phrase-id"] = "<N/A>";
            result["model"] = nnet3_rxfilename_;
            response.SendJson(result.dump(), true);
			//response.SetResult(recognitionResults, decode_info, requestInterrupted, (samp_counter / (request.Frequency() / 1000)));
			KALDI_VLOG(1) << "Recognized @ " << getMillisecondsSince(start_time) << " ms";
            //}

		CleanUp();

		KALDI_VLOG(1) << "Decode subroutine done";
	} catch (std::runtime_error &e) {
		response.SetError(e.what());
	}
};

int32 OnlineDecoder::DecodeIntermediate(int bestCount, std::vector<DecodedData> *result) {
	return Decode(false, bestCount, result);
}

/* Hossein: this is a nice and clean version of Decode. Actualy it does the
   job of Decode() and GetRecognitionResult() and Json conversion and more
   in one function. */
json OnlineDecoder::GetTranscription(bool end_of_utterance, int bestCount) {
  KALDI_LOG << "Decoding lattice...end of utt: " << end_of_utterance;
  kaldi::CompactLattice clat;
  GetLattice(&clat, end_of_utterance);

  json result;
  result["status"] = end_of_utterance ? "ok" : "intermediate";
  if (clat.NumStates() == 0)
    return result;
  if (lm_scale_ != 0)
    fst::ScaleLattice(fst::LatticeScale(lm_scale_, 1.0), &clat);

  fst::SymbolTable *wordsyms;
  if (req_ && !req_->phrase_id.empty() &&
      ref_texts_->find(req_->phrase_id) != ref_texts_->end()) {
    KALDI_LOG << "Using the special word list for transcribing...";
    wordsyms = word_syms_;
  } else {
    wordsyms = default_word_syms_;
    KALDI_LOG << "Using the generic (default) word list for transcribing...";
  }

  if (!end_of_utterance) {
    kaldi::CompactLattice best_path_clat;
    kaldi::CompactLatticeShortestPath(clat, &best_path_clat);
    kaldi::Lattice best_path_lat;
    fst::ConvertLattice(best_path_clat, &best_path_lat);
    std::vector<int32> alignment;
    std::vector<int32> words;
    LatticeWeight weight;
    GetLinearSymbolSequence(best_path_lat, &alignment, &words, &weight);
    std::string joined_result;
    for (size_t i = 0; i < words.size(); i++) {
      std::string s = wordsyms->Find(words[i]);
      joined_result += s + " ";
    }
    result["text"] = joined_result;
    return result;
  }

  kaldi::Lattice lat;
  fst::ConvertLattice(clat, &lat);
  kaldi::Lattice nbest_lat;
  fst::ShortestPath(lat, &nbest_lat, bestCount);
  std::vector<kaldi::Lattice> nbest_lats;
  fst::ConvertNbestToVector(nbest_lat, &nbest_lats);

  int32 max_states = 1000 + 10 * clat.NumStates();
  CompactLattice clat_ali;
  WordAlignLattice(clat, *trans_model_, *word_boundary_info_,
                   max_states, &clat_ali);

  json data = json::array();
  for (int n = 0; n < nbest_lats.size(); ++n) {
    json this_result;
    std::vector<int32> alignment;
    std::vector<int32> aliwords;
    LatticeWeight weight;
    GetLinearSymbolSequence(nbest_lats[n], &alignment, &aliwords, &weight);
    BaseFloat loglike = -(weight.Value1() + weight.Value2()); // / acoustic_scale;
    this_result["log-like"] = loglike;

    MinimumBayesRiskOptions mbr_opts;
    mbr_opts.decode_mbr = false;
    MinimumBayesRisk mbr(clat_ali, aliwords, mbr_opts);
    const std::vector<BaseFloat> &confs = mbr.GetOneBestConfidences();
    const std::vector<std::pair<BaseFloat, BaseFloat> > &times =
        mbr.GetOneBestTimes();
    const std::vector<int32> &words = mbr.GetOneBest();
    json trans = json::array();
    BaseFloat log_tot_conf = 0.0;
    for (size_t i = 0; i < words.size(); i++) {
      log_tot_conf += Log(confs[i]);
      std::string s = wordsyms->Find(words[i]);
      json this_word;
      BaseFloat conf = confs[i];
      if (req_ && req_->conf_threshold - req_->conf_threshold == 0.0
          && req_->conf_default - req_->conf_default == 0.0
          && conf < req_->conf_threshold) {
        KALDI_LOG << "Setting conf " << conf
                  << " to default: " << req_->conf_default;
        conf = req_->conf_default;
      }
      this_word["value"] = s;
      this_word["confidence"] = conf;
      this_word["start_time"] = times[i].first * frame_shift_;
      this_word["end_time"] = times[i].second * frame_shift_;
      trans.push_back(this_word);
    }
    // this_result["transcript"] = trans;
    this_result["text"] = trans;
    this_result["confidence"] = Exp(log_tot_conf / confs.size());
    // result["best-" + std::to_string(n + 1)] = this_result;
    data.push_back(this_result);
  }
  result["data"] = data;

  // ref-conf example: [ ["he", 1], ["lived", 0.027912], ["two", 0.9123], ...]
  if (end_of_utterance && req_ && !req_->phrase_id.empty()) {
    KALDI_LOG << "Getting ref-text confidences...";
    if (ref_texts_->find(req_->phrase_id) != ref_texts_->end()) {
      const std::vector<int32> &one_best = ref_texts_->at(req_->phrase_id);
      if (one_best.size() > 25) {
        KALDI_LOG << "ref too long. skipping...";
        return result;
      }
      int32 max_states = 1000 + 10 * clat.NumStates();
      CompactLattice clat_ali;
      WordAlignLattice(clat, *trans_model_, *word_boundary_info_,
                       max_states, &clat_ali);

      MinimumBayesRiskOptions mbr_opts;
      mbr_opts.decode_mbr = false;
      MinimumBayesRisk mbr(clat_ali, one_best, mbr_opts);
      const std::vector<BaseFloat> &conf = mbr.GetOneBestConfidences();
      json ref = json::array();
      for (size_t i = 0; i < conf.size(); i++) {
        std::string s = wordsyms->Find(one_best[i]);
        ref.push_back(json({s, conf[i]}));
      }
      result["ref-conf"] = ref;
      // KALDI_LOG << "ref-conf: " << ss.str();

    } else {
      KALDI_LOG << "Eror: no ref text was found for phrase-id: "
                << req_->phrase_id;
    }
  }

  return result;
}


int32 OnlineDecoder::Decode(bool end_of_utterance, int bestCount, std::vector<DecodedData> *result) {
    KALDI_LOG << "Decoding lattice...end of utt: " << end_of_utterance;
	kaldi::CompactLattice clat;
	GetLattice(&clat, end_of_utterance);

	if (clat.NumStates() == 0) {
		return 0;
	}

	if (lm_scale_ != 0) {
		fst::ScaleLattice(fst::LatticeScale(lm_scale_, 1.0), &clat);
	}

    fst::SymbolTable *wordsyms;
    if (req_ && !req_->phrase_id.empty() &&
        ref_texts_->find(req_->phrase_id) != ref_texts_->end()) {
      KALDI_LOG << "Using the special word list for transcribing...";
      wordsyms = word_syms_;
    } else {
      wordsyms = default_word_syms_;
      KALDI_LOG << "Using the generic (default) word list for transcribing...";
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
      /*kaldi::CompactLattice best_path_clat;
		kaldi::CompactLatticeShortestPath(clat, &best_path_clat);

		kaldi::Lattice best_path_lat;
		fst::ConvertLattice(best_path_clat, &best_path_lat);
		DecodedData decodeData;
		GetLinearSymbolSequence(best_path_lat, &(decodeData.alignment), &(decodeData.words), &(decodeData.weight));
		getWeightMeasures(best_path_lat, &(decodeData.weights));
		result->push_back(decodeData);
		resultsNumber = 1;*/
      json res;
      /*
        [
                {
                          "value": "HE",
                                    "confidence": 0.958944,
                                              "start_time": 0.06,
                                                        "end_time": 0.18
                                                                },
       */
        int32 max_states = 1000 + 10 * clat.NumStates();
        CompactLattice clat_ali;
        WordAlignLattice(clat, *trans_model_, *word_boundary_info_,
                         max_states, &clat_ali);
        MinimumBayesRisk mbr(clat_ali);
        const std::vector<BaseFloat> &conf = mbr.GetOneBestConfidences();
        const std::vector<std::pair<BaseFloat, BaseFloat> > &times =
            mbr.GetOneBestTimes();
        const std::vector<int32> &words = mbr.GetOneBest();
        std::ostringstream ss;
        ss << "[\n";
        float log_tot_conf = 0.0;
        for (size_t i = 0; i < conf.size(); i++) {
          std::string s = wordsyms->Find(words[i]);
          ss << "{\n";
          ss << "\"value\": \"" << s << "\",\n";
          ss << "\"confidence\": \"" << conf[i] << "\",\n";
          log_tot_conf += Log(conf[i]);
          if (req_->provide_times) {
            ss << "\"start_time\": \"" << std::fixed << std::setprecision(2) << times[i].first * frame_shift_ << "\",\n";
            ss << "\"end_time\": \"" << std::fixed << std::setprecision(2) << times[i].second * frame_shift_ << "\"\n";
            //ss << s << "(" << conf[i] << ")[" << std::fixed
            //   << std::setprecision(2) << times[i].first * frame_shift_
            //   << "," << times[i].second * frame_shift_ << "] ";
          }
          //else
          //  ss << s << "(" << conf[i] << ") ";
          if (i < conf.size() - 1)
            ss << "},\n";
          else
            ss << "}\n";
        }
        ss << "]\n";
        resultsNumber = 1;
        DecodedData decode_data;
        decode_data.conf = Exp(log_tot_conf/conf.size());
        decode_data.text = ss.str();
        result->push_back(decode_data);
        KALDI_LOG << "Decoded: " << ss.str();
	}

    // ref-conf example: [ ["he", 1], ["lived", 0.027912], ["two", 0.9123], ...]
    if (end_of_utterance && req_ && !req_->phrase_id.empty()) {
      KALDI_LOG << "Getting ref-text confidences...";
      if (ref_texts_->find(req_->phrase_id) != ref_texts_->end()) {
        const std::vector<int32> &one_best = ref_texts_->at(req_->phrase_id);
        if (one_best.size() > 25) {
          KALDI_LOG << "ref too long. skipping...";
          return resultsNumber;
        }
        int32 max_states = 1000 + 10 * clat.NumStates();
        CompactLattice clat_ali;
        WordAlignLattice(clat, *trans_model_, *word_boundary_info_,
                         max_states, &clat_ali);

        MinimumBayesRiskOptions mbr_opts;
        mbr_opts.decode_mbr = false;
        MinimumBayesRisk mbr(clat_ali, one_best, mbr_opts);
        const std::vector<BaseFloat> &conf = mbr.GetOneBestConfidences();
        std::ostringstream ss;
        ss << "[ ";
        for (size_t i = 0; i < conf.size(); i++) {
          std::string s = wordsyms->Find(one_best[i]);
          ss << "[\"" << s << "\", " << conf[i] << "]";
          if (i < conf.size() - 1)
            ss << ", ";
        }
        ss << " ]";
        result->back().ref_conf = ss.str();
        // KALDI_LOG << "ref-conf: " << ss.str();

      } else {
        KALDI_LOG << "Eror: no ref text was found for phrase-id: "
                  << req_->phrase_id;
      }
    }

	return resultsNumber;
}

} /* namespace apiai */
