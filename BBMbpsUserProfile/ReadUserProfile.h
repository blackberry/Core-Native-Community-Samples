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


#ifndef READUSERPROFILE_H_
#define READUSERPROFILE_H_

#include "bbmsp/bbmsp_userprofile.h"


void read_user_profile_data();

/*
 * Reads the users BBM SP display name and outputs it to the console.
 */
void read_profile_display_name(bbmsp_profile_t*);

/*
 * Reads the users BBM SP profile message and outputs it to the console.
 */
void read_profile_personal_message(bbmsp_profile_t*);

/*
 * Reads the users BBM SP status and outputs it to the console. 0 = available, 1 = busy.
 */
void read_profile_status(bbmsp_profile_t*);


/*
 * Reads the users BBM SP status message and outputs it to the console.
 */
void read_profile_status_message(bbmsp_profile_t*);


#endif /* READUSERPROFILE_H_ */
