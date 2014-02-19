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

using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;

public class BtHrmGuiHandler : MonoBehaviour {
	
	private GUIContent[] comboBoxList;
	private ComboBox comboBoxControl = new ComboBox();
	private GUIStyle listStyle = new GUIStyle();
	private GUIStyle bpmStyle = new GUIStyle();
	private GUIStyle messageStyle = new GUIStyle();
	private GUIStyle statusStyle = new GUIStyle();
	private Hashtable deviceList;
	private int heartRate;
	private string deviceAddress;
	private string message = "";
	private string statusMessage = "";
    private bool waiting = false;
	private bool initialiseButtonActive = true;
	private bool scanButtonActive = false;
	private bool startButtonActive = false;
	private bool stopButtonActive = false;
	private bool terminateButtonActive = false;
	private bool exitButtonActive = true;
	private int numbHor = 3;
	private int sw;
	private int sh;
	private int hm;
	private int vm;
	private int bh;
	private int bw;
	
	// Use this for initialization
	private void Start()
	{
		sw = Screen.width;
		sh = Screen.height;
		hm = 10;
		vm = 20;
		bh = 100;
		bw = (sw-4*hm)/numbHor;

		bpmStyle.fontSize = 200;
		bpmStyle.normal.textColor = Color.white;
		messageStyle.fontSize = 30;
		messageStyle.normal.textColor = Color.white;
		statusStyle.fontSize = 30;
		statusStyle.normal.textColor = Color.white;
	
	    comboBoxList = new GUIContent[1];
	    comboBoxList[0] = new GUIContent("No Bluetooth devices scanned yet!");

	    listStyle.normal.textColor = Color.white; 
	    listStyle.hover.background = new Texture2D(2, 2);
	    listStyle.padding.bottom = 4;
		listStyle.fontSize = 29;
		
        // Register for events
		BtHrmPlugin.ScanForHrmDevicesSuccessfulEvent += ScanSuccessful;
		BtHrmPlugin.ScanForHrmDevicesFailedEvent += ScanFailed;
		BtHrmPlugin.BtHrmNotificationEvent += BtHrmNotification;
		BtHrmPlugin.BtHrmMessageEvent += BtHrmMessage;
		BtHrmPlugin.ServiceDisconnectedEvent += ServiceDisconnected;
	}

    void OnGUI()
    {
		DateTime time = DateTime.Now;

		GUIStyle buttonLabelStyle = new GUIStyle(GUI.skin.button);
		buttonLabelStyle.fontSize = 29;

		GUIContent bpmRateMessage = new GUIContent();
		bpmRateMessage.text = heartRate.ToString();
		
		GUI.enabled = initialiseButtonActive;
		if (GUI.Button(new Rect(hm, vm, bw, bh), "Enable Bluetooth", buttonLabelStyle))
        {
			int rc = BtHrmPlugin.InitialiseBtLe();
		    StringBuilder sb = new StringBuilder();
	        sb.AppendFormat("Initialise Bluetooth - rc={0}\n", rc);
			displayMessage(sb.ToString());
			if (rc == 0) {
				initialiseButtonActive = false;
				scanButtonActive = true;
				startButtonActive = false;
				stopButtonActive = false;
				terminateButtonActive = true;
				exitButtonActive = true;
				statusMessage = "Bluetooth Initialised";
			} else {
				statusMessage = "Bluetooth Initialisation Failed";
			}
        }

		GUI.enabled = scanButtonActive;
		if (GUI.Button(new Rect(hm, 2*vm+bh, bw, bh), "Search", buttonLabelStyle))
        {
			if (!waiting) {
				displayMessage("Starting scan for devices\n");
				waiting = true;
				int rc = BtHrmPlugin.ScanForHrmDevices();
			    StringBuilder sb = new StringBuilder();
		        sb.AppendFormat("Scan for HRM devices - rc={0}\n", rc);
				displayMessage(sb.ToString());

				initialiseButtonActive = false;
				scanButtonActive = false;
				startButtonActive = false;
				stopButtonActive = false;
				terminateButtonActive = true;
				exitButtonActive = true;

				statusMessage = "Searching for HRM devices";

			} else {
	            Debug.LogError("Scan for HRM devices already in progress");
			    StringBuilder sb = new StringBuilder();
		        sb.Append("Scan for HRM devices already in progress\n");
				displayMessage(sb.ToString());
				statusMessage = "Searching for HRM devices";
			}
        }
        
		GUI.enabled = startButtonActive;
		if (GUI.Button(new Rect(2*hm+bw, 2*vm+bh, bw, bh), "Start Monitoring", buttonLabelStyle))
        {
			int selectedIndex = comboBoxControl.GetSelectedItemIndex();
			if (selectedIndex > 0) {
				string name = comboBoxList[selectedIndex].text;
				foreach (string address in deviceList.Keys) {
					if (name.Equals(deviceList[address])) {
						deviceAddress = address;
					    StringBuilder sb = new StringBuilder();
				        sb.AppendFormat("Monitoring HRM device - address={0}\n", deviceAddress);
						displayMessage(sb.ToString());
						int rc = BtHrmPlugin.StartMonitoring(deviceAddress);
					    sb = new StringBuilder();
				        sb.AppendFormat("Monitoring HRM device - rc={0}\n", rc);
						displayMessage(sb.ToString());

						initialiseButtonActive = false;
						scanButtonActive = false;
						startButtonActive = false;
						stopButtonActive = true;
						terminateButtonActive = false;
						exitButtonActive = true;

						statusMessage = "Monitoring started";

						break;
					}
				}
			} else {
			    StringBuilder sb = new StringBuilder();
		        sb.AppendFormat("Invalid HRM device selected\n");
				displayMessage(sb.ToString());

				statusMessage = "Invalid HRM device selected";
			}
        }
        
		GUI.enabled = stopButtonActive;
		if (GUI.Button(new Rect(3*hm+2*bw, 2*vm+bh, bw, bh), "Stop Monitoring", buttonLabelStyle))
        {
		    StringBuilder sb = new StringBuilder();
	        sb.AppendFormat("Stop Monitoring HRM device - address={0}\n", deviceAddress);
			displayMessage(sb.ToString());
			int rc = BtHrmPlugin.StopMonitoring(deviceAddress);
		    sb = new StringBuilder();
	        sb.AppendFormat("Stop Monitoring HRM device - rc={0}\n", rc);
			displayMessage(sb.ToString());

			initialiseButtonActive = false;
			scanButtonActive = true;
			startButtonActive = true;
			stopButtonActive = false;
			terminateButtonActive = true;
			exitButtonActive = true;
			
			statusMessage = "Monitoring stopped";

		}

		GUI.enabled = terminateButtonActive;
		if (GUI.Button(new Rect(2*hm+bw, vm, bw, bh), "Disable Bluetooth", buttonLabelStyle))
        {
			int rc = BtHrmPlugin.TerminateBtLe();
		    StringBuilder sb = new StringBuilder();
	        sb.AppendFormat("Terminate Bluetooth - rc={0}\n", rc);
			displayMessage(sb.ToString());

			initialiseButtonActive = true;
			scanButtonActive = false;
			startButtonActive = false;
			stopButtonActive = false;
			terminateButtonActive = false;
			exitButtonActive = true;
			
			statusMessage = "Bluetooth disabled";
		}
        
		GUI.enabled = exitButtonActive;
		if (GUI.Button(new Rect(3*hm+2*bw, vm, bw, bh), "Exit", buttonLabelStyle))
        {
		    StringBuilder sb = new StringBuilder();
	        sb.Append("Exit\n");
			displayMessage(sb.ToString());
			Application.Quit();
        }
        
		GUI.enabled = true;
		GUI.Label(new Rect(2*hm+bw+50, 2*vm+2*bh+50, Screen.width - (2*hm+bw)/2, Screen.height), bpmRateMessage, bpmStyle);

		GUI.Label(new Rect(hm, 500, Screen.width-2*hm, 50), statusMessage, statusStyle);

		int selectedItemIndex = comboBoxControl.GetSelectedItemIndex();
	    selectedItemIndex = comboBoxControl.List(new Rect(hm, 570, Screen.width , 40),
			comboBoxList[selectedItemIndex].text, comboBoxList, buttonLabelStyle, buttonLabelStyle, listStyle );
        
		GUI.Label( new Rect(hm, 720, Screen.width - 2*hm, Screen.height), message, messageStyle);
	}
	
