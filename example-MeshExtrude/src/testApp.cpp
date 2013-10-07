#include "testApp.h"

/**
 * tig: This example shows how to draw instanced geometry using 
 * ofVboMesh.drawInstanced() and matching GLSL shader code.
 * The most interesting things will happen in the shader files, 
 * and in the draw() method, where everything is set up properly 
 * to allow instanced rendering.
 *
 * Note that you can feed any ofVboMesh to the instanced draw method.
 *
 */

#define USE_PROGRAMMABLE_GL

void testApp::addFace(ofMesh& mesh, ofVec3f a, ofVec3f b, ofVec3f c) {
	mesh.addVertex(a);
	mesh.addVertex(b);
	mesh.addVertex(c);
}

//--------------------------------------------------------------
void testApp::addFace(ofMesh& mesh, ofVec3f a, ofVec3f b, ofVec3f c, ofVec3f d) {
	addFace(mesh, a, b, c);
	addFace(mesh, a, c, d);
}

//--------------------------------------------------------------
void testApp::setup(){
	
	// initialize variables:
	
	isShaderDirty = true;  // this flag will tell us whether to reload our shader from disk.
						   // this allows you to change your shaders without having to restart
						   // your app. we'll set it up so that pressing the SPACE key on your
						   // keyboard will reload the shader.
	
	// initialize screen, lock framerate to vsync:

	//ofSetFrameRate(0);
	//ofSetVerticalSync(true);

	
	// generate a box vboMesh from a primitive.
	
	ofBoxPrimitive tmpBox;
	// set the size to be 2 units.
	tmpBox.set(9);
	
	for( int i = 0; i < 60; i++ ) {
		for( int j = 0; j < 60; j++) {

			ofBoxPrimitive b2 = tmpBox;

			for( int k = 0; k < b2.getMesh().getVertices().size(); k++ ) {
				//cout << b2.getMesh().getVertex(k) << endl;
				
				b2.getMesh().getVerticesPointer()[k].set( b2.getMesh().getVertex(k).x + (i * 12), 
														  b2.getMesh().getVertex(k).y + (j * 12), 
														  b2.getMesh().getVertex(k).z);

			}

			

			mVboBox.append(b2.getMesh());
			//mVboBox = tmpBox.getMesh();
		}
	}

	fbo.allocate(1024, 768, GL_RGB);

	fbo.begin();
	ofClear(255,255,255, 0);
    fbo.end();
	

	// load the depth image into our texture
	//ofLoadImage(mTexDepth, "puppy.jpg");
	raysTexture.loadImage("rays.jpg");
	rayShader.load("shaders/rays.vert", "shaders/rays.frag");
	
	depthDisplace.load("shaders/instanced.vert", "shaders/instanced.frag");

	mCamMain.setDistance(800);
	mCamMain.setPosition(0, 600, 800);
	mCamMain.enableMouseInput();

	kinect.init( 0 );
	
	kinect.startColorStream(640, 480);
	kinect.startDepthStream(640, 480, true);

	kinect.start();
	
	plane.set(2048, 1536, 4, 4);
	plane.mapTexCoords(0, 0, 1024, 768);

	gui.setup();
	gui.add(exposureSlider.setup("exposure", 0.79, 0.0, 2.0));
	gui.add(densitySlider.setup("density", 0.49, 0.0, 2.0));
	gui.add(weightSlider.setup("weight", 1.22, 0.0, 2.0));
	gui.add(decaySlider.setup("decay", 0.26, 0.0, 2.0));

}

//--------------------------------------------------------------
void testApp::update(){
	
	ofSetWindowTitle(ofToString(ofGetFrameRate(), 2));

	kinect.update();
	
}

//--------------------------------------------------------------
void testApp::draw()
{
	
	fbo.begin();
	ofEnableDepthTest();
	ofClear(255,255,255, 0);
	// we don't care about alpha blending in this example, and by default alpha blending is on in openFrameworks > 0.8.0
	// so we de-activate it for now.
	ofDisableAlphaBlending();
	
	ofBackgroundGradient(ofColor(18,33,54), ofColor(18,22,28));
	
	ofSetColor(ofColor::white);
	mCamMain.begin();
	// bind the shader
	depthDisplace.begin();
	// give the shader access to our texture
	depthDisplace.setUniformTexture("tex0", kinect.getDepthTexture(), 0);
	depthDisplace.setUniformTexture("tex1", kinect.getColorTexture(), 1);

	depthDisplace.setUniform1f("timeValue", ofGetElapsedTimef());
	mVboBox.draw(OF_MESH_FILL);
	depthDisplace.end();
	mCamMain.end();
	ofDisableDepthTest();

	fbo.end();

	ofEnableAlphaBlending();

	rayShader.begin();
	rayShader.setUniformTexture("rtex", raysTexture.getTextureReference(), 0);
	rayShader.setUniformTexture("otex", fbo.getTextureReference(), 1);
	rayShader.setUniform2f("lightPositionOnScreen", mouseX, mouseY);

	rayShader.setUniform1f("exposure", exposureSlider );
	rayShader.setUniform1f("decay", decaySlider);
	rayShader.setUniform1f("density", densitySlider);
	rayShader.setUniform1f("weight", weightSlider);

	plane.draw();
	//fbo.getTextureReference(0).draw(0,0);
	//fbo.draw(0,0);
	rayShader.end();

	gui.draw();

	//ofEnableAlphaBlending();
	//fbo.draw(0,0);


	//ofSetColor(ofColor::white);
	//ofDrawBitmapString("Use mouse to move camera.\nPress 'f' to toggle fullscreen;\nSPACEBAR to reload shader.", 10, 20);


	//ofEnableAlphaBlending();
	
	
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	
}

void testApp::exit() {
	kinect.stop();
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	
	switch (key) {
		case ' ':
			isShaderDirty = true;
			// mark the shader as dirty - this will reload the shader.
			break;
		case 'f':
			ofToggleFullscreen();
			break;
		default:
			break;
	}
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	
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
