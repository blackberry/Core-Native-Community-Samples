/* Copyright (c) 2013 BlackBerry Limited.
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

#ifndef BtHrmTPluginTypes_H_
#define BtHrmTPluginTypes_H_

typedef struct _event_names_t {
	int id;
	const char *name;
} event_names_t;

#define DEVICE_INFO_BUFF_SIZE (256)
#define DEVICE_INFO_ADDR_SIZE (18)

typedef struct _btle_device_info {
	char address[DEVICE_INFO_ADDR_SIZE]; // '\0' terminated string
	char name[DEVICE_INFO_BUFF_SIZE];    // '\0' terminated string
} btle_bevice_info_t;

typedef void (*call_back_type_A_cb) (const char *a_string);
typedef void (*call_back_type_B_cb) (const int an_int);

#endif /* BtHrmTPluginTypes_H_ */
