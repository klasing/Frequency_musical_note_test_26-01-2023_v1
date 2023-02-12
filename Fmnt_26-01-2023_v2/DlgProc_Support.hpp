#pragma once
//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"
#include "Fmnt_26-01-2023_v2.h"

//*****************************************************************************
//*                     global
//*****************************************************************************
DWORD g_flags = 0;

//*****************************************************************************
//*                     Note
//*****************************************************************************
class Note
{
public:
	//************************************************************************
	//*                 <<constructor>>
	//************************************************************************
	Note()
	{
		for (int i = 0; i < 128; i++)
		{
			aFreq[i] = pitch_standard_hz
				* std::pow(2.0f, float(i - 69) / 12.0f);
		}
	}
	FLOAT aFreq[128] = { 0 };
	FLOAT pitch_standard_hz = 440.; // Hz
};

//*****************************************************************************
//*                     _Chord
//*****************************************************************************
class _Chord
{
public:
	//************************************************************************
	//*                 <<constructor>>
	//************************************************************************
	_Chord()
	{
		prepare_chord();
	}

	//************************************************************************
	//*                 prepare_chord
	//************************************************************************
	VOID prepare_chord()
	{
	}
private:
	Note oNote;
};

//*****************************************************************************
//*                     MyAudioSource
//*****************************************************************************
class MyAudioSource
{
public:
	//************************************************************************
	//*                 <<constructor>>
	//************************************************************************
	MyAudioSource() :
		format()
		, engine(__rdtsc())
		, float_dist(-1.f, 1.f)
	{}

	//************************************************************************
	//*                 SetFormat
	//************************************************************************
	HRESULT SetFormat(WAVEFORMATEX* pwfx)
	{
		if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
		{
			format = *reinterpret_cast<WAVEFORMATEXTENSIBLE*>(pwfx);
		}

		return S_OK;
	}

	//************************************************************************
	//*                 LoadData
	//************************************************************************
	// the size of an audio frame = nChannels * wBitsPerSample
	HRESULT LoadData(UINT32& numFramesAvailable
		, BYTE* pData
		, DWORD* pFlags
		, LPVOID lpVoid
	)
	{
		const UINT16 formatTag = EXTRACT_WAVEFORMATEX_ID(&format.SubFormat);
		if (formatTag == WAVE_FORMAT_IEEE_FLOAT)
		{
			float* fData = (float*)pData;
			if (*(UINT16*)lpVoid == NOISE)
			{
				for (UINT32 i = 0; i < format.Format.nChannels * numFramesAvailable; i++)
				{
					fData[i] = float_dist(engine);
				}
			}
			if (*(UINT16*)lpVoid == NOTE)
			{
				for (UINT32 i = 0; i < format.Format.nChannels * numFramesAvailable; i++)
				{
					float next_sample = std::sin(phase);
					phase = std::fmod(phase + delta, 2.f * static_cast<float>(M_PI));
					fData[i] = next_sample;
				}
			}
			if (*(UINT16*)lpVoid == SWEEP)
			{
				// this count controls the sweep rate
				if (cSweep == 16)
				{
					// 93 is the upper note index for the sweep
					if (up_down > 0 && index_sweep < 93)
						++index_sweep;
					else
					{
						up_down = -1;
						// 45 is the lower note index for the sweep
						if (up_down < 0 && index_sweep > 45)
							--index_sweep;
						else
							up_down = 1;
					}

					cSweep = 0;
				}
				else
					++cSweep;

				frequency_hz = oNote.aFreq[index_sweep];
				delta = 2.f * frequency_hz * float(M_PI / sample_rate);
				for (UINT32 i = 0; i < format.Format.nChannels * numFramesAvailable; i++)
				{
					float next_sample = std::sin(phase);
					phase = std::fmod(phase + delta, 2.f * static_cast<float>(M_PI));
					fData[i] = next_sample;
				}
			}
			if (*(UINT16*)lpVoid == CHORD)
			{				
				if (change_rate_chord < 128)
				{
					for (UINT32 i = 0; i < format.Format.nChannels * numFramesAvailable; i++)
					{
						float next_sample = std::sin(phase_60) + std::sin(phase_57) + std::sin(phase_53);
						phase_60 = std::fmod(phase_60 + delta_60, 2.f * static_cast<float>(M_PI));
						phase_57 = std::fmod(phase_57 + delta_57, 2.f * static_cast<float>(M_PI));
						phase_53 = std::fmod(phase_53 + delta_53, 2.f * static_cast<float>(M_PI));

						fData[i] = next_sample;
					}
				}
				else
				{
					for (UINT32 i = 0; i < format.Format.nChannels * numFramesAvailable; i++)
					{
						float next_sample = std::sin(phase_62) + std::sin(phase_59) + std::sin(phase_55);
						phase_62 = std::fmod(phase_62 + delta_62, 2.f * static_cast<float>(M_PI));
						phase_59 = std::fmod(phase_59 + delta_59, 2.f * static_cast<float>(M_PI));
						phase_55 = std::fmod(phase_55 + delta_55, 2.f * static_cast<float>(M_PI));

						fData[i] = next_sample;
					}
				}

				++change_rate_chord;
				if (change_rate_chord == 256) change_rate_chord = 0;
			}
			if (*(UINT16*)lpVoid == METRONOME)
			{				
				for (UINT32 i = 0; i < format.Format.nChannels * numFramesAvailable; i++)
				{
					float next_sample = std::sin(metronome_phase);
					metronome_phase = std::fmod(metronome_phase + metronome_delta, 2.f * static_cast<float>(M_PI));
					if (i < 2182 && cSampleRate < 2182)
						fData[i] = next_sample;
					else
						fData[i] = 0.f;
				
					cSampleRate = ++cSampleRate % (int)(0.5 * sample_rate);
				}
			}
			if (*(UINT16*)lpVoid == MELODY)
			{
				OutputDebugString(L"MELODY\n");
			}
		}
		return S_OK;
	}
private:
	WAVEFORMATEXTENSIBLE format;
	std::array<float, 2 * 48'000> a = {};
	std::mt19937_64 engine;
	std::uniform_real_distribution<float> float_dist;

