#pragma once

#include "ofMain.h"

#include "KinectCommonBridgeLib.h"
#include "NuiSensor.h"
#pragma comment (lib, "KinectCommonBridge.lib") // add path to lib additional dependency dir $(TargetDir)

#define MAX_PLAYERS 7

class SkeletonBone
{
public:
	// lots of constness because we're putting these in a map and that
	// copies stuff all over the place
	const ofQuaternion getCameraRotation();
	const ofMatrix4x4 getCameraRotationMatrix();

	const ofVec3f& getStartPosition();
	const ofVec3f getScreenPosition();
	const ofQuaternion&	getRotation();
	const ofMatrix4x4& getRotationMatrix();

	const int getStartJoint();
	int getEndJoint();

	SkeletonBone( const Vector4& inPosition, const _NUI_SKELETON_BONE_ORIENTATION& bone );


private:

	ofMatrix4x4 cameraRotation;
	int	endJoint;
	int	startJoint;
	ofVec3f	position;
	ofMatrix4x4	rotation;
	ofVec2f screenPosition;
};

typedef map<_NUI_SKELETON_POSITION_INDEX, SkeletonBone> Skeleton;

class ofxKinectCommonBridge : protected ofThread {
  public:
	ofxKinectCommonBridge();

	// new API
	bool initSensor( int id = 0 );
	bool initDepthStream( int width, int height, bool nearMode = false, bool mapDepthToColor = false );
	bool initColorStream( int width, int height, bool mapColorToDepth = false );
	bool initIRStream( int width, int height );
	bool initSkeletonStream( bool seated );
	bool start();

	void stop();

  	/// is the current frame new?
	bool isFrameNew();
	bool isFrameNewVideo();
	bool isFrameNewDepth();
	bool isNewSkeleton();

	void setDepthClipping(float nearClip=500, float farClip=4000);
	
	/// updates the pixel buffers and textures
	///
	/// make sure to call this to update to the latest incoming frames
	void update();
	ofPixels& getColorPixelsRef();
	ofPixels & getDepthPixelsRef();       	///< grayscale values
	ofShortPixels & getRawDepthPixelsRef();	///< raw 11 bit values

	/// enable/disable frame loading into textures on update()
	void setUseTexture(bool bUse);

	/// draw the video texture
	void draw(float x, float y, float w, float h);
	void draw(float x, float y);
	void draw(const ofPoint& point);
	void draw(const ofRectangle& rect);

	/// draw the grayscale depth texture
	void drawRawDepth(float x, float y, float w, float h);
	void drawRawDepth(float x, float y);
	void drawRawDepth(const ofPoint& point);
	void drawRawDepth(const ofRectangle& rect);

	/// draw the grayscale depth texture
	void drawDepth(float x, float y, float w, float h);
	void drawDepth(float x, float y);
	void drawDepth(const ofPoint& point);
	void drawDepth(const ofRectangle& rect);

	void drawIR( float x, float y, float w, float h );

	//do we have a player? 1-7 index
	int numPlayers();
	bool hasPlayer(int index);
	vector<int> getPlayers(); //list of all players currently present
	ofPixels& getPlayerPixels(int index);
	ofTexture& getPlayerTexture(int index);
	
	void drawPlayerTextures(float x, float y);
	void drawPlayerTextures(float x, float y, float w, float h);
	void drawPlayerTextures(ofPoint point);
	void drawPlayerTextures(ofRectangle rect);

	vector<Skeleton> &getSkeletons();
	void drawSkeleton(int index);

	ofTexture &getRawDepthTexture() {
		return rawDepthTex;
	}

	ofTexture &getDepthTexture() {
		return depthTex;
	}


	ofTexture &getColorTexture() {
		return videoTex;
	}

  private:

    KCBHANDLE hKinect;
	KINECT_IMAGE_FRAME_FORMAT depthFormat;
	KINECT_IMAGE_FRAME_FORMAT colorFormat;
	NUI_SKELETON_FRAME k4wSkeletons;

	ofPixels playerPixels[MAX_PLAYERS];
	ofTexture playerTexture[MAX_PLAYERS];

  	bool bInited;
	bool bStarted;
	vector<Skeleton> skeletons;
	map<USHORT,bool> playersPresent;

	//quantize depth buffer to 8 bit range
	vector<unsigned char> depthLookupTable;
	void updateDepthLookupTable();
	void updateDepthPixels();
	void updateIRPixels();
	bool bNearWhite;
	float nearClipping, farClipping;

  	bool bUseTexture;
	ofTexture depthTex; ///< the depth texture
	ofTexture rawDepthTex; ///<
	ofTexture videoTex; ///< the RGB texture
	//ofTexture irTex;

	ofPixels videoPixels;
	ofPixels videoPixelsBack;			///< rgb back
	ofPixels depthPixels;
	ofPixels depthPixelsBack;
	ofShortPixels depthPixelsRaw;
	ofShortPixels depthPixelsRawBack;	///< depth back

	ofShortPixels irPixelsRaw;
	ofShortPixels irPixelsBackRaw;
	ofPixels irPixels;
	ofPixels irPixelsBack;

	bool bIsFrameNewVideo;
	bool bNeedsUpdateVideo;
	bool bIsFrameNewDepth;
	bool bNeedsUpdateDepth;
	bool bNeedsUpdateSkeleton;
	bool bIsSkeletonFrameNew;
	bool bProgrammableRenderer;

	bool bVideoIsInfrared;
	bool bUsingSkeletons;
	bool bUsingDepth;

	BYTE *irPixelByteArray;

	void threadedFunction();

	bool mappingColorToDepth;
	bool mappingDepthToColor;
	bool beginMappingColorToDepth;

	NUI_IMAGE_RESOLUTION colorRes;
	NUI_IMAGE_RESOLUTION depthRes;

	INuiSensor *nuiSensor;
	INuiCoordinateMapper *mapper;


};
