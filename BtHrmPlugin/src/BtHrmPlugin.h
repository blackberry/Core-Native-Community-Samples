/* Copyright (c) 2013 BlackBerryn Limited.
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
#ifndef BtHrmPlugin_H_
#define BtHrmPlugin_H_

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

//#include <unistd.h>
//#include <pthread.h>
//#include <stdarg.h>
//#include <stdio.h>
//#include <gulliver.h>
//#include <math.h>
//#include <time.h>
#include <pthread.h>
//#include <time.h>
//#include <sys/timeb.h>

#include <btapi/btdevice.h>
#include <btapi/btgatt.h>
#include <btapi/btle.h>

#include "BtHrmPluginTypes.h"
#include "BtHrmPluginExtern.h"

#define LOGX(...)                            \
	do {                                     \
		fprintf(stderr, "XXXX " __VA_ARGS__);\
		fflush(stderr);                      \
	} while (0)

#define LOGU(...)                            \
	do {                                     \
		char __temp[500];                      \
		sprintf(__temp, __VA_ARGS__);          \
		UnitySendMessage("BtHrmPlugin", "BtHrmMessage", __temp); \
	} while (0)

#define LOGUX(...)                            \
	do {                                     \
		char __temp[500];                      \
		sprintf(__temp, __VA_ARGS__);          \
		UnitySendMessage("BtHrmPlugin", "BtHrmMessage", __temp); \
		fprintf(stderr, "XXXX " __VA_ARGS__);\
		fflush(stderr);                      \
	} while (0)

/*
 * Global constants
 */

#define HEART_RATE_SERVICE_UUID "0x180D"
#define HEART_RATE_MEASUREMENT "0x2A37"
#define API_VERSION_NUMBER "0.1.0"

// FLAGS field bit mask values
#define HEART_RATE_VALUE_FORMAT (1)
#define SENSOR_CONTACT_DETECTED (2)
#define SENSOR_CONTACT_FEATURE (4)
#define ENERGY_EXPENDED_FEATURE (8)
#define RR_INTERVAL_DATA (16)

/*
 * API version of plugin
 */

static char *API_VERSION = API_VERSION_NUMBER;

/*
 * Global variables
 */

static bool g_bt_initialised = false;
static bool g_hrm_service_initialised = false;
static bt_gatt_callbacks_t g_gatt_callbacks;
static uint16_t g_handle;
static uint16_t g_value_handle;
static uint8_t g_hrm_data;
static pthread_attr_t g_attr_s;
static pthread_t g_scanthread;
static bool g_scan_thread_running = false;
static pthread_mutex_t g_scan_mutex;

/*
 * Call backs
 */

/*
 * Forward declarations
 */
static void bt_event(const int event, const char *bt_addr, const char *event_data);
static const char *bt_event_name(const int id);
static void gatt_service_connected(const char *bt_addr, const char *service, int instance, int err, uint16_t conn_int, uint16_t latency, uint16_t super_timeout, void *user_data);
static void gatt_service_disconnected(const char *bt_addr, const char *service, int instance, int reason, void *user_data);
static void gatt_service_updated(const char *bt_addr, int instance, uint16_t conn_int, uint16_t latency, uint16_t super_timeout, void *user_data);
static void notifications_cb(int instance, uint16_t handle, const uint8_t *val, uint16_t len, void *user_data);
static void *scanForHrmDevicesThread(void *arg);
static int scanForHrmDevices();

#endif /* BtHrmPlugin_H_ */
