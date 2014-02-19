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
#include "BtHrmPlugin.h"
/*
 * Externally visible function
 */
int initialiseBtLeImpl()
{
	LOGX("In initialiseBtLeImpl()\n");

	if (g_bt_initialised) {
		LOGX("Bluetooth Already Initialised");
		return -1;
	}

	g_gatt_callbacks.connected = gatt_service_connected;
    g_gatt_callbacks.disconnected = gatt_service_disconnected;
    g_gatt_callbacks.updated = gatt_service_updated;

	int rc = bt_device_init(bt_event);

	if (rc == EOK) {
		LOGX("Bluetooth bt_device_init() OK\n");

		if (!bt_ldev_get_power()) {
			LOGX("Bluetooth calling bt_ldev_set_power()\n");
	        bt_ldev_set_power(true);
	    }

	    bt_gatt_init(&g_gatt_callbacks);

	    g_bt_initialised = true;

	    pthread_mutex_init(&g_scan_mutex, NULL);

	} else {
		LOGUX("Bluetooth bt_device_init() error - rc=%d, error=%s\n", rc, strerror(errno));

		g_bt_initialised = false;
	}

	return rc;
}

/*
 * Externally visible function
 */
int terminateBtLeImpl()
{
	LOGX("In terminateBtLeImpl()\n");

	g_bt_initialised = false;
    pthread_mutex_destroy(&g_scan_mutex);

	bt_device_deinit();
	bt_gatt_deinit();

	return 0;
}

/*
 * Externally visible function
 */

int scanForHrmDevicesImpl()
{
	LOGX("In scanForHrmDevicesImpl()\n");

	if (!g_bt_initialised) {
	    LOGUX("BT Not initialised\n");
	    return -1;
	}

	if (g_scan_thread_running) {
	    LOGUX("Scan already in progress\n");
	    return -1;
	}

	int policy;
    struct sched_param param;

    pthread_attr_init(&g_attr_s);
    pthread_attr_setdetachstate(&g_attr_s, PTHREAD_CREATE_DETACHED); // not interested in joining thread
    pthread_attr_setinheritsched(&g_attr_s, PTHREAD_EXPLICIT_SCHED);
    pthread_getschedparam(pthread_self(), &policy, &param);

    param.sched_priority=12;

    pthread_attr_setschedparam(&g_attr_s, &param);
    pthread_attr_setschedpolicy(&g_attr_s, SCHED_RR);
    return (pthread_create(&g_scanthread, &g_attr_s, &scanForHrmDevicesThread, NULL));
}

/*
 * Externally visible function
 */

int startMonitoringImpl(const char *address)
{
	LOGX("In startMonitoring(char *address)\n");
	LOGX("address=%s\n", address);

	if (!g_bt_initialised) {
	    LOGX("BT Not initialised\n");
	    return -1;
	}

	bt_gatt_conn_parm_t conParm;
	conParm.minConn = 0x30;
	conParm.maxConn = 0x50;
	conParm.latency = 0;
	conParm.superTimeout = 50;

	g_hrm_service_initialised = false;

	LOGX("calling bt_gatt_connect_service()\n");
	LOGX("address=%s\n", address);
	LOGX("HRM Service=%s\n", HEART_RATE_SERVICE_UUID);
	if (bt_gatt_connect_service(address, HEART_RATE_SERVICE_UUID, NULL, &conParm, NULL) < 0) {

		LOGUX("GATT connect service request failed: %s\n", strerror(errno));

		// there's a known issue where sometimes we get ERRNO=EBUSY (16)
		// when this is not the case and we've connected to the service OK.
		// So for now we ignore this errno value.

		if (errno != EBUSY) {
			return -1;
		}
		return EOK;

	} else {
		LOGX("requested connection to HR service OK\n");
		return EOK;
	}
}

/*
 * Externally visible function
 */

int stopMonitoringImpl(const char *address)
{
	LOGX("In stopMonitoring()\n");
	if (bt_gatt_disconnect_service(address, HEART_RATE_SERVICE_UUID) < 0) {
		LOGUX("GATT disconnect service request failed: %s\n", strerror(errno));
		return -1;
	} else {
		LOGX("GATT disconnect from HR service request OK\n");
		g_hrm_service_initialised = false;
		return EOK;
	}
}

/*
 * Static functions after this point
 */

