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

#include "bbmsp/bbmsp_userprofile.h"

#include <stdio.h>
#include <string.h>

void read_profile_display_name(bbmsp_profile_t* m_profile)
{
	  char displayName[BBMSP_PROFILE_DISPLAY_NAME_MAX];
	  if(BBMSP_FAILURE ==
	      bbmsp_profile_get_display_name(m_profile, displayName,
	                                     BBMSP_PROFILE_DISPLAY_NAME_MAX)) {
		  displayName[0] = 0;
		  fprintf(stdout, "Error: Failed to get Display Name.\n");
	  }

	  fprintf(stdout, "BBMSP display name=%s\n", displayName);
}




void read_profile_personal_message(bbmsp_profile_t* m_profile)
{
	  char personalMessage[BBMSP_PROFILE_PERSONAL_MSG_MAX];
	  if(BBMSP_FAILURE ==
	      bbmsp_profile_get_personal_message(m_profile,
	                                         personalMessage,
	                                         BBMSP_PROFILE_PERSONAL_MSG_MAX)){
		  personalMessage[0] = 0;
		  fprintf(stdout, "Error: Failed to get Personal Message.\n");
	  }
	  fprintf(stdout, "BBMSP personal message=%s\n", personalMessage);

}




void read_profile_status(bbmsp_profile_t* m_profile)
{
	  bbmsp_presence_status_t status;
	  if (bbmsp_profile_get_status(m_profile, &status) != BBMSP_SUCCESS) {
	    fprintf(stdout, "ERROR: cannot get profile status\n");
	  }
	  fprintf(stdout, "BBMSP status=%d\n", status);

}




void read_profile_status_message(bbmsp_profile_t* m_profile)
{
	  char statusMessage[BBMSP_PROFILE_STATUS_MSG_MAX];
	  if(BBMSP_FAILURE ==
	      bbmsp_profile_get_status_message(m_profile,
	                                       statusMessage,
	                                       BBMSP_PROFILE_STATUS_MSG_MAX)){
		  statusMessage[0] = 0;
		  fprintf(stdout, "Error: Failed to get Status Message.\n");
	  }
	  fprintf(stdout, "BBMSP status message=%s\n", statusMessage);

}




void read_user_profile_data()
{
	fprintf(stdout, "Reading BBM SP User Profile\n");

	bbmsp_profile_t* m_profile = 0;
	// Create an empty profile object for the library to use.
	bbmsp_profile_create(&m_profile);

	// Populate the profile object.
	bbmsp_get_user_profile(m_profile);

	read_profile_display_name(m_profile);
	read_profile_personal_message(m_profile);
	read_profile_status(m_profile);
	read_profile_status_message(m_profile);

	fflush(stdout);
}
