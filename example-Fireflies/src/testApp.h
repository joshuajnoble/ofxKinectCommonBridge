#pragma once

#include "ofMain.h"
#include "ofxKinect4Windows.h"
#include "ofxGui.h"

class testApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		ofxKinect4Windows kinect;
		ofShader shader;
		ofPlanePrimitive plane;

		ofVec3f head, lHand, rHand;
		bool hasSkeleton;

		float jointDistance;
		ofxPanel gui;

		ofxFloatSlider p1, p2, p3, p4, p5, p6;

};