static void *scanForHrmDevicesThread(void *arg)
{
	(void)arg; // suppress warning message

	pthread_mutex_lock(&g_scan_mutex);
	g_scan_thread_running = true;
	pthread_mutex_unlock(&g_scan_mutex);

	LOGX("scanForHrmDevicesThread() - entered - thread=%d\n", pthread_self());
	scanForHrmDevices();

	pthread_mutex_lock(&g_scan_mutex);
	g_scan_thread_running = false;
	pthread_mutex_unlock(&g_scan_mutex);

	return (NULL);
}

static int scanForHrmDevices()
{
	LOGX("In scanForHrmDevices()\n");

	bt_disc_start_inquiry(BT_INQUIRY_GIAC);

    bt_remote_device_t **remote_device_array = 0;
    bt_remote_device_t *remote_device = 0;
    int total_device_count = 0;

    LOGX("Calling bt_disc_retrieve_by_service\n");

    remote_device_array = bt_disc_retrieve_devices(BT_DISCOVERY_ALL, &total_device_count);

    LOGX("Total HR devices=%d\n", total_device_count);

    btle_bevice_info_t device_info[total_device_count];

    int hrm_device_count = 0;

    if (remote_device_array) {
    	int hrm_device_index;
    	int number_of_services = 0;
    	int i;
        for (i = 0; (remote_device = remote_device_array[i]); ++i) {
            const int device_type = bt_rdev_get_type(remote_device);

            LOGX("Device type=%d\n", device_type);

            if ((device_type == BT_DEVICE_TYPE_LE_PUBLIC) || (device_type == BT_DEVICE_TYPE_LE_PRIVATE)) {
                LOGX("Got a Bluetooth LE device\n");
        		char **services_array = bt_rdev_get_services_gatt(remote_device);
        		if (services_array) {
                    LOGX("Enumerating services on device\n");
                    int j;
        			for (j = 0; services_array[j]; j++) {
        				LOGX("Found service: %s\n", services_array[j]);
        				if (strcmpi(services_array[j], HEART_RATE_SERVICE_UUID) == 0) {
                			hrm_device_index = hrm_device_count++;
                            bt_rdev_get_address(remote_device, device_info[hrm_device_index].address);
                            LOGX("Address=%s\n", device_info[hrm_device_index].address);
                            bt_rdev_get_friendly_name(remote_device, device_info[hrm_device_index].name, sizeof(device_info[hrm_device_index].name));
                            LOGX("Name=%s\n", device_info[hrm_device_index].name);
        				}
        				number_of_services++;
        			}
        			bt_rdev_free_services(services_array);
        		} else {
                    LOGX("Unable to get service list - errno: %s\n", strerror(errno));
        		}
            }
        }
		bt_rdev_free_array(remote_device_array);
    }

	LOGX("hrm_device_count=%d\n", hrm_device_count);

	if (hrm_device_count > 0) {

    	char preamble[] = "{\"hrm_devices\": [";
    	char quote[] = "\"";
    	char comma[] = ", ";
    	char left_sb[] = "[";
    	char right_sb[] = "]";
    	char postscript[] = "]}";

		char json_for_unity[(hrm_device_count+1)*(20 + sizeof(btle_bevice_info_t))];

		strcpy(json_for_unity, preamble);

    	int i;
    	for (i = 0; i < hrm_device_count; i++) {

        	strcat(json_for_unity, left_sb);
        	strcat(json_for_unity, quote);
        	strcat(json_for_unity, device_info[i].address);
        	strcat(json_for_unity, quote);
    		strcat(json_for_unity, comma);
        	strcat(json_for_unity, quote);
        	strcat(json_for_unity, device_info[i].name);
        	strcat(json_for_unity, quote);
        	strcat(json_for_unity, right_sb);
        	if (i+1 < hrm_device_count) {
        		strcat(json_for_unity, comma);
        	}
    	}
    	strcat(json_for_unity, postscript);

    	LOGX("JSON=%s\n", json_for_unity);

    	UnitySendMessage("BtHrmPlugin", "RequestScanForHrmDevicesSucceeded", json_for_unity);

    	LOGX("Sent to Unity\n");
    } else {
		char json_for_unity[1000];
    	sprintf(json_for_unity, "{\"error_id\": %d, \"error_text\": \"%s\"}", 1, "No HRM devices found");
    	UnitySendMessage("BtHrmPlugin", "RequestScanForHrmDevicesFailed", json_for_unity);
    }
	return EOK;
}

