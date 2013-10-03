#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	kinect.init( 0 );
	
	kinect.startColorStream(640, 480);
	kinect.startDepthStream(320, 240, true);
	kinect.startSkeletonStream(true);

	kinect.start();
}

//--------------------------------------------------------------
void testApp::update(){
	kinect.update();
}

//--------------------------------------------------------------
void testApp::draw(){
	ofDisableAlphaBlending(); //Kinect alpha channel is default 0;
	//kinect.draw(0,0);
	//kinect.drawDepth(0,0);

	kinect.drawIR(0, 0, 640, 480);

	vector<Skeleton> skel = kinect.getSkeletons();
	for( int i = 0; i < skel.size(); i++) 
	{
		// 
		for( map<int, SkeletonBone>::iterator skelIt = skel.at(i).begin(); skelIt != skel.at(i).end(); ++skelIt )
		{
			//cout << skelIt->second.getScreenPosition ().x << " " << skelIt->second.getScreenPosition().y << endl;
			ofCircle( ofVec2f(skelIt->second.getScreenPosition().x, skelIt->second.getScreenPosition().y), 10);
		}
	}
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