	Note oNote;

	// for note
	float frequency_hz = oNote.aFreq[69]; // 69 = 440.f Hz
	int sample_rate = 48'000;
	float delta = 2.f * frequency_hz * float(M_PI / sample_rate);
	float phase = 0.f;

	// for sweep
	int cSweep = 0;
	int index_sweep = 45;
	int up_down = 1;

	// for chord
	// start with the highest frequency
	float frequency_hz_60 = oNote.aFreq[60];
	float delta_60 = 2.f * frequency_hz_60 * float(M_PI / sample_rate);
	float phase_60 = 0.f;

	// the delta for the lower frequencies are a fraction of the higher frequency delta
	float frequency_hz_57 = oNote.aFreq[57];
	float delta_57 = delta_60 * (frequency_hz_57 / frequency_hz_60);
	float phase_57 = 0.f;

	float frequency_hz_53 = oNote.aFreq[53]; //F3
	float delta_53 = delta_60 * (frequency_hz_53 / frequency_hz_60);
	float phase_53 = 0.f;

	// for another chord
	// start with the highest frequency
	float frequency_hz_62 = oNote.aFreq[62];
	float delta_62 = 2.f * frequency_hz_62 * float(M_PI / sample_rate);
	float phase_62 = 0.f;

	// the delta for the lower frequencies are a fraction of the higher frequency delta
	float frequency_hz_59 = oNote.aFreq[59];
	float delta_59 = delta_59 * (frequency_hz_59 / frequency_hz_62);
	float phase_59 = 0.f;

	float frequency_hz_55 = oNote.aFreq[55]; //F3
	float delta_55 = delta_55 * (frequency_hz_55 / frequency_hz_62);
	float phase_55 = 0.f;

	int change_rate_chord = 0;

	// the metronome ticks with oNote.aFreq[45] = 220 Hz
	float metronome_frequency_hz_60 = oNote.aFreq[60];
	float metronome_delta = 2.f * metronome_frequency_hz_60 * float(M_PI / sample_rate);
	float metronome_phase = 0.f;
	
	int cSampleRate = 0;
};

//*****************************************************************************
//*                     special purpose define
//*****************************************************************************
// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC 10'000'000
#define REFTIMES_PER_MILLISEC 10'000

#define EXIT_ON_ERROR(hres) \
	if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk) \
	if ((punk) != NULL) \
	  { (punk)->Release(); (punk) = NULL; }
