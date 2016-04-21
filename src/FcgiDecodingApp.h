// FcgiDecodingApp.h

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

#ifndef APIAI_DECODER_FCGIDECODINGAPP_H_
#define APIAI_DECODER_FCGIDECODINGAPP_H_

#include "Decoder.h"

namespace apiai {

/**
 * Decoding class with main routine defined.
 * Data IO implemented via FastCGI gate.
 * Input data expected as raw audio stream
 * Output data is JSON encoded objects
 */
class FcgiDecodingApp {
public:
	/** Initialize with given decoder */
	FcgiDecodingApp(Decoder &decoder) : decoder_(decoder),
		fcgi_threads_number_(1), fcgi_socket_backlog_(0), socket_id_(0),
		running_(false) {};

	/** Get run specifications and allowed arguments list */
	std::string &Usage() { return usage_; }
	/** Set run specifications and allowed arguments list */
	void Usage(std::string &usage) { usage_ = usage; }

	/** Run main routine and pass all given arguments */
	int Run(int argn, char **argv);
private:
	void RegisterOptions(kaldi::OptionsItf &po);
	void ProcessingRoutine(Decoder &decoder);
	static void *RunChildThread(void *app);

	Decoder &decoder_;
	std::string usage_;

	int fcgi_threads_number_;
	std::string fcgi_socket_path_;
	int fcgi_socket_backlog_;
	int socket_id_;
	bool running_;
};

} /* namespace apiai */

#endif /* APIAI_DECODER_FCGIDECODINGAPP_H_ */
