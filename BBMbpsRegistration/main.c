/*
 * Copyright (c) 2011-2012 Research In Motion Limited.
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

#include "BbmRegistration.h"

#include <bps/bps.h>
#include <bps/event.h>
#include <bbmsp/bbmsp_events.h>
#include <bbmsp/bbmsp.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Log the registration status and description if registration is done (failed
// or finished successfully).
void log_bbm_registration()
{
  const bbm_progress_t progress = get_bbm_registration_progress();
  if (progress == BBM_PROGRESS_FINISHED) {
    // TODO: Change this to suit your application.
    char status[256];
    char description[512];
    get_bbm_registration_text(status, 256, description, 512);
    fprintf(stdout, "BBM registration success=%d, status=%s, description=%s\n",
            bbmsp_is_access_allowed(), status, description);
  }
}

// Process BPS events.
void handle_events() {
  for(;;) {
    bps_event_t *event = NULL;
    // Need to flush stdout and stderr to make them appear in the sandbox
    // log file.
    fflush(stdout);
    fflush(stderr);
    bps_get_event(&event, -1);
    if (!event) {
      fprintf(stderr, "BPS event is NULL\n");
      return;
    }
    if (bps_event_get_domain(event) != bbmsp_get_domain()) {
      fprintf(stdout, "Received an event which is not handled\n");
      continue;
    }
    // Handle a BBM Social Platform event.
    int event_category = 0;
    int event_type = 0;
    bbmsp_event_t* bbmsp_event = NULL;
    bbmsp_event_get_category(event, &event_category);
    bbmsp_event_get_type(event, &event_type);
    bbmsp_event_get(event, &bbmsp_event);
    fprintf(stdout, "Received a BBMSP event: category=%d, type=%d\n",
            event_category, event_type);
    // Process registration events only.
    if (event_category == BBMSP_REGISTRATION &&
        event_type == BBMSP_SP_EVENT_ACCESS_CHANGED) {
      fprintf(stdout, "Processing a BBMSP registration event\n");
      process_bbm_registration_event(
        bbmsp_event_access_changed_get_access_error_code(bbmsp_event));
      log_bbm_registration();
      continue;
    }
    fprintf(stdout, "Received an event which is not handled\n");
  }
}

int main(int argc, char *argv[]) {
  // Initialize BPS library
  fprintf(stdout, "Initializing BPS: %d\n", bps_initialize());
  // Now we are ready to connect to BBM.
  fprintf(stdout, "Connecting to BBM\n");
  start_connecting_to_bbm();
  log_bbm_registration();
  handle_events();
  // Shut down BPS library for this process
  bps_shutdown();
  fprintf(stdout, "Exiting\n");
  fflush(stdout);
  fflush(stderr);
  return 0;
}
