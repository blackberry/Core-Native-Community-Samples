# Contents

* Background
	* Requirements
	* Author(s)
	* Contributing
* How To Build
	* Momentics (Optional)
	* Unity
* Additional Resources
* Disclaimer

# Background

This project provides both the code for the Shared Object (SO) Library as well as the Unity project files with sample usage of the invocation APIs.

## Requirements

* BlackBerry 10.2.1+
* Unity 4.3.3f1+
* Momentics IDE 2.0+ (Optional)

## Author(s)

* [Erik Oros](mailto:eoros@blackberry.com) | [Twitter](https://twitter.com/WaterlooErik)

## Contributing

To contribute code to this repository you must be [signed up as an official contributor](http://blackberry.github.com/howToContribute.html).

# How To Build

## Momentics (Optional)

These steps are not required to use the Invocation API within Unity as the SO Library is provided within the **Unity** folder. However, these instructions
are being provided to provide some background on the process of creating a Unity-BlackBerry extension.

1.	Clone/download this repository to your PC.

2.	Download and install the latest Momentics IDE from the following URL:
	https://developer.blackberry.com/native/downloads/

3.	Launch Momentics.

4.	Navigate **File > New > BlackBerry Project**.

5.	Choose **Library > Shared**, then click **Next**.

6.	Enter a **Project Name** and select **Language: C** and **Build: Makefile**, then click **Next**.

7.	Select **API Level: 10.2**, then click **Finish**.

8.	From the **momentics** folder of this repository, copy **BBInvoke.c** to the **root** folder or your Momentics project.

9.	From the **momentics/public** folder of this repository, copy **BBInvoke.h** to the **public** folder of your Momentics project.

10.	In Momentics, In the **Project Explorer** panel, refresh the contents to see your new files.

11.	Right-click on your project in the **Project Explorer** panel and navigate **Build Configurations > Set Active > Device-Release**.

12.	Right-click on your project in the **Project Explorer** panel and select **Build Project**.

This should create the SO Library (i.e. **libBBInvoke.so**) under the **arm/so.le-v7-g** folder. This is the same file that is referenced in the Unity project.

## Unity

1.	Clone/download this repository to your PC.

2.	Launch Unity and create a new project called **BBInvoke**.

3.	Import the included **BBInvoke.unitypackage** asset into your new project by navigating:

	a.	**Assets > Import Package > Custom Package**; and selecting **BBInvoke.unitypackage** from the files you downloaded from this repository.
	
	b.	Note that the Asset files are also included, unpacked, for reference.

4.	Double-click the **BBInvoke** scene to open the scene.

This sample must be run on a BlackBerry 10 device. The default sample performs **Facebook** invocation and requires configuration of a Facebook account on
the device. The sample can be modified to perform other invocations as outlined here:
https://developer.blackberry.com/native/documentation/cascades/device_platform/invocation/invoking_core_apps.html

These modifications are made in **BBInvokeTest.cs** which is simply attached to the camera to execute an invocation on **Awake**.

For more information on configuring Unity and exporting to BlackBerry 10, please refer to the **Getting Started** guide here:
http://docs.unity3d.com/Documentation/Manual/bb10-gettingstarted.html

# Additional Resources

* [Invoking Core Applications](https://developer.blackberry.com/native/documentation/cascades/device_platform/invocation/invoking_core_apps.html)

# Disclaimer

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.