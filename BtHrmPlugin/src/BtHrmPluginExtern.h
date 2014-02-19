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

#ifndef BtHrmTPluginExtern_H_
#define BtHrmTPluginExtern_H_

#include "BtHrmPluginTypes.h"

/*
 * Entry points visible externally
 */

#ifdef __cplusplus
extern "C" {
#endif
    int initialiseBtLeImpl();
    int terminateBtLeImpl();
    int scanForHrmDevicesImpl();
    int startMonitoringImpl(const char *address);
    int stopMonitoringImpl(const char *address);
#ifdef __cplusplus
}
#endif

void UnitySendMessage(const char *obj, const char *method, const char *msg);

#endif /* BtHrmTPluginExtern_H_ */
