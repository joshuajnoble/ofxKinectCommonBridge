#include "testApp.h"

#define USE_PROGRAMMABLE_GL

//--------------------------------------------------------------
void testApp::setup(){
	
	// generate a box vboMesh from a primitive.
	
	ofBoxPrimitive tmpBox;
	// set the size to be 2 units.
	tmpBox.set(10);
	
	for( int i = 0; i < 64; i++ ) {
		for( int j = 0; j < 48; j++) {

			ofBoxPrimitive b2 = tmpBox;

			for( int k = 0; k < b2.getMesh().getVertices().size(); k++ ) {
				b2.getMesh().getVerticesPointer()[k].set( b2.getMesh().getVertex(k).x + (i * 16), 
														  b2.getMesh().getVertex(k).y + (j * 14), 
														  b2.getMesh().getVertex(k).z);
			}
			mVboBox.append(b2.getMesh());
		}
	}

	ofFbo::Settings s;
	s.depthStencilAsTexture = true;
	s.useDepth = true;
	s.width = 1024;
	s.height = 768;
	s.internalformat = GL_RGB;
	fbo.allocate(s);

	fbo.begin();
	ofClear(255,255,255, 0);
	glClear(GL_DEPTH_BUFFER_BIT );
    fbo.end();
	

	// load the depth image into our texture
	raysTexture.loadImage("rays.jpg");

	rayShader.load("shaders/rays.vert", "shaders/rays.frag");
	depthDisplace.load("shaders/displace.vert", "shaders/displace.frag");

	mCamMain.setDistance(800);
	mCamMain.setPosition(512, 384, 800);
	mCamMain.enableMouseInput();

	kinect.initSensor( 0 );
	
	kinect.initColorStream(640, 480);
	kinect.initDepthStream(320, 240, false);

	kinect.start();
	
	plane.set(1024, 768, 4, 4);
	plane.mapTexCoords(0, 0, 1024, 768);

	gui.setup();
	gui.add(exposureSlider.setup("exposure", 0.79, 0.0, 2.0));
	gui.add(densitySlider.setup("density", 0.49, 0.0, 2.0));
	gui.add(weightSlider.setup("weight", 1.22, 0.0, 2.0));
	gui.add(decaySlider.setup("decay", 0.26, 0.0, 2.0));

	ofEnableDepthTest();

}

//--------------------------------------------------------------
void testApp::update()
{	
	kinect.update();
}

//--------------------------------------------------------------
void testApp::draw()
{
	ofMatrix4x4 camTextureMatrix;

	//ofEnableDepthTest();
	glClear(GL_DEPTH_BUFFER_BIT );
	fbo.begin();
	//ofClear(255,255,255, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ofDisableAlphaBlending();
	
	//ofBackgroundGradient(ofColor(18,33,54), ofColor(18,22,28));
	
	ofSetColor(ofColor::white);
	mCamMain.begin();

	// bind the shader
	depthDisplace.begin();
	// give the shader access to our texture
	depthDisplace.setUniformTexture("tex0", kinect.getDepthTexture(), 0);
	depthDisplace.setUniformTexture("tex1", kinect.getColorTexture(), 1);

	// figure out depth texture in FBO
	depthDisplace.setUniform1f("timeValue", ofGetElapsedTimef());
	mVboBox.draw(OF_MESH_FILL);
	depthDisplace.end();
	mCamMain.end();
	fbo.end();
	//ofDisableDepthTest();

	//ofEnableAlphaBlending();

	rayShader.begin();

	rayShader.setUniformTexture("rtex", raysTexture.getTextureReference(), 0);
	rayShader.setUniformTexture("otex", fbo.getTextureReference(), 1);
	rayShader.setUniformTexture("dtex", fbo.getDepthTexture(), 2);
	//rayShader.setUniform2f("lightPositionOnScreen", mouseX, mouseY);

	rayShader.setUniformMatrix4f("camModelViewProjectionMatrix", mCamMain.getModelViewProjectionMatrix() );
	rayShader.setUniform1f("exposure", exposureSlider );
	rayShader.setUniform1f("decay", decaySlider);
	rayShader.setUniform1f("density", densitySlider);
	rayShader.setUniform1f("weight", weightSlider);

	ofPushMatrix();
	ofTranslate(512, 384);

	plane.draw();

	ofPopMatrix();
	//fbo.getTextureReference(0).draw(0,0);
	//fbo.draw(0,0);
	rayShader.end();

	ofDisableDepthTest();
	ofEnableAlphaBlending();

	gui.draw();

	ofDisableAlphaBlending();

//	ofEnableAlphaBlending();
//	fbo.getDepthTexture().draw(0,0);

	
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
			//mCamMain.setPosition(512, 384, 800)
			mCamMain.reset();
			mCamMain.setDistance(800);
			mCamMain.setPosition(512, 384, 800);
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
