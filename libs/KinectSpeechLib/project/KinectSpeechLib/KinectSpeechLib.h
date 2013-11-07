/*
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the KINECTSPEECHLIB_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// KINECTSPEECHLIB_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

// This class is exported from the KinectSpeechLib.dll
class KINECTSPEECHLIB_API CKinectSpeechLib {
public:
	CKinectSpeechLib(void);
	// TODO: add your methods here.
};

extern KINECTSPEECHLIB_API int nKinectSpeechLib;

*/

#pragma warning(disable : 4996)
#pragma once

#ifdef _WIN32
	#ifdef KINECTSPEECHLIB_EXPORTS
		#define KINECTSPEECHLIB_API __declspec(dllexport)
	#else
		#define KINECTSPEECHLIB_API __declspec(dllimport)
		#pragma comment (lib, "KinectSpeechLib.lib") // be sure to add the .dll/.lib folder to your Linker path
	#endif
#endif

// Windows Header Files:
#include <windows.h>
#include <objbase.h>
#include <string>
#include <NuiApi.h>                // be sure to add Include Dir: $(KINECTSDK10_DIR)inc;$(KINECT_TOOLKIT_DIR)inc;

// exported api's
extern "C"
{
	// this throws 4190 but that's ok http://msdn.microsoft.com/en-us/library/1e02627y.aspx
	//struct speechResult {
	//	std::string result;
	//	int confidence;
	//};

	KINECTSPEECHLIB_API bool APIENTRY KinectSpeechInitialize(INuiSensor *sensor);
	KINECTSPEECHLIB_API HRESULT APIENTRY KinectSpeechLoadGrammar(std::string filename);
	KINECTSPEECHLIB_API HRESULT APIENTRY KinectSpeechStartRecognition();
	KINECTSPEECHLIB_API bool APIENTRY KinectSpeechProcess( std::string &result, int *confidence );
};