    void ScanSuccessful(BtHrmPlugin.ScanForHrmDevicesEventArgs args)
    {
		deviceList = args.ListOfDevices;

		comboBoxList = new GUIContent[deviceList.Count+1];
	    comboBoxList[0] = new GUIContent("Select a Bluetooth HRM device!");
		
        StringBuilder sb = new StringBuilder();
        sb.AppendFormat("Successfully Scanned!\n");
		
		int i = 1;
		foreach (string address in deviceList.Keys) {
			sb.AppendFormat("address:       {0}\n" +
				            "   name:       {1}\n", address, deviceList[address]);
		    comboBoxList[i++] = new GUIContent(deviceList[address].ToString());
		}
		displayMessage(sb.ToString());
		
		waiting = false;

		initialiseButtonActive = false;
		scanButtonActive = true;
		startButtonActive = true;
		stopButtonActive = false;
		terminateButtonActive = true;
		exitButtonActive = true;
		
		statusMessage = "HRM devices found - select one";
	}

    void ScanFailed(BtHrmPlugin.ErrorEventArgs args)
    {
        StringBuilder sb = new StringBuilder();
        sb.AppendFormat("Scan Failed: {0}\n", args.Error);
        sb.AppendFormat("errorText: {0}\n", args.ErrorText);
		displayMessage(sb.ToString());
		waiting = false;

		initialiseButtonActive = false;
		scanButtonActive = true;
		startButtonActive = false;
		stopButtonActive = false;
		terminateButtonActive = true;
		exitButtonActive = true;

		statusMessage = "HRM device search failed";
    }

	void ServiceDisconnected (BtHrmPlugin.ErrorEventArgs args)
	{
        StringBuilder sb = new StringBuilder();
        sb.AppendFormat("Service Disconnected: {0}\n", args.Error);
        sb.AppendFormat("errorText: {0}\n", args.ErrorText);
		displayMessage(sb.ToString());

		initialiseButtonActive = false;
		scanButtonActive = true;
		startButtonActive = true;
		stopButtonActive = false;
		terminateButtonActive = true;
		exitButtonActive = true;

		statusMessage = "HRM service disconnected";
	}

    void BtHrmNotification(BtHrmPlugin.BtHrmNotificationEventArgs args)
	{
        StringBuilder sb = new StringBuilder();
        sb.AppendFormat("Bluetooth Notification\n");
        sb.AppendFormat("HRM data=: {0}\n", args.HeartRate);
		displayMessage(sb.ToString());
		heartRate = args.HeartRate;
	}

	void BtHrmMessage (BtHrmPlugin.BtHrmMessageEventArgs args)
	{
        StringBuilder sb = new StringBuilder();
        sb.AppendFormat("P> {0}", args.message);
		displayMessage(sb.ToString());
	}

	void displayMessage(string messageToDisplay) {
		message = (messageToDisplay + message);
	}
}
