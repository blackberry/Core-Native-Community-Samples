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

#include "BbmRegistration.h"
#include "ReadUserProfile.h"
#include "WriteUserProfile.h"

#include <bps/bps.h>
#include <bbmsp/bbmsp.h>

#include <stdio.h>
#include <string.h>

// Keep track of whether the application needs to register.
bbm_progress_t progress = BBM_PROGRESS_NOT_STARTED;

void start_connecting_to_bbm() {
  progress = BBM_PROGRESS_STARTED;
  // Initialize the BBM Social Platform. bbmsp_request_events() must be called
  // only once in your application.
  if (bbmsp_request_events(0) == BBMSP_FAILURE) {
    fprintf(stderr, "bbmsp_request_events(0) failed\n");
    // Fatal error. Cannot use BBM APIs. Restarting the application may produce
    // a different result. Check that you are using the BlackBerry Messenger
    // permission in your bar-descriptor.xml file.
    progress = BBM_PROGRESS_FINISHED;
    bbm_registration_finished();
    return;
  }
  // Process the initial access status.
  process_bbm_registration_event(bbmsp_get_access_code());
}

void process_bbm_registration_event(const bbmsp_access_error_codes_t status)
{
  // Got a registration access status event. Based on the status, decide whether
  // we need to register. If we already registered successfully once (i.e. on a
  // previous application run), then we will not call bbmsp_register() again.
  fprintf(stdout, "BBMSP access status=%d\n", status);

  // Determine what to do based on the progress.
  switch (progress) {
  case BBM_PROGRESS_PENDING:
    // Registration is in progress. Check the status to see if it finished.
    if (status == BBMSP_ACCESS_PENDING) {
      // Ignore. This means the registration is in progress.
      return;
    }
    // Registration is done.
    progress = BBM_PROGRESS_FINISHED;
    fprintf(stdout, "Finished BBM registration, success=%d\n",
            bbmsp_is_access_allowed());
    bbm_registration_finished();
    return;

  case BBM_PROGRESS_STARTED:
    if (bbmsp_is_access_allowed()) {
      // Access is allowed, the application is registered.
      progress = BBM_PROGRESS_FINISHED;
      fprintf(stdout, "Successfully connected to BBM\n");
      bbm_registration_finished();
      return;
    }
    // Need to register.
    if (status == BBMSP_ACCESS_UNKNOWN) {
      // Status is not yet known. Wait for an event that will deliver the
      // status.
      fprintf(stdout, "BBMSP access status is UNKNOWN; waiting for initial "
              "status\n");
      return;
    }
    // Start registration.
    // Attempt to register the application with the following UUID.
    // Every application is required to have its own unique UUID. You should
    // keep using the same UUID when you release a new version of your
    // application.
    // TODO:  YOU MUST CHANGE THIS UUID!
    // Change this when creating your own application.
    // You can generate one here: http://www.guidgenerator.com/
    const char* const UUID = ;

    if (bbmsp_register(UUID)) {
      // Registration started. The user will see a dialog informing them that
      // your application is connecting to BBM.
      progress = BBM_PROGRESS_PENDING;
      fprintf(stdout, "BBMSP registration started\n");
      fprintf(stdout, "Verify that you are specifying a valid UUID\n");
      return;
    }
    // Could not start registration. No dialogs were shown.
    progress = BBM_PROGRESS_FINISHED;
    fprintf(stdout, "BBMSP registration could not be started\n");
    bbm_registration_finished();
    return;

  default:
    // Ignoring these cases.
    fprintf(stdout, "BBMSP access status ignored, progress=%d\n", progress);
    return;
  }
}

int get_bbm_registration_progress()
{
  return progress;
}

void bbm_registration_finished()
{
  if (bbmsp_is_access_allowed()) {
	  write_user_profile_data();
//	  read_user_profile_data();
  } else {
    // Insert your code here.
  }
}

