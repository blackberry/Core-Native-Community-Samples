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

public class BlackBerryInvokeTest : MonoBehaviour {

	Vector2 scrollPosition;

	// Use this for initialization
	void Start () {
		scrollPosition = Vector2.zero;
	}
	
	// Update is called once per frame
	void Update () {
		if (Input.touchCount > 0 && Input.GetTouch(0).phase == TouchPhase.Moved) {
			scrollPosition.y += Input.GetTouch(0).deltaPosition.y;
		}
	}

	void OnGUI () {
		float scrollViewX = 20.0f;
		float scrollViewY = 20.0f;
		float scrollViewW = Screen.width - 40.0f;
		float scrollViewH = Screen.height - 40.0f;

		GUI.skin.button.fontSize = 20;
		GUI.skin.label.fontSize = 20;
		GUI.skin.label.alignment = TextAnchor.MiddleCenter;

		scrollPosition = GUI.BeginScrollView(
			new Rect(scrollViewX, scrollViewY, scrollViewW, scrollViewH),
			scrollPosition,
			new Rect(0.0f, 0.0f, scrollViewW, 800.0f)
		);

		float itemY = 0;
		float itemH = 100.0f;

		GUI.Label(
			new Rect(scrollViewW * 0.3f, itemY, scrollViewW * 0.7f, itemH),
			"Invoke the BlackBerry Messenger application with default Share text: Hello World!"
		);
		if (GUI.Button(new Rect(0, itemY, scrollViewW * 0.3f, itemH), "BBM (Share Text)")) {
			BlackBerryInvoke.invoke("bb.action.SHARE", "Hello World!", null, null, 0, null, null, "sys.bbgroups.sharehandler", 0, "text/plain", null);
		}

		itemY += itemH;
		GUI.Label(
			new Rect(scrollViewW * 0.3f, itemY, scrollViewW * 0.7f, itemH),
			"Invoke the Facebook application with default Share text: Hello World!"
		);
		if (GUI.Button(new Rect(0, itemY, scrollViewW * 0.3f, itemH), "Facebook (Share Text)")) {
			BlackBerryInvoke.invoke("bb.action.SHARE", "Hello World!", null, null, 0, null, null, "Facebook", 0, "text/plain", null);
		}

		itemY += itemH;
		GUI.Label(
			new Rect(scrollViewW * 0.3f, itemY, scrollViewW * 0.7f, itemH),
			"Invoke the Twitter application with default Share text: Hello World!"
		);
		if (GUI.Button(new Rect(0, itemY, scrollViewW * 0.3f, itemH), "Twitter (Share Text)")) {
			BlackBerryInvoke.invoke("bb.action.SHARE", "Hello World!", null, null, 0, null, null, "Twitter", 0, "text/plain", null);
		}

		itemY += itemH;
		GUI.Label(
			new Rect(scrollViewW * 0.3f, itemY, scrollViewW * 0.7f, itemH),
			"Invoke the Email application to compose a message."
		);
		if (GUI.Button(new Rect(0, itemY, scrollViewW * 0.3f, itemH), "Email (Compose)")) {
			string uri = "";
			uri += "mailto:eoros@blackberry.com";
			uri += "?subject=Support Request";
			uri += "&body=This is an example of how you might use Email invocation to let your users submit support tickets.";

			BlackBerryInvoke.invoke("bb.action.OPEN, bb.action.SENDEMAIL", null, null, null, 0, null, null, "sys.pim.uib.email.hybridcomposer", 0, null, uri);
		}

		itemY += itemH;
		GUI.Label(
			new Rect(scrollViewW * 0.3f, itemY, scrollViewW * 0.7f, itemH),
			"Invoke the BlackBerry World application to specific content. In this case, the BlackBerry Community application."
		);
		if (GUI.Button(new Rect(0, itemY, scrollViewW * 0.3f, itemH), "BlackBerry World (App Listing)")) {
			BlackBerryInvoke.invoke("bb.action.OPEN", null, null, null, 0, null, null, "sys.appworld", 0, null, "appworld://content/46922891");
		}

		itemY += itemH;
		GUI.Label(
			new Rect(scrollViewW * 0.3f, itemY, scrollViewW * 0.7f, itemH),
			"Invoke the Browser application to a specific web page. In this case, the Invoking Core Applications documentation."
		);
		if (GUI.Button(new Rect(0, itemY, scrollViewW * 0.3f, itemH), "Browser (Open URL)")) {
			BlackBerryInvoke.invoke("bb.action.OPEN", null, null, null, 0, null, null, "sys.browser", 0, null, "https://developer.blackberry.com/native/documentation/cascades/device_platform/invocation/invoking_core_apps.html");
		}

		itemY += itemH;
		GUI.Label(
			new Rect(scrollViewW * 0.3f, itemY, scrollViewW * 0.7f, itemH),
			"Invoke the Settings application to a specific screen. In this case, Application Permissions."
		);
		if (GUI.Button(new Rect(0, itemY, scrollViewW * 0.3f, itemH), "Settings (Application Permissions)")) {
			BlackBerryInvoke.invoke("bb.action.OPEN", null, null, null, 0, null, null, "sys.settings.card", 0, "settings/view", "settings://permissions");
		}

		itemY += itemH;
		GUI.Label(
			new Rect(scrollViewW * 0.3f, itemY, scrollViewW * 0.7f, itemH),
			"Invoke the Miracast card to find devices your application can be shown on."
		);
		if (GUI.Button(new Rect(0, itemY, scrollViewW * 0.3f, itemH), "Mircast (Show On)")) {
			BlackBerryInvoke.invoke("bb.action.VIEW", null, null, null, 0, null, null, "sys.miracastviewer", 0, "application/vnd.rim.miracast.showon", null);
		}

		GUI.EndScrollView();
	}
}
