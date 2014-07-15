#include "ofxKinectCommonBridge.h"

#ifdef KCB_ENABLE_SPEECH
// speech event declaration
ofEvent<ofxKCBSpeechEvent> ofxKCBSpeechEvent::event;

#endif

#ifdef KCB_ENABLE_FT

ofVec3f ofxKCBFace::getLocationByIdentifier(FACE_POSITIONS position)
{
	return mesh.getVertex( (int) position );
}

ofRectangle ofxKCBFace::getFeatureBounding(FACE_POSITIONS position)
{
	//return mesh.getVertex( (int) position );
	// find the bounding box for all the features and return;

	ofRectangle bounds;
	return bounds;
}

#endif


SkeletonBone::SkeletonBone ( const Vector4& inPosition, const _NUI_SKELETON_BONE_ORIENTATION& orient, const NUI_SKELETON_POSITION_TRACKING_STATE& trackingState) {

	cameraRotation.set( orient.absoluteRotation.rotationMatrix.M11, orient.absoluteRotation.rotationMatrix.M12, orient.absoluteRotation.rotationMatrix.M13, orient.absoluteRotation.rotationMatrix.M14,
		orient.absoluteRotation.rotationMatrix.M21, orient.absoluteRotation.rotationMatrix.M22, orient.absoluteRotation.rotationMatrix.M23, orient.absoluteRotation.rotationMatrix.M24,
		orient.absoluteRotation.rotationMatrix.M31, orient.absoluteRotation.rotationMatrix.M32, orient.absoluteRotation.rotationMatrix.M33, orient.absoluteRotation.rotationMatrix.M34,
		orient.absoluteRotation.rotationMatrix.M41, orient.absoluteRotation.rotationMatrix.M42, orient.absoluteRotation.rotationMatrix.M43, orient.absoluteRotation.rotationMatrix.M44);

	position.set( inPosition.x, inPosition.y, inPosition.z );

	startJoint = orient.startJoint;
	endJoint = orient.endJoint;

	NuiTransformSkeletonToDepthImage(inPosition, &(screenPosition.x), &(screenPosition.y), NUI_IMAGE_RESOLUTION_640x480);

	rotation.set( orient.hierarchicalRotation.rotationMatrix.M11, orient.hierarchicalRotation.rotationMatrix.M12, orient.hierarchicalRotation.rotationMatrix.M13, orient.hierarchicalRotation.rotationMatrix.M14,
		orient.hierarchicalRotation.rotationMatrix.M21, orient.hierarchicalRotation.rotationMatrix.M22, orient.hierarchicalRotation.rotationMatrix.M23, orient.hierarchicalRotation.rotationMatrix.M24,
		orient.hierarchicalRotation.rotationMatrix.M31, orient.hierarchicalRotation.rotationMatrix.M32, orient.hierarchicalRotation.rotationMatrix.M33, orient.hierarchicalRotation.rotationMatrix.M34,
		orient.hierarchicalRotation.rotationMatrix.M41, orient.hierarchicalRotation.rotationMatrix.M42, orient.hierarchicalRotation.rotationMatrix.M43, orient.hierarchicalRotation.rotationMatrix.M44);
	
	switch(trackingState) {
	case NUI_SKELETON_POSITION_NOT_TRACKED:
		this->trackingState = NotTracked;
		break;
	case NUI_SKELETON_POSITION_INFERRED:
		this->trackingState = Inferred;
		break;
	case NUI_SKELETON_POSITION_TRACKED:
		this->trackingState = Tracked;
		break;
	}
}

const ofVec3f& SkeletonBone::getStartPosition() {
	return position;
}

const ofQuaternion&	SkeletonBone::getRotation() {
	return rotation.getRotate();
}

const ofMatrix4x4& SkeletonBone::getRotationMatrix() {
	return rotation;
}

const int SkeletonBone::getStartJoint() {
	return startJoint;
}

const ofQuaternion SkeletonBone::getCameraRotation() {
	return cameraRotation.getRotate();
}

const ofMatrix4x4 SkeletonBone::getCameraRotationMatrix() {
	return rotation;
}

int SkeletonBone::getEndJoint() {
	return endJoint;
}

SkeletonBone::TrackingState SkeletonBone::getTrackingState() {
	return trackingState;
}

const ofVec3f SkeletonBone::getScreenPosition() {
	return screenPosition;
}

ofxKinectCommonBridge::ofxKinectCommonBridge(){
	hKinect = NULL;
	mapper = NULL;
	nuiSensor = NULL;


	beginMappingColorToDepth = false;

	bIsFrameNewVideo = false;
	bNeedsUpdateVideo = false;
	bIsFrameNewDepth = false;
	bNeedsUpdateDepth = false;
	bVideoIsInfrared = false;
	bUsingSpeech = false;
	bUsingFaceTrack = false;
	bInitedColor = false;
	bInitedDepth = false;
	bInitedIR = false;

#ifdef KCB_USING_SPEECH
	bInitedSpeech = false;
#endif
	bStarted = false;

	mappingColorToDepth = false;
	mappingDepthToColor = false;

	bUsingSkeletons = false;
  	bUseTexture = true;
	bProgrammableRenderer = false;
	bNearWhite = false;
	
	setDepthClipping();
}

