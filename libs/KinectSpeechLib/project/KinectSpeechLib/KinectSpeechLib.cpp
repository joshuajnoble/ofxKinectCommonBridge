// KinectSpeechLib.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "KinectSpeechLib.h"


// KinectSpeechLib.cpp : Defines the exported functions for the DLL application.
//

// For speech APIs
// NOTE: To ensure that application compiles and links against correct SAPI versions (from Microsoft Speech
//       SDK), VC++ include and library paths should be configured to list appropriate paths within Microsoft
//       Speech SDK installation directory before listing the default system include and library directories,
//       which might contain a version of SAPI that is not appropriate for use together with Kinect sensor.
#include <sapi.h>
#include <sphelper.h>

#include "stdafx.h"
// For Kinect SDK APIs
#include <NuiApi.h>
#include <memory>

#include <wmcodecdsp.h>
#include <uuids.h> // FORMAT_WaveFormatEx and such
#include <mfapi.h> // IPropertyStore

// For IMediaObject and related interfaces
#include <dmo.h>

// For WAVEFORMATEX
#include <mmreg.h>

// For MMCSS functionality such as AvSetMmThreadCharacteristics
#include <avrt.h>

#include <stack>
#include <queue>
#include <sstream>

#include "KinectAudioStream.h"

class KinectSpeech
{

	//const int eventCount = 1;
	HANDLE hEvents[1];
	HANDLE m_hSpeechEvent;

	// Stream given to speech recognition engine
	ISpStream*              m_pSpeechStream;

	// Speech recognizer
	ISpRecognizer*          m_pSpeechRecognizer;

	// Speech recognizer context
	ISpRecoContext*         m_pSpeechContext;

	// Speech grammar
	ISpRecoGrammar*         m_pSpeechGrammar;

	KinectAudioStream*		m_pKinectAudioStream;

	INuiSensor*				m_Sensor;

	bool runRecognition;


	std::wstring s2ws(const std::string& s)
	{
		int len;
		int slength = (int)s.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}

	HRESULT initializeAudioStream()
	{
		INuiAudioBeam*      pNuiAudioSource = NULL;
		IMediaObject*       pDMO = NULL;
		IPropertyStore*     pPropertyStore = NULL;
		IStream*            pStream = NULL;

		// Get the audio source
		HRESULT hr = m_Sensor->NuiGetAudioSource(&pNuiAudioSource);
		if (SUCCEEDED(hr))
		{
			hr = pNuiAudioSource->QueryInterface(IID_IMediaObject, (void**)&pDMO);

			if (SUCCEEDED(hr))
			{
				hr = pNuiAudioSource->QueryInterface(IID_IPropertyStore, (void**)&pPropertyStore);

				// Set AEC-MicArray DMO system mode. This must be set for the DMO to work properly.
				// Possible values are:
				//   SINGLE_CHANNEL_AEC = 0
				//   OPTIBEAM_ARRAY_ONLY = 2
				//   OPTIBEAM_ARRAY_AND_AEC = 4
				//   SINGLE_CHANNEL_NSAGC = 5
				PROPVARIANT pvSysMode;
				PropVariantInit(&pvSysMode);
				pvSysMode.vt = VT_I4;
				pvSysMode.lVal = (LONG)(2); // Use OPTIBEAM_ARRAY_ONLY setting. Set OPTIBEAM_ARRAY_AND_AEC instead if you expect to have sound playing from speakers.
				pPropertyStore->SetValue(MFPKEY_WMAAECMA_SYSTEM_MODE, pvSysMode);
				PropVariantClear(&pvSysMode);

				// Set DMO output format
				WAVEFORMATEX wfxOut = { AudioFormat, AudioChannels, AudioSamplesPerSecond, AudioAverageBytesPerSecond, AudioBlockAlign, AudioBitsPerSample, 0 };
				DMO_MEDIA_TYPE mt = { 0 };
				MoInitMediaType(&mt, sizeof(WAVEFORMATEX));

				mt.majortype = MEDIATYPE_Audio;
				mt.subtype = MEDIASUBTYPE_PCM;
				mt.lSampleSize = 0;
				mt.bFixedSizeSamples = TRUE;
				mt.bTemporalCompression = FALSE;
				mt.formattype = FORMAT_WaveFormatEx;
				memcpy(mt.pbFormat, &wfxOut, sizeof(WAVEFORMATEX));

				hr = pDMO->SetOutputType(0, &mt, 0);

				if (SUCCEEDED(hr))
				{
					m_pKinectAudioStream = new KinectAudioStream(pDMO);

					hr = m_pKinectAudioStream->QueryInterface(IID_IStream, (void**)&pStream);

					if (SUCCEEDED(hr))
					{
						hr = CoCreateInstance(CLSID_SpStream, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpStream), (void**)&m_pSpeechStream);

						if (SUCCEEDED(hr))
						{
							hr = m_pSpeechStream->SetBaseStream(pStream, SPDFID_WaveFormatEx, &wfxOut);
						}
					}
				}

				MoFreeMediaType(&mt);
			}
		}

