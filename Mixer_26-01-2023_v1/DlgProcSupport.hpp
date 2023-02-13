#pragma once
//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"

//*****************************************************************************
//*                     global
//*****************************************************************************

// mixer capabilities
// A) IAudioAutoGainControl
// B) IAudioBass
// C) IAudioChannelConfig
// D) IAudioInputSelector
// E) IAudioLoudness
// F) IAudioMidRange
// G) IAudioMute
// H) IAudioOutputSelector
// I) IAudioPeakMeter
WCHAR wszBufferNameFunc[128] = { '\0' };
VOID s_ok(const HRESULT& hr
	, const wchar_t* pwszNameFunc
)
{
	if (hr == S_OK)
	{
		swprintf_s(wszBufferNameFunc, 128, L"%s", L"S_OK   ");
	}
	else
	{
		swprintf_s(wszBufferNameFunc, 128, L"%s", L"NOT_OK ");
	}
	wcscat_s(wszBufferNameFunc, 128, pwszNameFunc);
	wcscat_s(wszBufferNameFunc, 128, L"\n");
	OutputDebugString(wszBufferNameFunc);
}

#define PARTID_MASK 0x0000ffff
#//*****************************************************************************
//*                     onWmInitDialog_DlgProc
//*****************************************************************************
BOOL onWmInitDialog_DlgProc(const HINSTANCE& hInst
	, const HWND& hDlg
)
{
	HRESULT hr;

	hr = CoInitializeEx(NULL
		, COINIT_MULTITHREADED
	);
	s_ok(hr, L"CoInitializeEx()");

	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	IMMDeviceEnumerator* pEnumerator = NULL;
	hr = CoCreateInstance(CLSID_MMDeviceEnumerator
		, NULL
		, CLSCTX_ALL
		, IID_IMMDeviceEnumerator
		, (void**)&pEnumerator
	);
	s_ok(hr, L"CoCreateInstance()");

	IMMDevice* pDevice = NULL;
	hr = pEnumerator->GetDefaultAudioEndpoint(eRender
		, eConsole
		, &pDevice
	);
	s_ok(hr, L"GetDefaultAudioEndpoint()");

	const IID IID_IAudioSessionManager2 = __uuidof(IAudioSessionManager2);
	IAudioSessionManager2* pAudioSessionManager2;
	hr = pDevice->Activate(IID_IAudioSessionManager2
		, CLSCTX_ALL
		, NULL
		, (void**)&pAudioSessionManager2
	);
	s_ok(hr, L"Activate()");

	IAudioSessionEnumerator* pAudioSessionEnumerator;
	hr = pAudioSessionManager2->GetSessionEnumerator(&pAudioSessionEnumerator);
	s_ok(hr, L"GetSessionEnumerator()");

	CoUninitialize();

	return EXIT_SUCCESS;
}
/*
https://stackoverflow.com/questions/45534538/how-to-get-system-volume-level-as-a-scalar-from-0-to-100
void getSessions() {
	CoInitialize(NULL);

	IMMDeviceEnumerator *pDeviceEnumerator;
	IMMDevice *pDevice;
	IAudioSessionManager2 *pAudioSessionManager2;
	IAudioSessionEnumerator *pAudioSessionEnumerator;

	CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&pDeviceEnumerator);
	pDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);

	pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void **) &pAudioSessionManager2);
	pAudioSessionManager2->GetSessionEnumerator(&pAudioSessionEnumerator);

	int nSessionCount;
	pAudioSessionEnumerator->GetCount(&nSessionCount);

	while (true) {
		for (int nSessionIndex = 0; nSessionIndex < nSessionCount; nSessionIndex++) {
			IAudioSessionControl *pSessionControl;
			if (FAILED(pAudioSessionEnumerator->GetSession(nSessionIndex, &pSessionControl)))
				continue;

			ISimpleAudioVolume *pSimpleAudioVolume;
			pSessionControl->QueryInterface(&pSimpleAudioVolume);

			float fLevel;

			pSimpleAudioVolume->GetMasterVolume(&fLevel);

			std::cout << "fLevel Value: " << fLevel << std::endl;
		}

		Sleep(1000);
	}

	CoUninitialize();
}
*/
/*
	HRESULT hr;

	hr = CoInitializeEx(NULL
		, COINIT_MULTITHREADED
	);
	s_ok(hr, L"CoInitializeEx()");

	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	IMMDeviceEnumerator* pEnumerator = NULL;

	hr = CoCreateInstance(CLSID_MMDeviceEnumerator
		, NULL
		, CLSCTX_ALL
		, IID_IMMDeviceEnumerator
		, (void**)&pEnumerator
	);
	s_ok(hr, L"CoCreateInstance()");

	IMMDevice* pDevice = NULL;
	hr = pEnumerator->GetDefaultAudioEndpoint(eRender
		, eConsole
		, &pDevice
	);
	s_ok(hr, L"GetDefaultAudioEndpoint()");

	const IID IID_IAudioClient = __uuidof(IAudioClient);
	IAudioClient* pAudioClient = NULL;
	hr = pDevice->Activate(IID_IAudioClient
		, CLSCTX_ALL
		, NULL
		, (void**)&pAudioClient
	);
	s_ok(hr, L"Activate()");

	const IID IID_IDeviceTopology = __uuidof(IDeviceTopology);
	IDeviceTopology* pDevTopoEndpt = NULL;
	hr = pDevice->Activate(IID_IDeviceTopology
		, CLSCTX_ALL
		, NULL
		, (void**)&pDevTopoEndpt
	);
	s_ok(hr, L"Activate()");

	IConnector* pConnEndpt = NULL;
	hr = pDevTopoEndpt->GetConnector(0, &pConnEndpt);
	s_ok(hr, L"GetConnector()");

	IConnector* pConnHWDev = NULL;
	hr = pConnEndpt->GetConnectedTo(&pConnHWDev);
	s_ok(hr, L"GetConnectedTo()");

	IPart* pPart = NULL;
	const IID IID_IPart = __uuidof(IPart);
	hr = pConnHWDev->QueryInterface(IID_IPart
		, (void**)&pPart
	);
	s_ok(hr, L"QueryInterface()");

	// Get the topology object for the adapter device that contains
	// the subunit represented by the IPart interface.
	IDeviceTopology* pTopology = NULL;
	hr = pPart->GetTopologyObject(&pTopology);
	s_ok(hr, L"GetTopologyObject()");

	// Get the device ID string that identifies the adapter device.
	LPWSTR pwszDeviceId = NULL;
	hr = pTopology->GetDeviceId(&pwszDeviceId);
	s_ok(hr, L"GetDeviceId()");

	// Get the IMMDevice interface of the adapter device object.
	IMMDevice* pPnpDevice = NULL;
	hr = pEnumerator->GetDevice(pwszDeviceId, &pPnpDevice);
	s_ok(hr, L"GetDevice()");

	// Activate an IKsControl interface on the adapter device object.
	const IID IID_IKsControl = __uuidof(IKsControl);
	IKsControl* pKsControl = NULL;
	hr = pPnpDevice->Activate(IID_IKsControl
		, CLSCTX_ALL
		, NULL
		, (void**)&pKsControl
	);
	s_ok(hr, L"Activate()");

	// Get the local ID of the subunit (contains the KS node ID).
	UINT localId = 0;
	hr = pPart->GetLocalId(&localId);
	s_ok(hr, L"GetLocalId()");

	KSNODEPROPERTY_AUDIO_CHANNEL ksprop;
	ZeroMemory(&ksprop, sizeof(ksprop));
	ksprop.NodeProperty.Property.Set = KSPROPSETID_Audio;
	ksprop.NodeProperty.Property.Id = KSPROPERTY_AUDIO_BASS_BOOST;
	ksprop.NodeProperty.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
	ksprop.NodeProperty.NodeId = localId & PARTID_MASK;
	ksprop.Channel = 0;

	// Send the property request.to the device driver.
	BOOL bValue = FALSE;
	ULONG valueSize;
	hr = pKsControl->KsProperty(&ksprop.NodeProperty.Property
		, sizeof(ksprop)
		, &bValue
		, sizeof(bValue)
		, &valueSize
	);
	s_ok(hr, L"KsProperty()");

	//IPart* pPartBass = NULL;
	//const IID IID_IAudioBass = __uuidof(IAudioBass);
	//hr = pPartConn->Activate(CLSCTX_ALL
	//	, IID_IAudioBass
	//	, (void**)&pPartBass
	//);
	//s_ok(hr, L"Activate()");
*/
/*
#define PARTID_MASK 0x0000ffff
#define EXIT_ON_ERROR(hres)  \
			  if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
			  if ((punk) != NULL)  \
				{ (punk)->Release(); (punk) = NULL; }

const IID IID_IKsControl = __uuidof(IKsControl);

HRESULT GetBassBoost(IMMDeviceEnumerator *pEnumerator,
					 IPart *pPart, BOOL *pbValue)
{
	HRESULT hr;
	IDeviceTopology *pTopology = NULL;
	IMMDevice *pPnpDevice = NULL;
	IKsControl *pKsControl = NULL;
	LPWSTR pwszDeviceId = NULL;

	if (pEnumerator == NULL || pPart == NULL || pbValue == NULL)
	{
		return E_INVALIDARG;
	}

	// Get the topology object for the adapter device that contains
	// the subunit represented by the IPart interface.
	hr = pPart->GetTopologyObject(&pTopology);
	EXIT_ON_ERROR(hr)

	// Get the device ID string that identifies the adapter device.
	hr = pTopology->GetDeviceId(&pwszDeviceId);
	EXIT_ON_ERROR(hr)

	// Get the IMMDevice interface of the adapter device object.
	hr = pEnumerator->GetDevice(pwszDeviceId, &pPnpDevice);
	EXIT_ON_ERROR(hr)

	// Activate an IKsControl interface on the adapter device object.
	hr = pPnpDevice->Activate(IID_IKsControl, CLSCTX_ALL, NULL, (void**)&pKsControl);
	EXIT_ON_ERROR(hr)

	// Get the local ID of the subunit (contains the KS node ID).
	UINT localId = 0;
	hr = pPart->GetLocalId(&localId);
	EXIT_ON_ERROR(hr)

	KSNODEPROPERTY_AUDIO_CHANNEL ksprop;
	ZeroMemory(&ksprop, sizeof(ksprop));
	ksprop.NodeProperty.Property.Set = KSPROPSETID_Audio;
	ksprop.NodeProperty.Property.Id = KSPROPERTY_AUDIO_BASS_BOOST;
	ksprop.NodeProperty.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
	ksprop.NodeProperty.NodeId = localId & PARTID_MASK;
	ksprop.Channel = 0;

	// Send the property request.to the device driver.
	BOOL bValue = FALSE;
	ULONG valueSize;
	hr = pKsControl->KsProperty(
						 &ksprop.NodeProperty.Property, sizeof(ksprop),
						 &bValue, sizeof(bValue), &valueSize);
	EXIT_ON_ERROR(hr)

	*pbValue = bValue;

Exit:
	SAFE_RELEASE(pTopology)
	SAFE_RELEASE(pPnpDevice)
	SAFE_RELEASE(pKsControl)
	CoTaskMemFree(pwszDeviceId);
	return hr;
}
*/
//*****************************************************************************
//*                     onWmSize_DlgProc
//*****************************************************************************
BOOL onWmSize_DlgProc(const HWND& hDlg
)
{

	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     onWmCommand_DlgProc
//*****************************************************************************
INT_PTR onWmCommand_DlgProc(const HWND& hDlg
	, const WPARAM& wParam
)
{

	return (INT_PTR)FALSE;
}