//---------------------------------------------------------------------------
void ofxKinectCommonBridge::setDepthClipping(float nearClip, float farClip){
	nearClipping = nearClip;
	farClipping = farClip;
	updateDepthLookupTable();
}

//---------------------------------------------------------------------------
void ofxKinectCommonBridge::updateDepthLookupTable()
{
	unsigned char nearColor = bNearWhite ? 255 : 0;
	unsigned char farColor = bNearWhite ? 0 : 255;
	unsigned int maxDepthLevels = 10001;
	depthLookupTable.resize(maxDepthLevels);
	depthLookupTable[0] = 0;
	for(unsigned int i = 1; i < maxDepthLevels; i++)
	{
		depthLookupTable[i] = ofMap(i, nearClipping, farClipping, nearColor, farColor, true);
	}
}

/// is the current frame new?
bool ofxKinectCommonBridge::isFrameNew(){
	return isFrameNewVideo() || isFrameNewDepth();
}

bool ofxKinectCommonBridge::isFrameNewVideo(){
	return bIsFrameNewVideo;
}

bool ofxKinectCommonBridge::isFrameNewDepth(){
	return bIsFrameNewDepth;
}

bool ofxKinectCommonBridge::isNewSkeleton() {
	return bNeedsUpdateSkeleton;
}

