#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){

	kinect.initSensor(0);
	kinect.initColorStream(640, 480);
	kinect.initDepthStream(320, 240);
	kinect.initFaceTracking();

	kinect.start();
}

void testApp::exit()
{
	kinect.stop();
}

//--------------------------------------------------------------
void testApp::update(){
	kinect.update();
	if(kinect.isFaceNew()) {
		face = kinect.getFaceData();
	}
}

//--------------------------------------------------------------
void testApp::draw()
{

	ofBackground(100, 100, 100);
	ofSetColor(0, 255, 0 );

	ofNoFill();
	ofRect( face.rect ); // draw the bounding box of the face

	ofSetColor(255, 255, 255 );
	face.mesh.drawWireframe(); // draw the mesh of the face
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