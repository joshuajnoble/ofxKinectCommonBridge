#pragma once

// tig: uncomment the following line to use programmable GL , and GLSL 150
// otherwise this example will run using OpenGL 2.0 / GLSL 1.20

// #define USE_PROGRAMMABLE_GL

// note that if you use programmable GL, a different set of shaders will be loaded.
// see testApp.cpp


#include "ofMain.h"
#include "ofxKinect4Windows.h"
#include "ofxGui.h"

class testApp : public ofBaseApp{
	
	ofVboMesh	mVboBox;
	ofEasyCam	mCamMain;
	ofShader	depthDisplace;
	
	bool isShaderDirty;
	
public:
	void setup();
	void update();
	void draw();
	void exit();
	
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	void testApp::addFace(ofMesh& mesh, ofVec3f a, ofVec3f b, ofVec3f c);
	void addFace(ofMesh& mesh, ofVec3f a, ofVec3f b, ofVec3f c, ofVec3f d);
	
	ofxKinect4Windows kinect;
	ofImage raysTexture;
	ofShader rayShader;
	ofFbo fbo;
	ofPlanePrimitive plane;
	ofxPanel gui;

	ofxFloatSlider exposureSlider;
	ofxFloatSlider densitySlider;
	ofxFloatSlider weightSlider;
	ofxFloatSlider decaySlider;
	
};
