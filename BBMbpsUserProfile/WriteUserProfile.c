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
#include <bbmsp/bbmsp_presence.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>

void write_profile_personal_message(bbmsp_profile_t* m_profile)
{
	char message[46] = "Updated by the BBMSP bps User Profile sample!";
	if (BBMSP_FAILURE == bbmsp_set_user_profile_personal_message(message))
	{
		fprintf(stdout, "Failed to update BBM SP personal message.\n");
	} else
	{
		fprintf(stdout, "Updated BBM SP personal message to:%s.\n", message);
	}

}

void write_profile_status_message(bbmsp_profile_t* m_profile)
{
	bbmsp_presence_status_t status;

	if (BBMSP_FAILURE == bbmsp_profile_get_status(m_profile, &status))
	{
		fprintf(stdout, "ERROR: cannot get profile status\n");
	} else
	{
		char statusMessage[14] = "Sample Status";

		if (BBMSP_FAILURE
				== bbmsp_set_user_profile_status(status, statusMessage))
		{
			fprintf(stdout, "ERROR: cannot update status.\n");
		} else
		{
			fprintf(stdout, "Updated BBM SP status message to:%s.\n",
					statusMessage);
		}
	}
}

int write_profile_avatar(bbmsp_profile_t* m_profile)
{
	//Image Source: http://openclipart.org/detail/3320/earth-by-barretr
	//Max image width 333;
	//Max image height = 333;
	//Max image size = 32768;

	fprintf(stdout, "Updating profile avatar\n");

	//Get the current dir, which is where the Earth.png sample image is located.
	char imagePath[1024];
	char imageName[22] = "/app/native/Earth.png";
	char* imageBuf = NULL;
	int  len;
	FILE* fp = NULL;
	bbmsp_result_t rc = BBMSP_FAILURE;

	getcwd(imagePath, sizeof(imagePath));
	strcat(imagePath, imageName);

	// Open the source file
	fp = fopen(imagePath, "r");
	if (!fp)
	{
		fprintf(stdout, "Failed to open file: %s\n", imagePath);
		return 0;
	}

	// Get its length (in bytes)
	if (fseek(fp, 0, SEEK_END) != 0)
	{
		fclose(fp);
		fprintf(stdout, "Failed to get file size.\n");
		return 0;
	}

	len = ftell(fp);
	rewind(fp);

	// Get a buffer big enough to hold it entirely
	imageBuf = (char*) malloc(len);
	if (!imageBuf)
	{
		fclose(fp);
		fprintf(stdout, "Failed to allocate image buffer.\n");
		return 0;
	}

	// Read the entire file into the buffer
	if (!fread(imageBuf, len, 1, fp))
	{
		free(imageBuf);
		fclose(fp);
		fprintf(stdout, "Failed to read file.\n");
		return 0;
	}
	fclose(fp);

	//Done reading the file, now onto the BBM SP stuff.

	// Create the icon object and register the icon
	bbmsp_image_t* bbmspImage = NULL;
	bbmspImage=0;
	bbmsp_image_create_empty(&bbmspImage);
	rc = bbmsp_image_create(&bbmspImage, BBMSP_IMAGE_TYPE_JPG,
			imageBuf, len);

	if(BBMSP_SUCCESS == rc)
	{
		fprintf(stdout, "Created bbmsp_image_t");
		if (BBMSP_SUCCESS == bbmsp_set_user_profile_display_picture(bbmspImage))
		{
			fprintf(stdout, "Set new avatar image.\n");
		}
		bbmsp_image_destroy(&bbmspImage);
	}
	else
	{
		fprintf(stdout, "Failed to create bbmsp_image_t\n");
	}

	free(imageBuf);

	fprintf(stdout, "The path:%s", imagePath);

	return 0;
}

void write_user_profile_data()
{
	fprintf(stdout, "Reading BBM SP User Profile\n");

	bbmsp_profile_t* m_profile = 0;
	// Create an empty profile object for the library to use.
	bbmsp_profile_create(&m_profile);

	// Populate the profile object.
	bbmsp_get_user_profile(m_profile);

	write_profile_personal_message(m_profile);
	write_profile_status_message(m_profile);
	write_profile_avatar(m_profile);

	fflush(stdout);
}
