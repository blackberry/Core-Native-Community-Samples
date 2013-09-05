**SpaceGame**

========================================================================

SpaceGame is a space shooting game created as part of the blog tutorial on creating a complete game for BlackBerry 10 step by step.


**Author:**
[Pratik Sapra](https://github.com/pratiksapra)

**Feature summary:**
 - Game development basics 
 - Sprite Animation & actions
 - Particle effects
 - Handling Accelerometer & touch input
 - Collision Detection
 - Implementing AI 
 - Playing background music & sounds effects

**How to Play:**

Control - Tilt device to control the spaceship

Shoot - Tap anywhere on the screen to shoot

Mines - When enemy ship is chasing you, tap anywhere on the screen to deploy mines


========================================================================
**Requirements:**

 - BlackBerry® 10 Native SDK v10.0.9 or above
 - BlackBerry® 10 device v10.0.9 or above
 - Cocos2d-x (latest stable release)

========================================================================

**How to build SpaceGame:**

1. Download the latest stable version of [Cocos2d-x](http://www.cocos2d-x.org/projects/cocos2d-x/wiki/Download#Latest-stable-version)
2. Unzip Cocos2d-x to a desired directory
3. Download the SpaceGame project, unzip it
4. Copy the SpaceGame directory to the Cocos2d-x samples directory.  For example if you downloaded Cocos2d-x-2.1.4, the directory structure after copying the sample should look like below:
   cocos2d-x-2.1.4\samples\SpaceGame where SpaceGame should contain three directories - proj.blackberry, Classes, Resources. 
5. Open BlackBerry Native SDK, select File>Import
6.	In the Import dialog, select General>Existing Projects into Workspace and click next
5.	Browse to the directory you unzipped Cocos2d-x to in Step 2 and select it as the root directory
6.	In the projects selection, click deselect all.  Select only the BlackBerry projects (proj.blackberry) Box2D, chipmunk, cocos2dx, CocosDenshion, extensions and SpaceGame.  **Make sure "Copy projects into workspace" is unselected**
7. Once the projects are imported, select the SpaceGame project in the project explorer and from the Project menu select build.  This will build SpaceGame and all it’s dependencies including cocos2d-x.  So this step will take some time. 
8. Click on Run to deploy the game
