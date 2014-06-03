/**
 * Copyright 2014 BlackBerry Limited.
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

using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

/**
 * For more information on invoking core applications:
 * https://developer.blackberry.com/native/documentation/cascades/device_platform/invocation/invoking_core_apps.html
 * */

public class BlackBerryInvoke : MonoBehaviour {
	#if UNITY_IPHONE || UNITY_XBOX360
	
	// On iOS and Xbox 360 plugins are statically linked into
	// the executable, so we have to use __Internal as the
	// library name.
	[DllImport ("__Internal")]
	
	#else
	
	// Other platforms load plugins dynamically, so pass the name
	// of the plugin's dynamic library.
	[DllImport("BlackBerryInvoke")]
	
	#endif
	
	public static extern void invoke (
		string action,
		string data,
		string file_transfer_mode,
		string id,
		int list_id,
		string metadata,
		string source,
		string target,
		int target_type_mask,
		string type,
		string uri
	);
}
