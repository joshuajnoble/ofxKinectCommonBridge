ofxKinectCommonBridge
=================

A wrapper for the simple Kinect for Windows library developed at Microsoft in partnership with myself &amp; James George

Starting the library is easy:

````
kinect.start();
````

This creates a 640x480 color image stream and 320x240 depth image stream. A more complex example looks like so:

````
kinect.initSensor(1); // open the 2nd sensor
//kinect.initIRStream(640, 480); // size, you can do IR or Color
kinect.initColorStream(640, 480); // size
kinect.initDepthStream(320, 240, true); // size and close range?
kinect.initSkeletonStream(true); // seated?
````

To update the Kinect, you guessed it:

````
kinect.update();
````

This handles updating all the streams you have initialized.

To draw, choose one or all of:

````
kinect.draw(0,0); // draw the color image
kinect.drawDepth(kinect.getColorPixelsRef().getWidth(), 0); // draw the depth image
````

To access the pixels of any image:

````
ofPixels colorPix = getColorPixelsRef();
ofPixels depthPix = getDepthPixelsRef();///< grayscale values
ofShortPixels rawDepthPix = getRawDepthPixelsRef();	///< raw 11 bit values
````

To get the screen locations of the skeletons and joints of those skeletons

````
if(kinect.isNewSkeleton()) {
		for( int i = 0; i < kinect.getSkeletons().size(); i++) 
		{
      			// has a head? probably working ok then :)
			if(kinect.getSkeletons().at(i).find(NUI_SKELETON_POSITION_HEAD) != kinect.getSkeletons().at(i).end())
			{
				// just get the first one
				SkeletonBone headBone = kinect.getSkeletons().at(i).find(NUI_SKELETON_POSITION_HEAD)->second;
				ofVec2f headScrenPosition( headBone.getScreenPosition().x, headBone.getScreenPosition().y);
				return;
			}
		}
	}

````

Gestures, Face-Tracking, and the Voice API are all on their way.

As of yet 32bit only and, as this is leveraging the Kinect for Windows library, Windows only.

Download the Kinect For Windows SDK at http://www.microsoft.com/en-us/kinectforwindows/ and have fun



#projectGenerator Help

If you are using the project generator to create your ofxKinectCommonBridge projects, you may notice that your projects don't compile and throw errors about "Cannot open include file "NuiApi.h", etc. This is because there are a few additional settings that you must add to the Visual Studio project after generating that the projectGenerator currently can't add for you.

So, once you have generated a project that includes ofxKinectCommonBridge:

- Right click on the name of your solution in the Solution Explorer in Visual studio and select "Properties" at the bottom.
- Ensure that "All Configurations" is selected in the Configuration drop down on the top left

- Under **VC++ Directories > All Options**
  - Add the following to *Include Directories*
  
    ````
    $(KINECTSDK10_DIR)inc
    $(KINECT_TOOLKIT_DIR)inc
    ````
        
  - Add the following to *Library Directories*
  
    ````
    $(KINECTSDK10_DIR)\lib\x86
    ````
    
- Under **Linker > All Options**
  - add the following to *Additional Library Directories*
    
        ````
        ..\..\..\addons\ofxKinectCommonBridge\libs\KinectCommonBridge\lib\windows
        ````
    
  - add the following under *Additional Dependencies*
  
	    ````
        KinectCommonBridge.lib
        Kinect10.lib
	    ````

- Under **Build Events > Post Build Event**
  - add the following to *Command Line*

	````
    xcopy /e /i /y "$(ProjectDir)..\..\..\export\vs\*.dll" "$(ProjectDir)bin"
    xcopy /e /i /y "..\..\..\addons\ofxKinectCommonBridge\libs\KinectCommonBridge\lib\windows\*.dll" "$(ProjectDir)bin"
    ````
    
May require some modification for your specific machine...

# Using Property Sheets to do all these settings automatically

If you have used the Project Generator to create your project, you can use this route to add all the necessary settings for your project (e.g. include paths, libraries, build steps for copying necessary dll's).

- Open the *Property Manager* for your *Solution* (**View > Other Windows > Property Manager**)
  - Right click on your project (e.g. `emptyExample`) and select **Add Existing Property Sheet...**
  - Select the file `addons\ofxKinectCommonBridge\ofxKinectCommonBridge.props`