AudioLoopBackSampleToFile

Demonstrates how to record and playback audio simultaneously (Full Duplex) 
using the Audio Library APIs as part of the BlackBerry 10 Native SDK. This sample 
generates an output.wav file with capturing voice overlap of record and playback.

This sample is designed to easily plug into a VoIP application for encoding/decoding
the audio stream  

Feature Summary

Full duplex audio support
Interface to the microphone, receiver, loudspeaker and headset.
Hardware Audio Routing to select audio user interface (headset, speakerphone…)
Io-audio enables voice processing (AEC, NR, Gain Control) and routes audio to voice path
Volume control


Author(s)
Ashish Pradhan, Research In Motion


Requirements

BlackBerry Native SDK 10.0.09 beta
BlackBerry 10 Dev Alpha device running 10.0.09 OS


Steps to run application:

1) Importing a project into the Native SDK: From the the Sample apps page, download and extract the sample application.
   Launch the Native SDK.
   On the File menu, click Import.
   Expand General, and select Existing Projects into Workspace. Click Next.
   Browse to the location where you extracted the sample app, and click OK. The sample project should display in the the Projects section.
   Click Finish to import the project into your workspace.
2) Download and copy any .wav file into your “downloads” folder on your development alpha device
3) In the QNX Momentics IDE under run configurations add the below two application arguments
   -d /accounts/1000/shared/downloads/InputName.wav 
   -o /accounts/1000/shared/downloads/Outputname.wav
4) Build this sample 
	Note: If the sample does not build then 
	a. Create a new project copy the VoiceTest.c file, 
	b. dd libaries asound, audio_manager under project properties-> C/CC++ builds -> Settings 
	c. Make sure the shared_files & microphone permissions is enabled in your application bar-descriptor
5) Load TestProgram to your development alpha device and run this application
6) The recording of your input wave file will be played back to you and anything you record during this 
   playback will be saved to your “downloads” folder


To contribute code to this repository you must be signed up as an official contributor.

Contributing Changes
Please see the README of the Samples-for-Java repository for instructions on how to add new Samples or make modifications to existing Samples.


Bug Reporting and Feature Requests
If you find a bug in a Sample, or have an enhancement request, simply file an Issue for the Sample and send a message (via github messages) to the Sample Author(s) to let them know that you have filed an Issue.


Disclaimer
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.