vector<Skeleton> &ofxKinectCommonBridge::getSkeletons() {
	return skeletons;
}
/// updates the pixel buffers and textures
/// make sure to call this to update to the latest incoming frames
void ofxKinectCommonBridge::update()
{
	if(!bStarted)
	{
		ofLogError("ofxKinectCommonBridge::update") << "Kinect not started";
		return;
	}

	// update color or IR pixels and textures if necessary
	if(bNeedsUpdateVideo)
	{
		bIsFrameNewVideo = true;

		swap(videoPixels,videoPixelsBack);

		// if you're mapping color pix to depth space, downscale color pix
		/*if(mappingColorToDepth && beginMappingColorToDepth)
		{

			NUI_DEPTH_IMAGE_POINT  *pts = new NUI_DEPTH_IMAGE_POINT[colorFormat.dwHeight*colorFormat.dwWidth];
			NUI_DEPTH_IMAGE_PIXEL  *depth = new NUI_DEPTH_IMAGE_PIXEL[depthFormat.dwHeight*depthFormat.dwWidth];

			int i = 0; 
			while ( i < (depthFormat.dwWidth*depthFormat.dwHeight)) {
				depth[i].depth = (USHORT) depthPixelsRaw.getPixels()[i];
				depth[i].playerIndex = 0;
				i++;
			}

			HRESULT mapResult;
			mapResult = mapper->MapColorFrameToDepthFrame(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, NUI_IMAGE_RESOLUTION_640x480,
						640 * 480, depth,
						640 * 480, pts);

			for( int i = 0; i < (depthFormat.dwWidth*depthFormat.dwHeight); i++ )
			{
				videoPixels[i] = videoPixels[pts[i].y * depthFormat.dwWidth + pts[i].x];
			}

			delete[] pts;
			delete[] depth;

		}*/

		bNeedsUpdateVideo = false;

		if(bUseTexture) {
			if(bVideoIsInfrared) 
			{
				if(bProgrammableRenderer){
					videoTex.loadData(videoPixels.getPixels(), colorFormat.dwWidth, colorFormat.dwHeight, GL_RED);
				} else {
					videoTex.loadData(videoPixels.getPixels(), colorFormat.dwWidth, colorFormat.dwHeight, GL_LUMINANCE16);
				}
			} 
			else 
			{
				if( bProgrammableRenderer ) {
					// programmable renderer likes this
					// TODO
					// swizzle this to rgb & a -> GL_ONE
					videoTex.loadData(videoPixels.getPixels(), colorFormat.dwWidth, colorFormat.dwHeight, GL_BGRA);
				} else {
					videoTex.loadData(videoPixels.getPixels(), colorFormat.dwWidth, colorFormat.dwHeight, GL_RGBA);
				}
			}
		}
	} else {
		bIsFrameNewVideo = false;
	}


	// update depth pixels and texture if necessary
	if(bNeedsUpdateDepth){

		if(mappingColorToDepth) {
			beginMappingColorToDepth = true;
		}

		bIsFrameNewDepth = true;
		swap(depthPixelsRaw, depthPixelsRawBack);
		bNeedsUpdateDepth = false;
		
		// if mapping depth to color, upscale depth
		if(mappingDepthToColor) 
		{
			NUI_COLOR_IMAGE_POINT *pts = new NUI_COLOR_IMAGE_POINT[colorFormat.dwWidth*colorFormat.dwHeight];
			
			int i = 0; 
			while ( i < (depthFormat.dwWidth*depthFormat.dwHeight)) {
				depthPixelsNui[i].depth = NuiDepthPixelToDepth(depthPixelsRaw[i]);
				depthPixelsNui[i].playerIndex = NuiDepthPixelToPlayerIndex(depthPixelsRaw[i]);
				i++;
			}
			
			HRESULT mapResult;
			mapResult = mapper->MapDepthFrameToColorFrame(depthRes, (depthFormat.dwWidth*depthFormat.dwHeight), depthPixelsNui, NUI_IMAGE_TYPE_COLOR, colorRes, (depthFormat.dwWidth*depthFormat.dwHeight), pts);

			if(SUCCEEDED(mapResult))
			{

				for( int i = 0; i < (depthFormat.dwWidth*depthFormat.dwHeight); i++ ) {
					if(pts[i].x > 0 && pts[i].x < depthFormat.dwWidth && pts[i].y > 0 && pts[i].y < depthFormat.dwHeight) {
						depthPixels[i] = depthLookupTable[ ofClamp(depthPixelsRaw[pts[i].y * depthFormat.dwWidth + pts[i].x] >> 4, 0, depthLookupTable.size()-1 ) ];
					} else {
						depthPixels[i] = 0;
					}
				}
			} else {
				ofLog() << " mapping error " << mapResult << endl;
			}

			delete[] pts;
		
			for(int i = 0; i < depthPixels.getWidth()*depthPixels.getHeight(); i++) {
				depthPixelsRaw[i] = depthPixelsRaw[i] >> 4;
			}

		} else {

			for(int i = 0; i < depthPixels.getWidth()*depthPixels.getHeight(); i++) {
				depthPixels[i] = depthLookupTable[ ofClamp(depthPixelsRaw[i] >> 4, 0, depthLookupTable.size()-1 ) ];
				depthPixelsNui[i].depth = NuiDepthPixelToDepth(depthPixelsRaw[i]);
				depthPixelsNui[i].playerIndex = NuiDepthPixelToPlayerIndex(depthPixelsRaw[i]);
				depthPixelsRaw[i] = depthPixelsRaw[i] >> 4;
			}
		}


		if(bUseTexture) {
			//depthTex.loadData(depthPixels.getPixels(), depthFormat.dwWidth, depthFormat.dwHeight, GL_LUMINANCE);
			if( bProgrammableRenderer ) {
				depthTex.loadData(depthPixels.getPixels(), depthFormat.dwWidth, depthFormat.dwHeight, GL_RED);
				rawDepthTex.loadData(depthPixelsRaw.getPixels(), depthFormat.dwWidth, depthFormat.dwHeight, GL_RED);
			} else {
				depthTex.loadData(depthPixels.getPixels(), depthFormat.dwWidth, depthFormat.dwHeight, GL_LUMINANCE);
				rawDepthTex.loadData(depthPixelsRaw.getPixels(), depthFormat.dwWidth, depthFormat.dwHeight, GL_LUMINANCE16);
			}
		}
	} else {
		bIsFrameNewDepth = false;
	}

	// update skeletons if necessary
	if(bUsingSkeletons && bNeedsUpdateSkeleton)
	{	

		bIsSkeletonFrameNew = true;
		bNeedsUpdateSkeleton = false;
		bool foundSkeleton = false;

		for ( int i = 0; i < NUI_SKELETON_COUNT; i++ ) 
		{
			skeletons.at( i ).clear();

			if (  k4wSkeletons.SkeletonData[ i ].eTrackingState == NUI_SKELETON_TRACKED || k4wSkeletons.SkeletonData[ i ].eTrackingState == NUI_SKELETON_POSITION_ONLY ) {
				//cout << " we have a skeleton " << ofGetElapsedTimeMillis() << endl;
				_NUI_SKELETON_BONE_ORIENTATION bones[ NUI_SKELETON_POSITION_COUNT ];
				if(SUCCEEDED(NuiSkeletonCalculateBoneOrientations( &(k4wSkeletons.SkeletonData[i]), bones ))) {
					//error( hr );
				}

				for ( int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++ ) 
				{
					SkeletonBone bone( k4wSkeletons.SkeletonData[i].SkeletonPositions[j], bones[j], k4wSkeletons.SkeletonData[i].eSkeletonPositionTrackingState[j] );
					skeletons.at(i).insert( std::pair<NUI_SKELETON_POSITION_INDEX, SkeletonBone>( NUI_SKELETON_POSITION_INDEX(j), bone ) );
				}
				bNeedsUpdateSkeleton = true;
			}
		}

	} else {
		bNeedsUpdateSkeleton = false;
	}

#ifdef KCB_ENABLE_SPEECH
	if(bUpdateSpeech)
	{
		ofxKCBSpeechEvent spEvent;
		spEvent.detectedSpeech = speechData.detectedSpeech;
		spEvent.confidence = speechData.confidence;

		ofNotifyEvent( ofxKCBSpeechEvent::event, spEvent, this);

		bUpdateSpeech = false;
	}
#endif

#ifdef KCB_ENABLE_FT
	if(bUpdateFaces)
	{
		//swap<ofxKCBFace>( faceData, faceDataBack ); // copy it in, need lock?
		faceData = faceDataBack;
		bIsFaceNew = true;
	}
#endif
}

#ifdef KCB_ENABLE_FT

