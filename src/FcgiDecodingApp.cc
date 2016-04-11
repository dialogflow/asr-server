// FcgiDecodingApp.cc

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
#include "ResponseJsonWriter.h"
#include "FcgiDecodingApp.h"
#include "QueryStringParser.h"
#include <fastcgi/fcgio.h>
#include <vector>
#include <string>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

namespace apiai {

void FcgiDecodingApp::RegisterOptions(kaldi::OptionsItf &po) {
    po.Register("fcgi-socket", &fcgi_socket_path_, "FCGI connection string, if undefined then stdin and stdout will be used");
    po.Register("fcgi-socket.backlog", &fcgi_socket_backlog_, "FCGI socket backlog size.");
}

void FcgiDecodingApp::ProcessingRoutine() {
	if (socket_id_ < 0) {
		KALDI_WARN << "Socket not opened";
		return;
	}

    FCGX_Request request;
    FCGX_InitRequest(&request, socket_id_, 0);

    while (FCGX_Accept_r(&request) == 0) {
        fcgi_streambuf cin_fcgi_streambuf(request.in);
        fcgi_streambuf cout_fcgi_streambuf(request.out);
        fcgi_streambuf cerr_fcgi_streambuf(request.err);

		std::istream fcgiin(&cin_fcgi_streambuf);
		std::ostream fcgiout(&cout_fcgi_streambuf);
		std::ostream fcgierr(&cerr_fcgi_streambuf);

        fcgiout << "Content-type: application/json\r\n"
             << "\r\n";

		try {
			RequestRawReader reader(&fcgiin);
			ResponseJsonWriter writer(&fcgiout);

			char *queryString = FCGX_GetParam("QUERY_STRING", request.envp);
			if (queryString) {
				QueryStringParser queryStringParser(queryString);
				std::string name, value;
				while (queryStringParser.Next(&name, &value)) {
					if ("nbest" == name) {
						reader.BestCount(atoi(value.data()));
						KALDI_VLOG(1) << "Setting n-best: " << reader.BestCount();
					} else if ("intermediate" == name) {
						reader.IntermediateIntervalMillisec(atoi(value.data()));
						KALDI_VLOG(1) << "Setting intermediate interval: " << reader.IntermediateIntervalMillisec() << " ms";
					} else {
						KALDI_VLOG(1) << "Skipping unknown parameter \"" << name << "\"";
					}
				}
			}

			decoder_.Decode(reader, writer);
		} catch (std::exception &e) {
			KALDI_LOG << "Fatal exception: " << e.what();
		}

		FCGX_Finish_r(&request);
    }
}

int FcgiDecodingApp::Run(int argc, char **argv) {

	if (socket_id_ >= 0) {
		KALDI_WARN << "Socket already opened";
		return 1;
	}

	// Predefined configuration args
	const char *extra_args[] = {
		"--feature-type=mfcc",
		"--mfcc-config=mfcc.conf",
		"--frame-subsampling-factor=3",
		"--max-active=2000",
		"--beam=15.0",
		"--lattice-beam=6.0",
		"--acoustic-scale=1.0"
	};

    FCGX_Init();

    kaldi::ParseOptions po(usage_.data());
    RegisterOptions(po);
    decoder_.RegisterOptions(po);

    std::vector<const char*> args;
    args.push_back(argv[0]);
    args.insert(args.end(), extra_args, extra_args + sizeof(extra_args) / sizeof(extra_args[0]));
    args.insert(args.end(), argv + 1, argv + argc);
    po.Read(args.size(), args.data());

    if (fcgi_socket_path_.size() > 0) {
		socket_id_ = FCGX_OpenSocket(fcgi_socket_path_.data(), fcgi_socket_backlog_);
		if (socket_id_ < 0) {
			KALDI_WARN << "Error opening socket" << fcgi_socket_path_ << "(backlog: " << fcgi_socket_backlog_ << ")";
			return 1;
		} else {
			KALDI_LOG << "Listening FastCGI data at \"" << fcgi_socket_path_ << "\"";
		}
    } else {
    	KALDI_LOG << "Listening FastCGI data at stdin";
    }

	if (!decoder_.Initialize(po)) {
		po.PrintUsage();
	    return 1;
	}

	KALDI_VLOG(1) << "Single thread ready";
	ProcessingRoutine();

	return 0;
}
} /* namespace apiai */
