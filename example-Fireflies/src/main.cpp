#include "ofMain.h"
#include "testApp.h"

#include "ofGLProgrammableRenderer.h"

#define USE_PROGRAMMABLE_RENDERER

//========================================================================
int main( ){

#ifdef USE_PROGRAMMABLE_RENDERER
	ofPtr<ofBaseRenderer> renderer(new ofGLProgrammableRenderer(true));
	ofSetCurrentRenderer(renderer);
#endif

	ofSetupOpenGL(1024,768, OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp( new testApp());

}
