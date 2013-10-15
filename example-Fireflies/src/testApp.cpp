#include "testApp.h"

#define USE_PROGRAMMABLE_RENDERER

void testApp::setup(){

	kinect.initSensor( 0 );
	
	kinect.initColorStream(640, 480);
	kinect.initDepthStream(320, 240, true);
	kinect.initSkeletonStream(true);

	kinect.start();

#ifdef USE_PROGRAMMABLE_RENDERER
	shader.load("shaders/blobs.vs", "shaders/blobs.fs");
#else
	shader.load("shaders/blobs_gl2.vs", "shaders/blobs_gl2.fs");
#endif

	GLint err = glGetError();
	if (err != GL_NO_ERROR){
		ofLogNotice() << "Load Shader came back with GL error:	" << err;
	}

	plane.set(1024, 768, 4, 4);
	plane.setPosition(0, 0, 0);
	plane.mapTexCoords(0, 0, 1024, 768);
	
	hasSkeleton = false;

	gui.setup();
	gui.add(p1.setup("head", 0.0, -1.0, 1.0));
	gui.add(p2.setup("leftHand", 0.0, -1.0, 1.0));
	gui.add(p3.setup("rightHand", 0.0, -1.0, 1.0));
	gui.add(p4.setup("blobDensity", 0.8, 0.0, 2.0));
	gui.add(p5.setup("frequency", 0.18, 0.0, 2.0));
	gui.add(p6.setup("scalar", 2.5, -1.0, 3.0));

	jointDistance = 1.f;

}

//--------------------------------------------------------------
void testApp::update()
{
	kinect.update();

	if(kinect.isNewSkeleton()) {
		for( int i = 0; i < kinect.getSkeletons().size(); i++) 
		{

			if(kinect.getSkeletons().at(i).find(NUI_SKELETON_POSITION_HEAD) != kinect.getSkeletons().at(i).end())
			{

				// just get the first one
				SkeletonBone headBone = kinect.getSkeletons().at(i).find(NUI_SKELETON_POSITION_HEAD)->second;
				SkeletonBone lHandBone = kinect.getSkeletons().at(i).find(NUI_SKELETON_POSITION_HAND_LEFT)->second;
				SkeletonBone rHandBone = kinect.getSkeletons().at(i).find(NUI_SKELETON_POSITION_HAND_RIGHT)->second;
				ofVec3f hb( headBone.getScreenPosition().x, headBone.getScreenPosition().y, 0 );
				head = head.getInterpolated(hb, 0.5);
				head.z =  ofInterpolateCosine( head.z, headBone.getStartPosition().x, 0.5) + 0.1;
				ofVec3f lhb(lHandBone.getScreenPosition().x, lHandBone.getScreenPosition().y, 0);
				lHand = lHand.getInterpolated( lhb, 0.5);
				lHand.z = ofInterpolateCosine( lHand.z, lHandBone.getStartPosition().x, 0.5);
				ofVec3f rhb(rHandBone.getScreenPosition().x, rHandBone.getScreenPosition().y, 0);
				rHand = rHand.getInterpolated( rhb, 0.5);
				rHand.z = ofInterpolateCosine( rHand.z, rHandBone.getStartPosition().x, 0.5);

				cout << headBone.getScreenPosition()  << endl;
				cout << rHandBone.getScreenPosition() << endl;
				cout << lHandBone.getScreenPosition() << endl;

				//cout << kinect.getSkeletons().at(i).find(NUI_SKELETON_POSITION_HEAD)->second.getScreenPosition() << endl;
				//cout << kinect.getSkeletons().at(i).find(NUI_SKELETON_POSITION_HAND_LEFT)->second.getScreenPosition() << endl;
				//cout << kinect.getSkeletons().at(i).find(NUI_SKELETON_POSITION_HAND_RIGHT)->second.getScreenPosition() << endl;

				jointDistance = head.distance(rHand);
				jointDistance += lHand.distance(rHand);
				jointDistance += lHand.distance(head);

				hasSkeleton = true;

				return;
			}
		}
	}
}

//--------------------------------------------------------------
void testApp::draw(){

	ofDisableAlphaBlending();

	ofPushMatrix();
	ofTranslate(512, 768/2);
	
	shader.begin();
	shader.setUniform1f("time", ofGetElapsedTimef()/2.0);
	shader.setUniform2f("resolution", 1024, 768);
	shader.setUniformTexture("videoTex", kinect.getColorTexture(), 0);

	if(hasSkeleton)
	{
		
		// something in here blows up in release mode
		shader.setUniform3f("headPoint", 1.0 + (head.x/-320.0), 1.0 + (head.y/-240.0) - 0.1,head.z);
		shader.setUniform3f("lHandPoint", 1.0 + (lHand.x/-320.0), 1.0 + (lHand.y/-240.0), lHand.z);
		shader.setUniform3f("rHandPoint", 1.0 + (rHand.x/-320.0), 1.0 + (rHand.y/-240.0), rHand.z);
		//shader.setUniform1f("frequency", p4);
		//shader.setUniform1f("scalar", p5);
		shader.setUniform1f("blobDensity", jointDistance/600.0);
		//shader.setUniform1f("blobDensity", 2.0);

	} else {

		shader.setUniform3f("headPoint", p1, p1, p1);
		shader.setUniform3f("lHandPoint", p2, p2, p2);
		shader.setUniform3f("rHandPoint", p3, p3, p3);
		shader.setUniform1f("frequency", p4);
		shader.setUniform1f("scalar", p5);
		shader.setUniform1f("blobDensity", p6);
	}
	 
	plane.draw();
	shader.end();

	ofPopMatrix();

	if(!hasSkeleton) 
	{
		ofEnableAlphaBlending();
		gui.draw();
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