void ofxKinectCommonBridge::updateFaceTrackingData( IFTResult* ftResult )
{

	zoomFactor = 1.0;

	//FT_VECTOR2D* points2D;
	//UINT pointCount;

	//ftResult->Get2DShapePoints( &points2D, &pointCount );

	RECT pRect;
	ftResult->GetFaceRect( &pRect );
	
	faceDataBack.rect.set( ofVec2f(pRect.left, pRect.top), ofVec2f(pRect.right, pRect.bottom ));

	FLOAT *AUCoefficients;
    UINT AUCount;
    ftResult->GetAUCoefficients(&AUCoefficients, &AUCount);

    FLOAT scale, rotationXYZ[3], translationXYZ[3];
    ftResult->Get3DPose(&scale, rotationXYZ, translationXYZ);

	faceDataBack.rotation.set(rotationXYZ[0], rotationXYZ[1], rotationXYZ[2]);
	faceDataBack.translation.set(translationXYZ[0], translationXYZ[1], translationXYZ[2]);

	IFTFaceTracker *ftracker;
	KinectGetFaceTracker( hKinect, &ftracker );

	FLOAT* pSU = NULL;
    UINT numSU;
    BOOL suConverged;
    ftracker->GetShapeUnits(NULL, &pSU, &numSU, &suConverged);

	IFTModel *ftModel;
	ftracker->GetFaceModel( &ftModel );

	FT_CAMERA_CONFIG ftCameraConfig;
	KinectGetColorStreamCameraConfig( hKinect, ftCameraConfig );

	POINT viewOffset = {0, 0};

	UINT vertexCount = ftModel->GetVertexCount();
    FT_VECTOR2D* pPts2D = reinterpret_cast<FT_VECTOR2D*>(_malloca(sizeof(FT_VECTOR2D) * vertexCount));

	ftModel->GetProjectedShape(&ftCameraConfig, zoomFactor, viewOffset, pSU, ftModel->GetSUCount(), AUCoefficients, 
		AUCount, scale, rotationXYZ, translationXYZ, pPts2D, vertexCount);

	FT_TRIANGLE* pTriangles;
    UINT triangleCount;
    ftModel->GetTriangles(&pTriangles, &triangleCount);

	faceDataBack.mesh.clear();
	for( UINT i = 0; i<vertexCount; i++) 
	{
		ofVec3f v( pPts2D[i].x, pPts2D[i].y, 0);
		faceDataBack.mesh.getVertices().push_back(v);
	}

	for( UINT i = 0; i<triangleCount; i++) 
	{
		//if( pTriangles[i].i < pointCount && pTriangles[i].j < pointCount && pTriangles[i].k < pointCount )
		//{
			faceDataBack.mesh.addTriangle( pTriangles[i].i, pTriangles[i].j, pTriangles[i].k );
		//}
	}

	_freea(pPts2D);

	ftModel->Release();
	ftracker->Release();
}

ofxKCBFace& ofxKinectCommonBridge::getFaceData() {
	return faceData;
}

#endif


//------------------------------------
ofPixels& ofxKinectCommonBridge::getColorPixelsRef(){
	return videoPixels;
}

//------------------------------------
ofPixels & ofxKinectCommonBridge::getDepthPixelsRef(){       	///< grayscale values
	return depthPixels;
}

//------------------------------------
ofShortPixels & ofxKinectCommonBridge::getRawDepthPixelsRef(){
	return depthPixelsRaw;
}

//------------------------------------
NUI_DEPTH_IMAGE_PIXEL* ofxKinectCommonBridge::getNuiDepthPixelsRef(){
	return depthPixelsNui;
}

//------------------------------------
void ofxKinectCommonBridge::setUseTexture(bool bUse){
	bUseTexture = bUse;
}

//----------------------------------------------------------
void ofxKinectCommonBridge::draw(float _x, float _y, float _w, float _h) {
	if(bUseTexture) {
		videoTex.draw(_x, _y, _w, _h);
	}
}

//----------------------------------------------------------
void ofxKinectCommonBridge::draw(float _x, float _y) {
	draw(_x, _y, (float)colorFormat.dwWidth, (float)colorFormat.dwHeight);
}

//----------------------------------------------------------
void ofxKinectCommonBridge::draw(const ofPoint & point) {
	draw(point.x, point.y);
}

//----------------------------------------------------------
void ofxKinectCommonBridge::draw(const ofRectangle & rect) {
	draw(rect.x, rect.y, rect.width, rect.height);
}

//----------------------------------------------------------
void ofxKinectCommonBridge::drawRawDepth(float _x, float _y, float _w, float _h) {
	if(bUseTexture) {
		rawDepthTex.draw(_x, _y, _w, _h);
	}
}

//----------------------------------------------------------
void ofxKinectCommonBridge::drawRawDepth(float _x, float _y) {
	drawRawDepth(_x, _y, (float)colorFormat.dwWidth, (float)colorFormat.dwHeight);
}

//----------------------------------------------------------
void ofxKinectCommonBridge::drawRawDepth(const ofPoint & point) {
	drawRawDepth(point.x, point.y);
}

//----------------------------------------------------------
void ofxKinectCommonBridge::drawRawDepth(const ofRectangle & rect) {
	drawRawDepth(rect.x, rect.y, rect.width, rect.height);
}

//----------------------------------------------------------
void ofxKinectCommonBridge::drawDepth(float _x, float _y, float _w, float _h) {
	if(bUseTexture) {
		depthTex.draw(_x, _y, _w, _h);
	}
}

