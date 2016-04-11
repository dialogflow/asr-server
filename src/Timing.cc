// Timing.cc

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

#include "Timing.h"

namespace apiai {

milliseconds_t getMilliseconds(struct timezone *tz) {
	timeval tv;
	if (!gettimeofday(&tv, tz)) {
		return tv.tv_sec * 1000 + tv.tv_usec / 1000;
	} else {
		return 0;
	}
}

milliseconds_t getMillisecondsSince(milliseconds_t since, struct timezone *tz) {
	return getMilliseconds(tz) - since;
}

milliseconds_t getMilliseconds() {
	return getMilliseconds(0);
}

milliseconds_t getMillisecondsSince(milliseconds_t since) {
	return getMillisecondsSince(since, 0);
}


} /* namespace apiai */
