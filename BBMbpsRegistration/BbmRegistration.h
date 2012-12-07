/*
 * Copyright (c) 2012 Research In Motion Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BBM_REGISTRATION_HH_20121117010101
#define BBM_REGISTRATION_HH_20121117010101

#include <bbmsp/bbmsp_context.h>

#include <stdlib.h>


// Typical usage:
//
// application starts
//
// bps_initialize();
//
// start_connecting_to_bbm();
//
// use get_bbm_registration_progress() to see if registration is done
//
// inside the BPS event loop:
//
//  if the event is a BBM Social Platform registration event,
//    call process_bbm_registration_event(status);
//    use get_bbm_registration_progress() to see if registration is done
//

// BBM registration progress enumeration.
typedef enum
{
  // Registration has not started and has never been attempted since the
  // application started.
  BBM_PROGRESS_NOT_STARTED = 1,
  // Registration has started.
  BBM_PROGRESS_STARTED = 2,
  // Registration is in progress.
  BBM_PROGRESS_PENDING = 3,
  // Registration is done. Use bbmsp_is_access_allowed() to check if it
  // finished successfully.
  BBM_PROGRESS_FINISHED = 4
} bbm_progress_t;

// Initiate connecting to BBM. Assumes that BPS is already initialized.
void start_connecting_to_bbm();

// Process a registration event.
// \a status is the registration status carried by a BPS event.
void process_bbm_registration_event(const bbmsp_access_error_codes_t status);

// Return the registration progress.
int get_bbm_registration_progress();

// Placeholder to insert code when BBM registration is done (successful or
// otherwise).
void bbm_registration_finished();

// Updates the \a status buffer with a short description of the registration
// status (Connected, Not Connected, Connecting..., etc). Updates the
// \a description buffer with a longer description of what the status means and
// what the user can do next to connect the application to BBM.
//
// The \a status_size is the size of the \a status buffer. This should be 256.
//
// The \a description_size is the size of the \a description buffer. This should
// be 512.
//
void get_bbm_registration_text(char* status,
                               const size_t status_size,
                               char* description,
                               const size_t description_size);

#endif