//---------------------------------------------------------------------------
void ofxKinectCommonBridge::drawDepth(float _x, float _y) {
	drawDepth(_x, _y, (float)depthFormat.dwWidth, (float)depthFormat.dwHeight);
}

//----------------------------------------------------------
void ofxKinectCommonBridge::drawDepth(const ofPoint & point) {
	drawDepth(point.x, point.y);
}

//----------------------------------------------------------
void ofxKinectCommonBridge::drawDepth(const ofRectangle & rect) {
	drawDepth(rect.x, rect.y, rect.width, rect.height);
}

void ofxKinectCommonBridge::drawSkeleton( int index )
{
	// Iterate through skeletons
	uint32_t i = 0;
	if(index > skeletons.size())
	{
		ofLog() << " skeleton index too high " << endl;
		return;
	}

	// Iterate through joints
	for ( Skeleton::iterator it = skeletons.at(index).begin(); it != skeletons.at(index).end(); ++it ) 
	{

		// Get position and rotation
		SkeletonBone bone	= it->second;

		ofSetColor(255, 255, 255);
		ofSetLineWidth(3.0); // fat lines
		int startJoint = bone.getStartJoint();
		// do we have a start joint?
		if ( skeletons.at(index).find( ( NUI_SKELETON_POSITION_INDEX ) startJoint ) != skeletons.at(index).end() ) 
		{
			// draw the line
			ofLine( bone.getScreenPosition(), skeletons.at(index).find( ( NUI_SKELETON_POSITION_INDEX ) startJoint )->second.getScreenPosition() );
		}

		ofSetColor(255, 0, 0);
		// Draw joint
		ofCircle( bone.getScreenPosition(), 10 );
	}

	ofSetColor(255, 255, 255);
}


bool ofxKinectCommonBridge::initSensor( int id )
{
	if(bStarted){
		ofLogError("ofxKinectCommonBridge::initSensor") << "Cannot configure once the sensor has already started" << endl;
		return false;
	}

	UINT count = KinectGetPortIDCount();
	WCHAR portID[KINECT_MAX_PORTID_LENGTH];

	if( !SUCCEEDED(KinectGetPortIDByIndex( id, _countof(portID), portID ))) {
		ofLog() << " can't find kinect of ID " << id << endl;
		return false;
	}

	hKinect = KinectOpenSensor(portID);

	if(!hKinect) {
		ofLogError("ofxKinectCommonBridge::initSensor") << " can't open Kinect of ID " << id;
		return false;
	}

	if(ofGetCurrentRenderer()->getType() == ofGLProgrammableRenderer::TYPE)
	{
		bProgrammableRenderer = true;
	}

	return true;
}

bool ofxKinectCommonBridge::initDepthStream( int width, int height, bool nearMode, bool mapDepthToColor )
{

	mappingDepthToColor = mapDepthToColor;

	if (mappingDepthToColor)
	{
		this->getNuiSensor().NuiGetCoordinateMapper(&mapper);
	}

	if(bStarted){
		ofLogError("ofxKinectCommonBridge::initDepthStream") << " Cannot configure once the sensor has already started";
		return false;
	}

	if( width == 320 ) {
		depthRes = NUI_IMAGE_RESOLUTION_320x240;
	} else if( width == 640 ) {
		depthRes= NUI_IMAGE_RESOLUTION_640x480;
	} else {
		ofLogError("ofxKinectCommonBridge::initDepthStream") << " invalid image size" << endl;
	}

	KINECT_IMAGE_FRAME_FORMAT df = { sizeof(KINECT_IMAGE_FRAME_FORMAT), 0 };
    KinectEnableDepthStream(hKinect, nearMode, depthRes, &df);
    if( KinectStreamStatusError != KinectGetDepthStreamStatus(hKinect) )
	{
		depthFormat = df;
		createDepthPixels();
	} 
	else
	{
		ofLogError("ofxKinectCommonBridge::open") << "Error opening depth stream";
		return false;
	}
	bInitedDepth = true;
	return true;
}

