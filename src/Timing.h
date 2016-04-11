// Timing.h

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

#ifndef TIMING_H_
#define TIMING_H_

#include <sys/time.h>

namespace apiai {

typedef long int milliseconds_t;

/**
 * Get current time of specified time zone in milliseconds
 */
milliseconds_t getMilliseconds(struct timezone *tz);

/**
 * Get time difference between current time of specified time zone and the given time
 */
milliseconds_t getMillisecondsSince(milliseconds_t since, struct timezone *tz);

/**
 * Get current time in milliseconds
 */
milliseconds_t getMilliseconds();

/**
 * Get time difference between current time and the given
 */
milliseconds_t getMillisecondsSince(milliseconds_t since);

} /* namespace apiai */


#endif /* TIMING_H_ */
