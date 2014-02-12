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

public class GamepadInput : MonoBehaviour {

	/* Counter variables for our for loops. */
	private static int n;
	private static int m;

	/* Our main camera, all UI elements are contained as children. */
	private static GameObject _activeUI;

	/* An array of connected gamepads. */
	private string[] connectedGamepads;
	/* The number of connected gamepads. */
	private int nConnectedGamepads;
	/* The most recent primary gamepad. */
	private string newConnectedGamepad;
	/* The current primary gamepad. */
	private string connectedGamepad;

	/**
	 * We'll use transforms and materials as a method of updating our UI elements
	 * to respond to gamepad input.
	 */
	private Transform[] transforms;
	private Material[][] materials;
	
	/**
	 * Use this for initialization.
	 */
	void Start () {
		/* Obtain our main camera. */
		_activeUI = GameObject.FindGameObjectWithTag("MainCamera");

		/* Initialize our variables. */
		connectedGamepads = Input.GetJoystickNames();
		nConnectedGamepads = connectedGamepads.Length;
		newConnectedGamepad = "";
		connectedGamepad = "";

		/* Grab all the transforms we'll need, now. transforms indices correspond to materials indices. */
		transforms = new Transform[] {
			_activeUI.transform.Find("joystickLeft/pad"),
			_activeUI.transform.Find("joystickRight/pad"),
			_activeUI.transform.Find("dpad/left"),
			_activeUI.transform.Find("dpad/right"),
			_activeUI.transform.Find("dpad/down"),
			_activeUI.transform.Find("dpad/up"),
			_activeUI.transform.Find("buttons/button0"),
			_activeUI.transform.Find("buttons/button1"),
			_activeUI.transform.Find("buttons/button2"),
			_activeUI.transform.Find("buttons/button3"),
			_activeUI.transform.Find("buttons/button4"),
			_activeUI.transform.Find("buttons/button5"),
			_activeUI.transform.Find("buttons/button6"),
			_activeUI.transform.Find("buttons/button7")
		};

		/* Grab all the materials we'll need, now. materials indices correspond to materials indices. */
		materials = new Material[][] {
			null,
			null,
			new Material[] {Resources.Load<Material>("_materials/dpadLeft"), Resources.Load<Material>("_materials/dpadLeftLit")},
			new Material[] {Resources.Load<Material>("_materials/dpadRight"), Resources.Load<Material>("_materials/dpadRightLit")},
			new Material[] {Resources.Load<Material>("_materials/dpadDown"), Resources.Load<Material>("_materials/dpadDownLit")},
			new Material[] {Resources.Load<Material>("_materials/dpadUp"), Resources.Load<Material>("_materials/dpadUpLit")},
			new Material[] {Resources.Load<Material>("_materials/buttonCircle"), Resources.Load<Material>("_materials/buttonCircleLit")},
			new Material[] {Resources.Load<Material>("_materials/buttonCircle"), Resources.Load<Material>("_materials/buttonCircleLit")},
			new Material[] {Resources.Load<Material>("_materials/buttonCircle"), Resources.Load<Material>("_materials/buttonCircleLit")},
			new Material[] {Resources.Load<Material>("_materials/buttonCircle"), Resources.Load<Material>("_materials/buttonCircleLit")},
			new Material[] {Resources.Load<Material>("_materials/buttonQuad"), Resources.Load<Material>("_materials/buttonQuadLit")},
			new Material[] {Resources.Load<Material>("_materials/buttonQuad"), Resources.Load<Material>("_materials/buttonQuadLit")},
			new Material[] {Resources.Load<Material>("_materials/buttonQuad"), Resources.Load<Material>("_materials/buttonQuadLit")},
			new Material[] {Resources.Load<Material>("_materials/buttonQuad"), Resources.Load<Material>("_materials/buttonQuadLit")}
		};

		/* Loop through the connected gamepads on startup to determine whether we have a valid match. */
		foreach (string gamepad in connectedGamepads) {
			if (gamepad.Contains("BlackBerry Gamepad") == true) {
				_activeUI.transform.Find("status").GetComponent<GUIText>().text = gamepad;
				newConnectedGamepad = gamepad;
				break;
			}
		}

		/* If we don't find a gamepad, let the end-user know. */
		connectedGamepad = newConnectedGamepad;
		if (connectedGamepad.Equals("") == true) {
			_activeUI.transform.Find("status").GetComponent<GUIText>().text = "No Gamepad Detected";
		}
	}
	