bool ofxKinectCommonBridge::createDepthPixels( int width, int height )
{

	if(height != 0 && width != 0) {
		depthFormat.dwWidth = width; // this might not work for you if you don't use a proper size;
		depthFormat.dwHeight = height; // this might not work for you if you don't use a proper size;
		depthFormat.cbBufferSize = width * height * 2;
		depthFormat.cbBytesPerPixel = 2;
	}

    if( hKinect != 0 )
	{
		depthPixelsNui = new NUI_DEPTH_IMAGE_PIXEL[(depthFormat.dwWidth*depthFormat.dwHeight)];

		if(bProgrammableRenderer) {
			depthPixels.allocate(depthFormat.dwWidth, depthFormat.dwHeight, OF_IMAGE_COLOR);
			depthPixelsBack.allocate(depthFormat.dwWidth, depthFormat.dwHeight, OF_IMAGE_COLOR);
		} else {
			depthPixels.allocate(depthFormat.dwWidth, depthFormat.dwHeight, OF_IMAGE_GRAYSCALE);
			depthPixelsBack.allocate(depthFormat.dwWidth, depthFormat.dwHeight, OF_IMAGE_GRAYSCALE);
		}

		depthPixelsRaw.allocate(depthFormat.dwWidth, depthFormat.dwHeight, OF_IMAGE_GRAYSCALE);
		depthPixelsRawBack.allocate(depthFormat.dwWidth, depthFormat.dwHeight, OF_IMAGE_GRAYSCALE);

		if(bUseTexture)
		{
			if(bProgrammableRenderer)
			{
				//int w, int h, int glInternalFormat, bool bUseARBExtention, int glFormat, int pixelType
				depthTex.allocate(depthFormat.dwWidth, depthFormat.dwHeight, GL_R8);//, true, GL_R8, GL_UNSIGNED_BYTE);
				depthTex.setRGToRGBASwizzles(true);

				//rawDepthTex.allocate(depthFormat.dwWidth, depthFormat.dwHeight, GL_R16, true, GL_RED, GL_UNSIGNED_SHORT);
				rawDepthTex.allocate(depthPixelsRaw, true);
				rawDepthTex.setRGToRGBASwizzles(true);

				cout << rawDepthTex.getWidth() << " " << rawDepthTex.getHeight() << endl;
				//depthTex.allocate(depthFormat.dwWidth, depthFormat.dwHeight, GL_RGB);
			} 
			else 
			{
				depthTex.allocate(depthFormat.dwWidth, depthFormat.dwHeight, GL_LUMINANCE);
				rawDepthTex.allocate(depthFormat.dwWidth, depthFormat.dwHeight, GL_LUMINANCE16);
			}
		}

	} 
	else
	{
		ofLogError("ofxKinectCommonBridge::createDepthPixels") << " No Kinect ";
		return false;
	}
	bInitedDepth = true;
	return true;
}

bool ofxKinectCommonBridge::initColorStream( int width, int height, bool mapColorToDepth )
{
	mappingColorToDepth = mapColorToDepth;
	if(mappingColorToDepth && mapper == NULL) 
	{
		/*
		// get the port ID from the simple api
		const WCHAR* wcPortID = KinectGetPortID(hKinect);

		// create an instance of the same sensor
		INuiSensor* nuiSensor = nullptr;
		HRESULT hr = NuiCreateSensorById(wcPortID, &nuiSensor);

		nuiSensor->NuiGetCoordinateMapper(&mapper);
		*/

		ofLogWarning("ofxKinectCommonBridge::initColorStream") << " mapping color to depth is not yet supported " << endl;
	}

	KINECT_IMAGE_FRAME_FORMAT cf = { sizeof(KINECT_IMAGE_FRAME_FORMAT), 0 };

	if( width == 320 ) {
		colorRes = NUI_IMAGE_RESOLUTION_320x240;
	} else if( width == 640 ) {
		colorRes = NUI_IMAGE_RESOLUTION_640x480;
	} else if( width == 1280 ) {
		colorRes = NUI_IMAGE_RESOLUTION_1280x960;
	} else {
		ofLog() << " invalid image size passed to startColorStream() " << endl;
	}
    
    KinectEnableColorStream(hKinect, colorRes, &cf);
	if( KinectStreamStatusError != KinectGetColorStreamStatus(hKinect) )
	{
		colorFormat = cf;
		createColorPixels();
	}

	if(bStarted){
		ofLogError("ofxKinectCommonBridge::initColorStream") << " Cannot configure once the sensor has already started";
		return false;
	}

	bInitedColor = true;
	return true;
}

bool ofxKinectCommonBridge::createColorPixels( int width, int height )
{

	if(height != 0 && width != 0) {
		colorFormat.dwWidth = width; // this might not work for you if you don't use a proper size;
		colorFormat.dwHeight = height; // this might not work for you if you don't use a proper size;
		colorFormat.cbBufferSize = width * height * 4;
		colorFormat.cbBytesPerPixel = 4;
	}

	videoPixels.allocate(colorFormat.dwWidth, colorFormat.dwHeight, OF_IMAGE_COLOR_ALPHA);
	videoPixelsBack.allocate(colorFormat.dwWidth, colorFormat.dwHeight, OF_IMAGE_COLOR_ALPHA);
	if(bUseTexture)
	{
		videoTex.allocate(colorFormat.dwWidth, colorFormat.dwHeight, GL_RGBA);
	}
	return true; // remove this
}