		pStream->Release();
		pPropertyStore->Release();
		pDMO->Release();
		pNuiAudioSource->Release();

		return hr;
	}

public:

	// singleton

	static KinectSpeech& getInstance()
	{
		static KinectSpeech instance; // Guaranteed to be destroyed.
		// Instantiated on first use.
		return instance;
	}

	bool initializeSpeechEngine( INuiSensor *sensor )
	{

		if (sensor != NULL)
		{
			m_Sensor = sensor;
			// Initialize the Kinect and specify that we'll be using audio signal
			HRESULT hr = m_Sensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_AUDIO);
			if (FAILED(hr))
			{
				// Some other application is streaming from the same Kinect sensor
				m_Sensor->Release();
				m_Sensor = NULL;
				return false;
			}
		}

		initializeAudioStream();

		ISpObjectToken *pEngineToken = NULL;

		HRESULT hr = CoCreateInstance(CLSID_SpInprocRecognizer, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpRecognizer), (void**)&m_pSpeechRecognizer);

		if (SUCCEEDED(hr))
		{
			m_pSpeechRecognizer->SetInput(m_pSpeechStream, FALSE);
			hr = SpFindBestToken(SPCAT_RECOGNIZERS, L"Language=409;Kinect=True", NULL, &pEngineToken);

			if (SUCCEEDED(hr))
			{
				m_pSpeechRecognizer->SetRecognizer(pEngineToken);
				hr = m_pSpeechRecognizer->CreateRecoContext(&m_pSpeechContext);

				// For long recognition sessions (a few hours or more), it may be beneficial to turn off adaptation of the acoustic model. 
				// This will prevent recognition accuracy from degrading over time.
				//if (SUCCEEDED(hr))
				//{
				//    hr = m_pSpeechRecognizer->SetPropertyNum(L"AdaptationOn", 0);                
				//}
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}

		delete pEngineToken;
		return true;
	}

	HRESULT LoadSpeechGrammar(std::string filename)
	{
		
		std::wstring stemp = s2ws(filename);
		LPCWSTR GrammarFileName = stemp.c_str();

		HRESULT hr = m_pSpeechContext->CreateGrammar(1, &m_pSpeechGrammar);

		if (SUCCEEDED(hr))
		{
			// Populate recognition grammar from file
			hr = m_pSpeechGrammar->LoadCmdFromFile(GrammarFileName, SPLO_STATIC);
		}

		return hr;
	}

	/// <summary>
	/// Start recognizing speech asynchronously.
	/// </summary>
	/// <returns>
	/// <para>S_OK on success, otherwise failure code.</para>
	/// </returns>
	HRESULT StartSpeechRecognition()
	{
		HRESULT hr = m_pKinectAudioStream->StartCapture();

		if (SUCCEEDED(hr))
		{
			// Specify that all top level rules in grammar are now active
			m_pSpeechGrammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);

			// Specify that engine should always be reading audio
			m_pSpeechRecognizer->SetRecoState(SPRST_ACTIVE_ALWAYS);

			// Specify that we're only interested in receiving recognition events
			m_pSpeechContext->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));

			// Ensure that engine is recognizing speech and not in paused state
			hr = m_pSpeechContext->Resume(0);
			if (SUCCEEDED(hr))
			{
				m_hSpeechEvent = m_pSpeechContext->GetNotifyEventHandle();
			}
		}

		return hr;
	}
	
	//void run()
	//{
	//	// Main message loop
	//	while(runRecognition == true)
	//	{
	//		hEvents[0] = m_hSpeechEvent;

	//		// Check to see if we have either a message (by passing in QS_ALLINPUT)
	//		// Or a speech event (hEvents)
	//		MsgWaitForMultipleObjectsEx(1, hEvents, INFINITE, QS_ALLINPUT, MWMO_INPUTAVAILABLE);

	//		// Explicitly check for new speech recognition events since
	//		// MsgWaitForMultipleObjects can return for other reasons
	//		// even though it is signaled.
	//		ProcessSpeech();

	//		/*while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
	//		{
	//			// If a dialog message will be taken care of by the dialog proc
	//			if ((hWndApp != NULL) && IsDialogMessageW(hWndApp, &msg))
	//			{
	//				continue;
	//			}

	//			TranslateMessage(&msg);
	//			DispatchMessageW(&msg);
	//		}*/
	//	}
	//}

	//void stop()
	//{
	//	runRecognition = false;
	//}

	/// <summary>
	/// Process recently triggered speech recognition events.
	/// </summary>
	bool ProcessSpeech(std::string &result, int *confidence)
	{
		bool detectionFlag = false;
		
		const float ConfidenceThreshold = 0.3f;

		SPEVENT curEvent;
		ULONG fetched = 0;
		HRESULT hr = S_OK;

		m_pSpeechContext->GetEvents(1, &curEvent, &fetched);

		// this needs to be threaded, yikes
		while (fetched > 0)
		{
			switch (curEvent.eEventId)
			{
			case SPEI_RECOGNITION:
				if (SPET_LPARAM_IS_OBJECT == curEvent.elParamType)
				{
					// this is an ISpRecoResult
					ISpRecoResult* res = reinterpret_cast<ISpRecoResult*>(curEvent.lParam);
					SPPHRASE* pPhrase = NULL;

					hr = res->GetPhrase(&pPhrase);
					if (SUCCEEDED(hr))
					{
						if ((pPhrase->pProperties != NULL) && (pPhrase->pProperties->pFirstChild != NULL))
						{
							const SPPHRASEPROPERTY* pSemanticTag = pPhrase->pProperties->pFirstChild;
							if (pSemanticTag->SREngineConfidence > ConfidenceThreshold)
							{
								//TurtleAction action = MapSpeechTagToAction(pSemanticTag->pszValue);
								//m_pTurtleController->DoAction(action);
								// we have an action
								// and we can use pSemanticTag->pszValue
								
								size_t origsize = wcslen(pSemanticTag->pszName) + 1;
								char *name = new char[ (origsize * 2) ];
								size_t origTimesTwo = origsize * 2;
								//errno_t wcstombs_s(size_t *pReturnValue,char *mbstr,size_t sizeInBytes,const wchar_t *wcstr,size_t count);
								wcstombs_s(&origTimesTwo, &name[0], origsize, pSemanticTag->pszName, _TRUNCATE);
								
								std::stringstream ss;
								ss << *name;
								
								result = ss.str();
								*confidence = pSemanticTag->Confidence;
								
								return true;

							}
						}
						::CoTaskMemFree(pPhrase);
					}
				}
				break;
			}

			m_pSpeechContext->GetEvents(1, &curEvent, &fetched);
		}

		// if we're here
		result = "";
		*confidence = -1;
		return false;
	}

	private:
		KinectSpeech() {};
		KinectSpeech(KinectSpeech const&);
		void operator=(KinectSpeech const&);

};


bool APIENTRY KinectSpeechInitialize(INuiSensor *sensor) {
	return KinectSpeech::getInstance().initializeSpeechEngine(sensor);
}

HRESULT APIENTRY KinectSpeechLoadGrammar(std::string filename) {
	return KinectSpeech::getInstance().LoadSpeechGrammar(filename);
}

HRESULT APIENTRY KinectSpeechStartRecognition() {
	return KinectSpeech::getInstance().StartSpeechRecognition();
}

//KINECT_SPEECH void KinectSpeechRun() {
//	KinectSpeech::getInstance().run();
//}
//KINECT_SPEECH void KinectSpeechStop() {
//	KinectSpeech::getInstance().stop();
//}

bool APIENTRY KinectSpeechProcess(std::string &result, int *confidence) {
	return KinectSpeech::getInstance().ProcessSpeech(result, confidence);
}