/*
 * Internal (static) functions only after this point
 */

static void bt_event(const int event, const char *bt_addr, const char *event_data)
{
	const char *name = bt_event_name(event);
	LOGX("Bluetooth event: %s\n", name);

	if (bt_addr) {
		LOGUX("Bluetooth address: %s\n", bt_addr);
	}

	if (event_data) {
		LOGUX("Bluetooth event data: %s\n", event_data);
	}
}

static const char *bt_event_name(const int id) {
	const event_names_t descriptions[] = {
			{ BT_EVT_ACCESS_CHANGED, "BT_EVT_ACCESS_CHANGED" },
			{ BT_EVT_RADIO_SHUTDOWN, "BT_EVT_ACCESS_CHANGED" },
			{ BT_EVT_RADIO_INIT, "BT_EVT_ACCESS_CHANGED" },
			{ BT_EVT_CONFIRM_NUMERIC_REQUEST, "BT_EVT_ACCESS_CHANGED" },
			{ BT_EVT_PAIRING_COMPLETE, "BT_EVT_ACCESS_CHANGED" },
			{ BT_EVT_DEVICE_ADDED, "BT_EVT_ACCESS_CHANGED" },
			{ BT_EVT_DEVICE_DELETED, "BT_EVT_ACCESS_CHANGED" },
			{ BT_EVT_SERVICE_CONNECTED, "BT_EVT_ACCESS_CHANGED" },
			{ BT_EVT_SERVICE_DISCONNECTED, "BT_EVT_ACCESS_CHANGED" },
			{ BT_EVT_LE_DEVICE_CONNECTED, "BT_EVT_ACCESS_CHANGED" },
			{ BT_EVT_LE_DEVICE_DISCONNECTED, "BT_EVT_ACCESS_CHANGED" },
			{ BT_EVT_LE_NAME_UPDATED, "BT_EVT_ACCESS_CHANGED" },
			{ BT_EVT_LE_GATT_SERVICES_UPDATED, "BT_EVT_ACCESS_CHANGED" },
			{ BT_EVT_FAULT, "BT_EVT_ACCESS_CHANGED" },
			{ BT_EVT_UNDEFINED_EVENT, "BT_EVT_ACCESS_CHANGED" },
			{ -1, NULL }
	};

	int i;
	for (i = 0; descriptions[i].id != -1; i++) {
		if (descriptions[i].id == id) {
			return descriptions[i].name;
		}
	}

	return "Unknown Event";
}