bool ofxKinectCommonBridge::initIRStream( int width, int height )
{
	if(bStarted){
		ofLogError("ofxKinectCommonBridge::startIRStream") << " Cannot configure when the sensor has already started";
		return false;
	}

	bVideoIsInfrared = true;

	_NUI_IMAGE_RESOLUTION res;
	if( width == 320 ) {
		res = NUI_IMAGE_RESOLUTION_320x240;
	} else if( width == 640 ) {
		res = NUI_IMAGE_RESOLUTION_640x480;
	} else if( width == 1280 ) {
		res = NUI_IMAGE_RESOLUTION_1280x960;
	} else {
		ofLog() << " invalid image size passed to startIRStream() " << endl;
	}

	KINECT_IMAGE_FRAME_FORMAT cf = { sizeof(KINECT_IMAGE_FRAME_FORMAT), 0 };
    
    KinectEnableIRStream(hKinect, res, &cf);
    if( KinectStreamStatusError != KinectGetIRStreamStatus(hKinect) )
	{

		// IR is two byte, but we can't use shortPixels so we'll make a raw array and put it together
		// in the update() method. Probably should be changed in future versions
		colorFormat = cf;

		ofLog() << "allocating a buffer of size " << colorFormat.dwWidth*colorFormat.dwHeight*sizeof(unsigned char)*2 << " when k4w wants size " << colorFormat.cbBufferSize << endl;


		irPixelByteArray = new BYTE[colorFormat.cbBufferSize];

		videoPixels.allocate(colorFormat.dwWidth, colorFormat.dwHeight, OF_IMAGE_GRAYSCALE);
		videoPixelsBack.allocate(colorFormat.dwWidth, colorFormat.dwHeight, OF_IMAGE_GRAYSCALE);

		if(bUseTexture)
		{
			if(bProgrammableRenderer){
				videoTex.allocate(colorFormat.dwWidth, colorFormat.dwHeight, GL_R8);
				videoTex.setRGToRGBASwizzles(true);
			}
			else{
				videoTex.allocate(colorFormat.dwWidth, colorFormat.dwHeight, GL_LUMINANCE);
			}
		}
	} else {
		ofLogError("ofxKinectCommonBridge::open") << "Error opening color stream";
		return false;
	}

	bInitedIR = true;
	return true;
}

bool ofxKinectCommonBridge::initSkeletonStream( bool seated )
{
	if(bStarted){
		ofLogError("ofxKinectCommonBridge::initSkeletonStream") << "Cannot configure once the sensor has already started";
		return false;
	}

	NUI_TRANSFORM_SMOOTH_PARAMETERS p = { 0.5f, 0.1f, 0.5f, 0.1f, 0.1f };
    
    KinectEnableSkeletonStream( hKinect, seated, SkeletonSelectionModeDefault, &p);
    if( KinectStreamStatusError != KinectGetSkeletonStreamStatus(hKinect) ) {
		//cout << " we have skeletons " << endl;

		//vector<Skeleton>::iterator skelIt = skeletons.begin();
		for( int i = 0; i < NUI_SKELETON_COUNT; i++ )
		{
			Skeleton s;
			skeletons.push_back(s);
		}

		bUsingSkeletons = true;
		return true;
	} 

	ofLogError("ofxKinectCommonBridge::initSkeletonStream") << "cannot initialize stream";
	return false;
}

#ifdef KCB_ENABLE_SPEECH
bool ofxKinectCommonBridge::initSpeech()
{

	//KCB_SPEECH_LANGUAGE *lang;
	ULONGLONG interest;
	bool adaptive = false;

	HRESULT hr; 

	// testing
	//string path = "C:\\en-US.grxml";
	//WCHAR file[255];
	
	int sz = MultiByteToWideChar(CP_UTF8, 0, grammarFile.c_str(), -1, NULL, 0);
	WCHAR *file = new WCHAR[sz];
	MultiByteToWideChar(CP_UTF8, 0, grammarFile.c_str(), -1, file, sz);

	KinectEnableSpeech(hKinect, &file[0], NULL, NULL, &adaptive);

	hr = KinectStartSpeech(hKinect);
	if(hr != S_OK)
	{
		ofLogError(" ofxKinectCommonBridge::startSpeech cannot start speech" );
		return false;
	}

	bUsingSpeech = true;
	bInitedSpeech = true;
	return true;
}
#endif

#ifdef KCB_ENABLE_FT
bool ofxKinectCommonBridge::initFaceTracking() {

	// initialize camera params if we don't already have a Kinect
	if( hKinect == 0) 
	{
		hKinect = KinectOpenDefaultSensor();
		createDepthPixels(320, 240);
		createColorPixels(640, 480);
	}

	if( KCB_INVALID_HANDLE == hKinect )
    {
		ofLogError() << "initFaceTracking: KinectOpenDefaultSensor() " << endl;
        // this rarely happens and may be a memory issue typically
        return false;
    }

	// enable face tracking
    HRESULT hr = KinectEnableFaceTracking(hKinect, true);
    
	if(FAILED(hr))
	{
		ofLogError() << "KinectEnableFaceTracking: unable to enable face tracking" << endl;
	} 
	else 
	{
		bUsingFaceTrack = true;
	}

	bUseStreams = false;
    return SUCCEEDED(hr);
}
#endif

//----------------------------------------------------------
bool ofxKinectCommonBridge::start()
{
	if(bStarted){
		ofLogError("ofxKinectCommonBridge::start") << "Stream already started";
		return false;
	}

	if(hKinect == NULL){
		cout << "init sensor" << endl;
		initSensor();
	}

	if(!bInitedColor && bUseStreams)
	{
		initColorStream(640,480);
	}

	if(!bInitedDepth && bUseStreams)
	{
		initDepthStream(320,240);
	}

	HRESULT hr = KinectStartStreams(hKinect);
	if( FAILED(hr) )
	{
		return false;
	}

	startThread(true, false);
	bStarted = true;	

	return true;
}

//----------------------------------------------------------
KCBHANDLE ofxKinectCommonBridge::getHandle() {
	return this->hKinect;
}