//*****************************************************************************
//*                     playAudioStream
//*****************************************************************************
DWORD WINAPI playAudioStream(LPVOID lpVoid)
{
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioClient = __uuidof(IAudioClient);
	const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

	HRESULT hr;
	REFERENCE_TIME hnsRequestDuration = REFTIMES_PER_SEC;
	REFERENCE_TIME hnsActualDuration;
	UINT32 numFramesPadding = 0;
	UINT32 numFramesAvailable = 0;
	UINT32 bufferFrameCount = 0;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* pDevice = NULL;
	IAudioClient* pAudioClient = NULL;
	WAVEFORMATEX* pwfx = NULL;
	IAudioRenderClient* pRenderClient = NULL;
	BYTE* pData;

	MyAudioSource* pMySource = new MyAudioSource;

	hr = CoInitialize(nullptr);
	hr = CoCreateInstance(CLSID_MMDeviceEnumerator
		, NULL
		, CLSCTX_ALL
		, IID_IMMDeviceEnumerator
		, (void**)&pEnumerator
	);
	EXIT_ON_ERROR(hr);

	hr = pEnumerator->GetDefaultAudioEndpoint(eRender
		, eConsole
		, &pDevice
	);
	EXIT_ON_ERROR(hr);

	hr = pDevice->Activate(IID_IAudioClient
		, CLSCTX_ALL
		, NULL
		, (void**)&pAudioClient
	);
	EXIT_ON_ERROR(hr);

	hr = pAudioClient->GetMixFormat(&pwfx);
	EXIT_ON_ERROR(hr);

	hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED
		, 0
		, hnsRequestDuration
		, 0
		, pwfx
		, NULL
	);
	EXIT_ON_ERROR(hr);

	hr = pMySource->SetFormat(pwfx);
	EXIT_ON_ERROR(hr);

	// tell the audio source which format to use
	hr = pAudioClient->GetBufferSize(&bufferFrameCount);
	EXIT_ON_ERROR(hr);

	// get the actual size of the allocated buffer
	hr = pAudioClient->GetService(IID_IAudioRenderClient
		, (void**)&pRenderClient
	);
	EXIT_ON_ERROR(hr);

	// grab the entire buffer for the initial fill operation
	hr = pRenderClient->GetBuffer(bufferFrameCount
		, &pData
	);
	EXIT_ON_ERROR(hr);

	// load the initial data into the shared buffer
	hr = pMySource->LoadData(bufferFrameCount
		, pData
		, &g_flags
		, lpVoid
	);
	EXIT_ON_ERROR(hr);

	hr = pRenderClient->ReleaseBuffer(bufferFrameCount
		, g_flags
	);
	EXIT_ON_ERROR(hr);

	// calculate the actual duration of the allocated buffer
	hnsActualDuration = REFTIMES_PER_SEC
		* bufferFrameCount / pwfx->nSamplesPerSec;

	// start playing
	hr = pAudioClient->Start();
	EXIT_ON_ERROR(hr);

	// each loop fills about half of the shared buffer
	while (g_flags != AUDCLNT_BUFFERFLAGS_SILENT)
	{
		// sleep for half the buffer duration
		Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));

		// see how much buffer space is available
		hr = pAudioClient->GetCurrentPadding(&numFramesPadding);
		EXIT_ON_ERROR(hr);

		numFramesAvailable = bufferFrameCount - numFramesPadding;

		// grab all the available space in the shared buffer
		hr = pRenderClient->GetBuffer(numFramesAvailable
			, &pData
		);
		EXIT_ON_ERROR(hr);

		// get next 1/2-second of data from the audio source
		hr = pMySource->LoadData(numFramesAvailable
			, pData
			, &g_flags
			, lpVoid
		);
		EXIT_ON_ERROR(hr);

		hr = pRenderClient->ReleaseBuffer(numFramesAvailable
			, g_flags
		);
		EXIT_ON_ERROR(hr);
	}

	// wait for the last data in buffer to play before stopping
	// time unit in millisecond
	// don't wait too long
	Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 64));

	hr = pAudioClient->Stop();
	EXIT_ON_ERROR(hr);

Exit:
	// cleanup
	CoTaskMemFree(pwfx);
	SAFE_RELEASE(pEnumerator);
	SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pAudioClient);
	SAFE_RELEASE(pRenderClient);

	return 0;
}

//*****************************************************************************
//*                     start_play
//*****************************************************************************
BOOL start_play(const UINT16& define)
{
	// set g_flags to 0
	g_flags = 0;

	CreateThread(NULL
		, 0
		, playAudioStream
		, (LPVOID)new UINT16(define)
		, 0
		, NULL
	);

	return EXIT_SUCCESS;
}
//*****************************************************************************
//*                     onWmInitDialog_DlgProc
//*****************************************************************************
BOOL onWmInitDialog_DlgProc(const HINSTANCE& hInst
	, const HWND& hDlg
)
{
	// check the first radiobutton
	SendMessage(GetDlgItem(hDlg, IDC_NOISE)
		, BM_SETCHECK
		, (WPARAM)BST_CHECKED
		, (LPARAM)0
	);
	return EXIT_SUCCESS;
}

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
	switch (LOWORD(wParam))
	{
	case IDC_START:
	{
		WCHAR pwszBuffer[64] = { '\0' };
		HWND hWnd = GetDlgItem(hDlg, IDC_START);
		SendMessage(hWnd
			, WM_GETTEXT
			, (WPARAM)64
			, (LPARAM)pwszBuffer
		);
		if (wcscmp(pwszBuffer, L"Start") == 0)
		{
			SendMessage(hWnd
				, WM_SETTEXT
				, (WPARAM)0
				, (LPARAM)L"Stop"
			);
			if (SendMessage(GetDlgItem(hDlg, IDC_NOISE)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				start_play(NOISE);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_NOTE)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				start_play(NOTE);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_SWEEP)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				start_play(SWEEP);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_CHORD)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				start_play(CHORD);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_METRONOME)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				start_play(METRONOME);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_MELODY)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				start_play(MELODY);
			}
		}
		if (wcscmp(pwszBuffer, L"Stop") == 0)
		{
			SendMessage(hWnd
				, WM_SETTEXT
				, (WPARAM)0
				, (LPARAM)L"Start"
			);
			// stop func playAudioStream() and let the thread die
			g_flags = AUDCLNT_BUFFERFLAGS_SILENT;
		}
		return (INT_PTR)TRUE;
	} // eof IDC_START
	} // eof wsitch
	return (INT_PTR)FALSE;
}


