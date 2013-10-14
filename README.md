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