static void gatt_service_connected(const char *bt_addr, const char *service, int instance, int err, uint16_t conn_int, uint16_t latency, uint16_t super_timeout, void *user_data)
{
	LOGX("Bluetooth service connected: %s on %s instance %d\n", bt_addr, service, instance);

	g_hrm_service_initialised = true;

	LOGX("YYYY registering for notifications\n");

	errno = 0;
	int rc = bt_gatt_reg_notifications(instance, notifications_cb);

	LOGX("rc from bt_gatt_reg_notifications=%d\n", rc);

	if (rc != 0) {
		LOGUX("bt_gatt_reg_notifications errno=%s\n", strerror(errno));
		return;
	} else {
		LOGX("bt_gatt_reg_notifications was presumably OK\n");
	}

	LOGX("Allocated memory for notifications\n");

	int num_characteristics = bt_gatt_characteristics_count(instance);

	LOGX("# characteristics=%d\n", num_characteristics);

	if (num_characteristics > -1) {
		bt_gatt_characteristic_t *characteristic_list;

		characteristic_list = (bt_gatt_characteristic_t*) malloc(num_characteristics * sizeof(bt_gatt_characteristic_t));

		if (NULL == characteristic_list) {
			LOGUX("GATT characteristics: Not enough memory\n");
			bt_gatt_disconnect_instance(instance);
			return;
		}

		/* BEGIN WORKAROUND - Temporary fix to address race condition */
		int number = 0;
		do {
			number = bt_gatt_characteristics(instance, characteristic_list, num_characteristics);
		} while ((number == -1) && (errno== EBUSY));

		int characteristic_list_size = number;

		int i;
		for (i = 0; i < characteristic_list_size; i++) {

			LOGX("characteristic: uuid,handle,value_handle: %s, %d, %d\n", characteristic_list[i].uuid,
					characteristic_list[i].handle, characteristic_list[i].value_handle);

			if (strcmpi(characteristic_list[i].uuid, HEART_RATE_MEASUREMENT) == 0) {
				LOGX("heart rate characteristic available\n");

				g_handle = characteristic_list[i].handle;
				g_value_handle = characteristic_list[i].value_handle;

				// enable=1 switches the notification on for the specified characteristic

				LOGX("registering for heart_rate_measurement notification. uuid,handle,value_handle=%s, %d, %d\n",
						characteristic_list[i].uuid, characteristic_list[i].handle, characteristic_list[i].value_handle);
				errno= 0;
				rc = bt_gatt_enable_notify(instance, &characteristic_list[i], 1);
				if (rc != 0) {
					LOGUX("bt_gatt_enable_notify errno=%s\n", strerror(errno));
				} else {
					LOGX("bt_gatt_enable_notify was presumably OK\n");
					LOGX("updating client characteristic config to switch on notifications on the HRM\n");
					g_hrm_data = 1;
					if (bt_gatt_write_value_noresp(instance, characteristic_list[i].handle, 0, &g_hrm_data, sizeof(g_hrm_data)) == EOK) {
						LOGX("notifications config written successfully\n");
					} else {
						LOGUX("notifications config write error - %s\n", strerror(errno));
					}
				}
				LOGX("rc from registering for heart_rate_measurement notification=%d\n", rc);

			} else {
				LOGX("other characteristic: uuid,handle,value_handle=%s, %d, %d\n", characteristic_list[i].uuid,
						characteristic_list[i].handle, characteristic_list[i].value_handle);
			}
		}

		if (characteristic_list != NULL) {
			free(characteristic_list);
			characteristic_list = NULL;
		}

		/* END WORKAROUND */

		LOGX("done registering for heart_rate_measurement notifications\n");

	}
}

static void gatt_service_disconnected(const char *bt_addr, const char *service, int instance, int reason, void *user_data)
{
	LOGX("Bluetooth service disconnected: %s on %s instance %d\n", bt_addr, service, instance);
	g_hrm_service_initialised = false;

	char json_for_unity[1000];
	sprintf(json_for_unity, "{\"error_id\": %d, \"error_text\": \"Device %s, instance %d, disconnected - reason %d\"}", 2, bt_addr, instance, reason);
	UnitySendMessage("BtHrmPlugin", "RequestServiceDisconnected", json_for_unity);

	return;
}

static void gatt_service_updated(const char *bt_addr, int instance, uint16_t conn_int, uint16_t latency, uint16_t super_timeout, void *user_data)
{
	LOGX("Bluetooth service disconnected: %s instance %d\n", bt_addr, instance);
	return;
}