void get_bbm_registration_text(char* status,
                               const size_t status_size,
                               char* description,
                               const size_t description_size)
{
  // You can use the following code to update your UI component to indicate
  // whether your application is connected to BBM.
  // TODO: MODIFY FOR YOUR APPLICATION
  switch (bbmsp_get_access_code()) {
  case BBMSP_ACCESS_ALLOWED:
    strlcpy(status, "Connected", status_size);
    strlcpy(description, "BBM functionality is available", description_size);
    return;

  case BBMSP_ACCESS_UNKNOWN:
    strlcpy(status, "Unknown", status_size);
    strlcpy(description, "Wait for the status to refresh", description_size);
    return;

  case BBMSP_ACCESS_UNREGISTERED: {
    strlcpy(status, "Not Connected", status_size);
    // The application should be registering on start up. You can also allow
    // the user to trigger registration by clicking a button.
    strlcpy(description, "Would you like to connect the application to BBM?",
            description_size);
    return;
  }
  case BBMSP_ACCESS_PENDING: {
    strlcpy(status, "Connecting...", status_size);
    // The user will never see this. The BBM Social Platform already displays
    // a "Connecting" dialog.
    strlcpy(description, "Connecting to BBM. Please wait.", description_size);
    return;
  }
  case BBMSP_ACCESS_BLOCKED_BY_USER:
    strlcpy(status, "Disconnected", status_size);
    strlcpy(description, "Go to Settings -> Security and Privacy -> "
            "Application Permissions and connect this application to BBM",
            description_size);
    return;

  case BBMSP_ACCESS_BLOCKED_BY_RIM:
    strlcpy(status, "Disconnected by RIM", status_size);
    strlcpy(description, "RIM is preventing this application from connecting to"
            " BBM", description_size);
    return;

  case BBMSP_ACCESS_NO_DATA_CONNECTION:
    strlcpy(status, "Not Connected", status_size);
    // The application should be registering on start up. You can also allow
    // the user to trigger registration by clicking a button.
    strlcpy(description, "Would you like to connect the application to BBM?",
            description_size);
    return;

  case BBMSP_ACCESS_UNEXPECTED_ERROR:
    strlcpy(status, "Not Connected", status_size);
    // The application should be registering on start up. You can also allow
    // the user to trigger registration by clicking a button.
    strlcpy(description, "Would you like to connect the application to BBM?",
            description_size);
    return;

  case BBMSP_ACCESS_INVALID_UUID:
    // You should be resolving this error at development time.
    strlcpy(status, "Not Connected", status_size);
    strlcpy(description, "Invalid UUID. Report this error to the vendor.",
            description_size);
    return;

  case BBMSP_ACCESS_TEMPORARY_ERROR:
    strlcpy(status, "Not Connected", status_size);
    // The application should be registering on start up. You can also allow
    // the user to trigger registration by clicking a button.
    strlcpy(description, "Would you like to connect the application to BBM?",
            description_size);
    return;

  case BBMSP_ACCESS_MAX_DOWNLOADS_REACHED:
    strlcpy(status, "Not Connected", status_size);
    strlcpy(description, "Cannot connect to BBM. Download this application from"
            " AppWorld to keep using it.", description_size);
    return;

  case BBMSP_ACCESS_EXPIRED:
    strlcpy(status, "Not Connected", status_size);
    strlcpy(description, "Cannot connect to BBM. Download this application from"
            " AppWorld to keep using it.", description_size);
    return;

  case BBMSP_ACCESS_CANCELLED_BY_USER:
    strlcpy(status, "Not Connected", status_size);
    // The application should be registering on start up. You can also allow
    // the user to trigger registration by clicking a button.
    strlcpy(description, "Would you like to connect the application to BBM?",
            description_size);
    return;

  case BBMSP_ACCESS_MAX_APPS_REACHED:
    strlcpy(status, "Not Connected", status_size);
    strlcpy(description, "Too many applications are connected to BBM. Uninstall"
            " one or more applications and try again.", description_size);
    return;

// This error code will be available in the next NDK release.
//  case BBMSP_ACCESS_BLOCKED_BBM_DISABLED:
//    strlcpy(status, "Not Connected", status_size);
//    strlcpy(description, "BBM is not setup. Open BBM to set it up and try again.",
//            description_size);
//    return;

  default:
    // New codes may be added in the future.
    strlcpy(status, "Not Connected", status_size);
    strlcpy(description, "Would you like to connect the application to BBM?",
            description_size);
    return;
  }
}