//----------------------------------------------------------
INuiSensor & ofxKinectCommonBridge::getNuiSensor() {
	// get the port ID from the simple api
	const WCHAR* wcPortID = KinectGetPortID(hKinect);

	// create an instance of the same sensor
	INuiSensor* nuiSensor = nullptr;
	HRESULT hr = NuiCreateSensorById(wcPortID, &nuiSensor);
	return * nuiSensor;
}

//----------------------------------------------------------
void ofxKinectCommonBridge::stop() {
	if(bStarted){
		waitForThread(true);
		bStarted = false;

		// release these interfaces when done
		if (mapper)
		{
			mapper->Release();
			mapper = nullptr;
		}
		if (nuiSensor)
		{
			nuiSensor->Release();
			nuiSensor = nullptr;
		}

		if(depthPixelsNui){
			delete[] depthPixelsNui;
		}
		
		KinectStopStreams( hKinect );
		KinectCloseSensor( hKinect );

	}
}	

//----------------------------------------------------------
void ofxKinectCommonBridge::threadedFunction(){

	LONGLONG timestamp;
	
	cout << "STARTING THREAD" << endl;

	//JG: DO WE NEED TO BAIL ON THIS LOOP IF THE DEVICE QUITS? 
	//how can we tell?
	while(isThreadRunning()) {

        if( KinectIsDepthFrameReady(hKinect) && SUCCEEDED( KinectGetDepthFrame(hKinect, depthFormat.cbBufferSize, (BYTE*)depthPixelsRawBack.getPixels(), &timestamp) ) )
		{
			bNeedsUpdateDepth = true;
        }

		if(bVideoIsInfrared)
		{
			if(  KinectIsColorFrameReady(hKinect) && SUCCEEDED( KinectGetColorFrame(hKinect, colorFormat.cbBufferSize, irPixelByteArray, &timestamp) ) )
			{
				bNeedsUpdateVideo = true;
				
				for (int i = 0; i < colorFormat.dwWidth * colorFormat.dwHeight; i++)
				{
					videoPixelsBack.getPixels()[i] = reinterpret_cast<USHORT*>(irPixelByteArray)[i] >> 8;
				}
			}
		}
		else
		{
			if( SUCCEEDED( KinectGetColorFrame(hKinect, colorFormat.cbBufferSize, videoPixelsBack.getPixels(), &timestamp) ) )
			{
				bNeedsUpdateVideo = true;
			}
		}

		if(bUsingSkeletons) {
			if( KinectIsSkeletonFrameReady(hKinect) && SUCCEEDED ( KinectGetSkeletonFrame(hKinect, &k4wSkeletons )) ) 
			{
				bNeedsUpdateSkeleton = true;
			}
		}
		#ifdef KCB_ENABLE_SPEECH
		if(bUsingSpeech)
		{
			if(KinectIsSpeechEventReady(hKinect))
			{
				// dispatch an event
				//https://github.com/Traksewt/molecular-control-toolkit/blob/master/Controllers/KinectNativeController/SpeechBasics.cpp
				SPEVENT spevent;
				ULONG fetched;
				HRESULT hr = S_OK;
				KinectGetSpeechEvent(hKinect, &spevent, &fetched);

				// just look for speech events here. can+should be optimized
				while (fetched > 0)
				{
					switch (spevent.eEventId)
					{
						case SPEI_RECOGNITION:
							if (SPET_LPARAM_IS_OBJECT == spevent.elParamType)
							{
								// this is an ISpRecoResult
								ISpRecoResult* result = reinterpret_cast<ISpRecoResult*>(spevent.lParam);
								SPPHRASE* pPhrase = NULL;
                    
								hr = result->GetPhrase(&pPhrase);
								if (SUCCEEDED(hr))
								{
									if ((pPhrase->pProperties != NULL) && (pPhrase->pProperties->pFirstChild != NULL))
									{
										const SPPHRASEPROPERTY* pSemanticTag = pPhrase->pProperties->pFirstChild;
										if (pSemanticTag->SREngineConfidence > speechConfidenceThreshold)
										{
											//updateSpeechData( pSemanticTag );
											bUpdateSpeech = true;
											char pmbbuf[255];
											int size = wcstombs(&pmbbuf[0], pSemanticTag->pszValue, 255);
											speechData.detectedSpeech = pmbbuf;
											speechData.detectedSpeech.resize(size+1);
											speechData.confidence = pSemanticTag->SREngineConfidence; //??
										}
									}
									// necessary?
									::CoTaskMemFree(pPhrase);
								}
							}
							break;
					}

					KinectGetSpeechEvent(hKinect, &spevent, &fetched);
				}
			}
		}
#endif

		/*if(bUsingAudio) {	 // not doing audio quite yet
		}*/
#ifdef KCB_ENABLE_FT

		if(bUsingFaceTrack)
		{
			IFTResult *ftResult;

			if(KinectIsFaceTrackingResultReady(hKinect))
            {
                IFTResult* pResult;

				if (SUCCEEDED(KinectGetFaceTrackingResult(hKinect, &pResult)) && SUCCEEDED(pResult->GetStatus()))
                {
					updateFaceTrackingData(pResult);
					bUpdateFaces = true;
                }
            }
		}
#endif

		//TODO: TILT
		//TODO: ACCEL
		//TODO: FACE
		//TODO: AUDIO
		ofSleepMillis(10);
	}
}