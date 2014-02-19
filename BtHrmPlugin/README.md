# BtHrmPlugin

The purpose of this application is to demonstrate how to develop Unity 3D plugin for BlackBerry 10 that works with the standard Heart Rate Service ([overview available here](http://developer.bluetooth.org/TechnologyOverview/Pages/HRS.aspx)) as specified by the Bluetooth SIG enabling it to interwork with any Bluetooth Smart device that implements this profile.

The sample code for this application is Open Source under 
the [Apache 2.0 License](http://www.apache.org/licenses/LICENSE-2.0.html).

**Applies To**

* [Cascades for BlackBerry 10](https://bdsc.webapps.blackberry.com/cascades/)

**Author(s)** 

* [John Murray](https://github.com/jcmurray)

**Release History**

* **V1.0.0** - Initial release

**Known Issues**

* None.

**Dependencies**

1. BlackBerry 10 Device Software **10.2.0**
2. Unity 3D version 4.2 or greater development environment supporting BlackBerry 10.

Thanks must go to the authors of two very useful C# Unity community components: [SimpleJSON](http://wiki.unity3d.com/index.php/SimpleJSON) written by @Bunny83; and,
[ComboBox](http://wiki.unity3d.com/index.php?title=PopupList#C.23_-_ComboBox_-_Update) Created by Eric Haines and extended by Hyungseok Seo (see LICENSE file for details). 

**How to Build BtHrmPlugin**

Simply import the project into a workspace in your NDK. This will allow you to build the shared library plugin **libBtHrmPlugin.so**. 

A example Unity application (packaged as an importable UnityPackage) that uses this plugin can be found here **/Unity/HRM-Plugin-Example/Assets/BtHrmPlugin.unitypackage** in the project.

After building the plugin the shared library file **libBtHrmPlugin.so** should be copied to the path **/Assets/Plugins/BlackBerry/libBtHrmPlugin.so** in the Unity project.
 
To contribute code to this repository you must be [signed up as an 
official contributor](http://blackberry.github.com/howToContribute.html).

## Contributing Changes

Please see the [README](https://github.com/blackberry/Core-Native-Community-Samples/blob/master/README.md) of the Core-Native-Community-Samples repository for instructions on how to add new Samples or 
make modifications to existing Samples.

## Bug Reporting and Feature Requests

If you find a bug in a Sample, or have an enhancement request, simply file 
an [Issue](https://github.com/blackberry/Core-Native-Community-Samples/issues) for 
the Sample and send a message (via github messages) to the Sample Author(s) to let 
them know that you have filed an [Issue](https://github.com/blackberry/Core-Native-Community-Samples/issues).


## Disclaimer

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.