	/**
	 * Update is called once per frame.
	 */
	void Update () {
		/**
		 * Monitor for changes in connected gamepads. In our case, we will default to the first
		 * BlackBerry Gamepad that we find as the primary input device.
		 */

		/* Check if the number of connected gamepads has changed (this will indicate a connect/disconnect. */
		connectedGamepads = Input.GetJoystickNames();
		if (connectedGamepads.Length != nConnectedGamepads) {
			/* Reset our variables as we will be looping through the list now. */
			nConnectedGamepads = connectedGamepads.Length;
			newConnectedGamepad = "";

			/* Cycle through each connected gamepad. */
			foreach(string gamepad in connectedGamepads) {

				/* We're only interested in BlackBerry supported gamepads. */
				if (gamepad.Contains("BlackBerry Gamepad") == true) {
					_activeUI.transform.Find("status").GetComponent<GUIText>().text = gamepad;
					newConnectedGamepad = gamepad;
					break;
				}
			}

			/* If we could not find a connected gamepad. */
			connectedGamepad = newConnectedGamepad;
			if (connectedGamepad.Equals("") == true) {
				_activeUI.transform.Find("status").GetComponent<GUIText>().text = "No Gamepad Detected";
			}
		}

		/* Handle left and right joystick inputs. */
		transforms[0].localPosition = new Vector3(Input.GetAxisRaw("X axis") * 0.3f, Input.GetAxisRaw("Y axis") * 0.3f, -0.5f);
		transforms[1].localPosition = new Vector3(Input.GetAxisRaw("4th axis") * 0.3f, Input.GetAxisRaw("5th axis") * 0.3f, -0.5f);

		/* Handle D-Pad inputs; left/right. */
		if (Input.GetAxisRaw("6th axis") < -0.1f) {
			transforms[2].gameObject.renderer.material = materials[2][1];
			transforms[3].gameObject.renderer.material = materials[3][0];
		} else if (Input.GetAxisRaw("6th axis") > 0.1f) {
			transforms[3].gameObject.renderer.material = materials[3][1];
			transforms[2].gameObject.renderer.material = materials[2][0];
		} else {
			transforms[2].gameObject.renderer.material = materials[2][0];
			transforms[3].gameObject.renderer.material = materials[3][0];
		}

		/* Handle D-Pad inputs; up/down. */
		if (Input.GetAxisRaw("7th axis") < -0.1f) {
			transforms[4].gameObject.renderer.material = materials[4][1];
			transforms[5].gameObject.renderer.material = materials[5][0];
		} else if (Input.GetAxisRaw("7th axis") > 0.1f) {
			transforms[5].gameObject.renderer.material = materials[5][1];
			transforms[4].gameObject.renderer.material = materials[4][0];
		} else {
			transforms[4].gameObject.renderer.material = materials[4][0];
			transforms[5].gameObject.renderer.material = materials[5][0];
		}

		/**
		 * Handle all buttons; based on the button's corresponding axis, we either toggle
		 * the button to be highlighted or not.
		 * 
		 * Note that the axes were configured in the Unity Input Manager to follow the pattern:
		 * Button 0, Button 1, Button 2, etc.
		 * 
		 * This allows us to easily loop through and poll the current interactions.
		 * 
		 * Alternative to the Input Manager, we can directly access Joystick Buttons with:
		 * Input.GetKey and Input.GetKeyDown
		 * 
		 * For more information on the direct approach, please see:
		 * http://devblog.blackberry.com/2013/11/up-up-down-down-left-right-left-right-b-a-gamepad-offer-update/
		 */
		for (n = 6, m = transforms.Length; n < m; ++n) {
			if (Input.GetAxisRaw("Button " + (n-6)) > 0.1f) {
				transforms[n].gameObject.renderer.material = materials[n][1];
			} else {
				transforms[n].gameObject.renderer.material = materials[n][0];
			}
		}
	}
}
