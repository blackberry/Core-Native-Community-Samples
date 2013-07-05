ToneGenerator is a sample core native application that demonstrates how to add low-latency audio generation and playback to your application.  This application generates a tone when you touch the screen.  A grid of pixels is mapped to various audio frequencies ranging from 0 to 8 kHhz.  There are also regions at the bottom of the screen that are toggle buttons for different waveform shapes.  The screen also displays a snapshot of the generated wave form sample data (approximately 8 ms worth of data for one channel in the smaller waveform trace and half that in the larger trace).  The tone plays as long as your finger is touching the screen and fades out after you lift your finger.  If you touch the screen before the initial tone fades out, the new tone will overlap the fadeout from the old tone.  If you have a good ear for music notes, you may be able to play simple tunes with the app over time by remembering where to press for specific notes.  

If you update or modify this example, please update the running version info as well.

Features in this version of the sample app:

- low latency tone generation from 0 Hz to 8 kHz (minimum latency measured is 60 ms)
- use of OpenGL to render 2D waveform traces on the screen
- multitouch support
- chords sound better after a bug fix was made to the tone blending code in the play thread
- rendering code was updated to adjust the rendering to support Q10 / Q5 screen resolutions (tone generation range is reduced as a result of the smaller screen height)

For more information about our other Open Source projects, visit:

BlackBerry Open Source microsite (http://blackberry.github.com)

To check the Samples Catalog, visit:
Samples Catalog (http://blackberry.github.com/samples)

For more information about Native development, visit:
The Native microsite (http://developer.blackberry.com/native)