static void notifications_cb(int instance, uint16_t handle, const uint8_t *val, uint16_t len, void *user_data)
{
	LOGX("Bluetooth service notification call-back received\n");

	if (!g_hrm_service_initialised) {
		LOGX("Bluetooth service not initialised so returning\n");
		return;
	}

	LOGX("Value length=%d\n", len);

	// parse value, paying attention to the bit settings in the FLAGS field

	bool hr_data_format_8 = false;
	bool sensor_contact_detected = false;
	bool sensor_contact_feature_supported = false;
	bool energy_expended_feature_supported = false;
	bool rr_interval_data_present = false;
	uint8_t hr_measurement = 0;
	uint16_t energy_expended = 0;
	uint8_t num_rr_intervals = 0;
	uint16_t* rr_intervals;

	int index = 0;

	uint8_t flags = val[index++];
	LOGX("FLAGS=%d\n", flags);

	hr_data_format_8 = ((flags & HEART_RATE_VALUE_FORMAT) != HEART_RATE_VALUE_FORMAT);
	sensor_contact_detected = ((flags & SENSOR_CONTACT_DETECTED) == SENSOR_CONTACT_DETECTED);
	sensor_contact_feature_supported = ((flags & SENSOR_CONTACT_FEATURE) == SENSOR_CONTACT_FEATURE);
	energy_expended_feature_supported = ((flags * ENERGY_EXPENDED_FEATURE) == ENERGY_EXPENDED_FEATURE);
	rr_interval_data_present = ((flags & RR_INTERVAL_DATA) == RR_INTERVAL_DATA);

	if (!hr_data_format_8) {
		LOGX("16 bit heart rate measurement data encountered - we only support 8 bit values. Please unstrap your HR monitor from that frightened gerbil immediately!\n");
		return;
	}

	hr_measurement = val[index++];

	if (energy_expended_feature_supported) {
		energy_expended = val[index];
		index = index + 2;
	}

	if (rr_interval_data_present) {
		num_rr_intervals = (len - index) / sizeof(uint16_t);
		int num_bytes = num_rr_intervals * sizeof(uint16_t);
		rr_intervals = (uint16_t*) malloc(num_rr_intervals * sizeof(uint16_t*));
		memcpy(rr_intervals, (&val) + index, num_bytes);
	}

	LOGX("HR Rate=%d\n", hr_measurement);

	/*

	 {
	 	 "hr_data_format_8": 1,
	 	 "sensor_contact_feature_supported": 1,
	 	 "energy_expended_feature_supported": 1,
	 	 "rr_interval_data_present": 1,
	 	 "heart_rate": 124,
	 	 "sensor_contact_detected": 1,		// optional
	 	 "energy_expended": 123,			// optional
	 	 "rr_intervals": [ 1, 2, 45, 12, 1] // optional - num_rr_intervals of uint16_t
	 }

	 */

	char hr_data_format_8_label[] = "hr_data_format_8";
	char sensor_contact_feature_supported_label[] = "sensor_contact_feature_supported";
	char energy_expended_feature_supported_label[] = "energy_expended_feature_supported";
	char rr_interval_data_present_label[] = "rr_interval_data_present";
	char heart_rate_label[] = "heart_rate";
	char sensor_contact_detected_label[] = "sensor_contact_detected";
	char energy_expended_label[] = "energy_expended";
	char num_rr_intervals_label[] = "num_rr_intervals";
	char rr_intervals_label[] = "rr_intervals";

	int overhead = 20;
	int padding = 1000;
	int num_attributes = 9;

	int json_max_size_estimate = num_attributes * (sizeof(energy_expended_feature_supported_label) + overhead) + num_rr_intervals*overhead + padding;

	char json_for_unity[json_max_size_estimate];

	sprintf(json_for_unity, "{ \"%s\": %d, \"%s\": %d, \"%s\": %d, \"%s\": %d, \"%s\": %d",
			hr_data_format_8_label, hr_data_format_8,
			sensor_contact_feature_supported_label, sensor_contact_feature_supported,
			energy_expended_feature_supported_label, energy_expended_feature_supported,
			rr_interval_data_present_label, rr_interval_data_present,
			heart_rate_label, hr_measurement
			);

	if (sensor_contact_feature_supported) {
		char temp[padding];
		sprintf(temp, ", \"%s\": %d", sensor_contact_detected_label, sensor_contact_detected);
		strcat(json_for_unity, temp);
	}

	if (energy_expended_feature_supported) {
		char temp[padding];
		sprintf(temp, ", \"%s\": %d", energy_expended_label, energy_expended);
		strcat(json_for_unity, temp);
	}

	if (rr_interval_data_present) {
		char temp[num_rr_intervals*overhead + padding];
		sprintf(temp, ", \"%s\": %d, \"%s\": [", num_rr_intervals_label, num_rr_intervals, rr_intervals_label);
		strcat(json_for_unity, temp);

		int i;
		for (i=0; i<num_rr_intervals; i++) {

			if ((i+1) < num_rr_intervals) {
				sprintf(temp, "%d,", (uint16_t)(*(rr_intervals + i*sizeof(uint16_t))));
			} else {
				sprintf(temp, "%d",  (uint16_t)(*(rr_intervals + i*sizeof(uint16_t))));
			}
			strcat(json_for_unity, temp);
		}
		strcat(json_for_unity, "]");
	}

	strcat(json_for_unity, "}");

	LOGX("JSON=%s\n", json_for_unity);

	UnitySendMessage("BtHrmPlugin", "BtHrmNotification", json_for_unity);

	LOGX("Sent to Unity\n");

	if (rr_intervals) {
		num_rr_intervals = 0;
		free(rr_intervals);
		rr_intervals = 0;
	}
}

