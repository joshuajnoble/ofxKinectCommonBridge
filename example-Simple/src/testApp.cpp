#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){

	kinect.initSensor();
	kinect.initIRStream(640, 480);
	kinect.initColorStream(640, 480);
	//kinect.initDepthStream(320, 240, true);
	//kinect.initSkeletonStream(true);

	//simple start
	kinect.start();

}

//--------------------------------------------------------------
void testApp::update(){
	kinect.update();
}

//--------------------------------------------------------------
void testApp::draw(){

	ofDisableAlphaBlending(); //Kinect alpha channel is default 0;
	kinect.draw(0,0);
	kinect.drawDepth(kinect.getColorPixelsRef().getWidth(), 0);

}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}