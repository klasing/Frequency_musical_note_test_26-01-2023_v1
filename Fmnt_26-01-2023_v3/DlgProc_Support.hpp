#pragma once
//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"
#include "Fmnt_26-01-2023_v3.h"

//*****************************************************************************
//*                     special purpose define
//*****************************************************************************
#define PITCH_STANDARD_HZ	440.f
#define SAMPLE_RATE			48'000

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
			aFreq[i] = PITCH_STANDARD_HZ
				* std::pow(2.0f, float(i - 69) / 12.0f);
		}
	}
	FLOAT aFreq[128] = { 0 };
};

//*****************************************************************************
//*                     global
//*****************************************************************************
Note g_oNote;

//*****************************************************************************
//*                     MyAudioSource
//*****************************************************************************
class MyAudioSource
{
public:
	//************************************************************************
	//*                 <<constructor>>
	//************************************************************************
	MyAudioSource() : format()
		, engine(__rdtsc())
		, float_dist(-1.f, 1.f)
	{ }

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
		, UINT16* play_item
	)
	{
		const UINT16 formatTag = EXTRACT_WAVEFORMATEX_ID(&format.SubFormat);
		if (formatTag == WAVE_FORMAT_IEEE_FLOAT)
		{
			float* fData = (float*)pData;
			switch (*play_item)
			{
			case METRONOME:
			{
				break;
			} // eof METRONOME
			} // eof switch
		}

		return S_OK;
	}

	//************************************************************************
	//*                 initMetronome
	//************************************************************************
	VOID initMetronome(const UINT16& bpm)
	{
		// at this interval we need to generate a metronome tick
		sample_interval_bpm = 60 * SAMPLE_RATE / bpm;
	}

private:
	WAVEFORMATEXTENSIBLE format;
	std::array<float, 2 * 48'000> a = {};
	std::mt19937_64 engine;
	std::uniform_real_distribution<float> float_dist;

	// the metronome ticks with oNote.aFreq[57] = 220 Hz
	float metronome_frequency_hz = g_oNote.aFreq[57];
	float metronome_delta = 2.f * metronome_frequency_hz * float(M_PI / SAMPLE_RATE);
	float metronome_phase = 0.f;

	UINT16 sample_interval_bpm = 0;
	UINT16 sample_tick_bpm = SAMPLE_RATE / metronome_frequency_hz;
};

//*****************************************************************************
//*                     struct
//*****************************************************************************
using define_code = UINT16;
typedef struct tagPARAM
{
	define_code _play_item;
	UINT16 _bpm;
} PARAM, * PPARAM;

//*****************************************************************************
//*                     special purpose global
//*****************************************************************************
const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

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
	HRESULT hr;
	REFERENCE_TIME hnsRequestDuration = REFTIMES_PER_SEC;
	REFERENCE_TIME hnsActualDuration;
	UINT32 numFramesPadding = 0;
	UINT32 numFramesAvailable = 0;
	UINT32 bufferFrameCount = 0;
	WAVEFORMATEX* pwfx = NULL;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* pDevice = NULL;
	IAudioClient* pAudioClient = NULL;
	IAudioRenderClient* pRenderClient = NULL;
	BYTE* pData;
	DWORD flags = 0;
	PPARAM lpParameter = static_cast<PPARAM>(lpVoid);
	UINT16* play_item = &lpParameter->_play_item;

	std::unique_ptr<MyAudioSource> pMySource(new MyAudioSource);
	if (*play_item == METRONOME)
	{
		pMySource->initMetronome(lpParameter->_bpm);
	}

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
		, &flags
		, play_item
	);
	EXIT_ON_ERROR(hr);

	hr = pRenderClient->ReleaseBuffer(bufferFrameCount
		, flags
	);
	EXIT_ON_ERROR(hr);

	// calculate the actual duration of the allocated buffer
	hnsActualDuration = REFTIMES_PER_SEC
		* bufferFrameCount / pwfx->nSamplesPerSec;

	// start playing
	hr = pAudioClient->Start();
	EXIT_ON_ERROR(hr);

	// each loop fills about half of the shared buffer
	while (flags != AUDCLNT_BUFFERFLAGS_SILENT)
	{
	}

Exit:
	// cleanup
	CoTaskMemFree(pwfx);
	SAFE_RELEASE(pEnumerator);
	SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pAudioClient);
	SAFE_RELEASE(pRenderClient);
	// func CoUninitialize() is called when WndProc
	// is handling the WM_NCDESTROY message

	return 0;
}

//*****************************************************************************
//*                     start_play
//*****************************************************************************
using define_code = UINT16;
BOOL start_play(const define_code& play_item
	, const UINT16& bpm
)
{
	PPARAM lpParameter = new PARAM;
	lpParameter->_play_item = play_item;
	lpParameter->_bpm = bpm;

	CreateThread(NULL
		, 0
		, playAudioStream
		, (LPVOID)lpParameter
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
	// add content to the combobox IDC_CB_BPM
	for (auto i = 0; i <= 42; ++i)
	{
		// the available range is from 60 bpm to 480 bpm
		// 480 = 60 + 10 * 42
		SendMessage(GetDlgItem(hDlg, IDC_CB_BPM)
			, CB_ADDSTRING
			, (WPARAM)0
			, (LPARAM)std::to_wstring((int)60 + i * 10).c_str()
		);
	}
	// check the fifth//first// radiobutton
	SendMessage(GetDlgItem(hDlg, IDC_METRONOME)
		, BM_SETCHECK
		, (WPARAM)BST_CHECKED
		, (LPARAM)0
	);
	// set first list item as current selection
	// in the combobox IDC_CB_BPM
	SendMessage(GetDlgItem(hDlg, IDC_CB_BPM)
		, CB_SETCURSEL
		, (WPARAM)0
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
			// change text on button
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
				//start_play(NOISE);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_NOTE)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				//start_play(NOTE);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_SWEEP)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				//start_play(SWEEP);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_CHORD)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				//start_play(CHORD);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_METRONOME)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				WCHAR wszBufferBpm[8] = { '\0' };;
				SendMessage(GetDlgItem(hDlg, IDC_CB_BPM)
					, WM_GETTEXT
					, (WPARAM)8
					, (LPARAM)wszBufferBpm
				);
				start_play(METRONOME
					, _wtoi(wszBufferBpm)
				);
				//CALLBACK_STRUCT callback_struct;
				//callback_struct.hWnd = hDlg;
				//callback_struct.bpm = static_cast<uint16_t>(
				//	SendMessage(GetDlgItem(hDlg, IDC_CB_BPM)
				//		, CB_GETCURSEL
				//		, (WPARAM)0
				//		, (LPARAM)0));
				//// 60 bpm ... 480 bpm
				//start_play(METRONOME
				//	, callback_struct
				//);

				// 60 bpm ... 480 bpm
				//start_play(hDlg
				//	, METRONOME
				//	, SendMessage(GetDlgItem(hDlg, IDC_CB_BPM)
				//		, CB_GETCURSEL
				//		, (WPARAM)0
				//		, (LPARAM)0)
				//);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_MELODY)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				//start_play(MELODY);
			}
		}
		if (wcscmp(pwszBuffer, L"Stop") == 0)
		{
			SendMessage(hWnd
				, WM_SETTEXT
				, (WPARAM)0
				, (LPARAM)L"Start"
			);
			// kill timer
			//KillTimer(hDlg, IDT_BPM_TIM);
			// stop func playAudioStream() and let the thread die
			//g_flags = AUDCLNT_BUFFERFLAGS_SILENT;
		}
		return (INT_PTR)TRUE;
	} // eof IDC_START
	} // eof wsitch
	return (INT_PTR)FALSE;
}

/*
/////////////////////// waste ////////////////////////////////////////////////
//****************************************************************************
//*                     define
//****************************************************************************
#define SECOND_IN_MILLISECOND	60'000
#define NCHANNEL				2
#define SAMPLE_RATE				48'000
#define PITCH_STANDARD_HZ		440.0

//****************************************************************************
//*                     struct
//****************************************************************************
typedef struct tagCALLBACK_STRUCT
{
	HWND hWnd;
	uint16_t bpm;
} CALLBACK_STRUCT, *PCALLBACK_STRUCT;

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
			aFreq[i] = PITCH_STANDARD_HZ
				* std::pow(2.0f, float(i - 69) / 12.0f);
		}
	}
	FLOAT aFreq[128] = { 0 };
};

//****************************************************************************
//*                     global
//****************************************************************************
//std::array<float, NCHANNEL * SAMPLE_RATE> g_audio_buffer = {};
Note g_oNote;

//****************************************************************************
//*                     callback_bpm_tim
//****************************************************************************
void callback_bpm_tim()
{
	OutputDebugString(L"callback_bpm_tim()\n");

	//Beep(g_oNote.aFreq[45], 5);

	// set buffer
	//for (uint32_t i = 0; i < NCHANNEL * SAMPLE_RATE; ++i)
	//{
	//	if (i < 655)
	//	{
	//		// the metronome ticks with oNote.aFreq[45] = 220 Hz
	//		// x = 3 * 48'000 / 220 = 654,545
	//		// place roughly 3 operiods of tick sound into buffer
	//	}
	//}
}

//*****************************************************************************
//*                     produce_sound
//*
//* this thread produces, for now, only one sound; a metronome sound
//*
//*****************************************************************************
DWORD WINAPI produce_sound(LPVOID lpVoid)
{
	// the metronome ticks with oNote.aFreq[57] = 220 Hz
//	//for (UINT32 i = 0; i < format.Format.nChannels * numFramesAvailable; i++)
	//{
	//	float next_sample = std::sin(metronome_phase);
	//	metronome_phase = std::fmod(metronome_phase + metronome_delta, 2.f * static_cast<float>(M_PI));
	//	if (i < 2182 && cSampleRate < 2182)
	//		fData[i] = next_sample;
	//	else
	//		fData[i] = 0.f;
	//}
	
	//CALLBACK_STRUCT callback_struct = *static_cast<PCALLBACK_STRUCT>(lpVoid);
	//unsigned int bpm = 60 + *static_cast<uint16_t*>(lpVoid) * 10;

	// create timer
	//SetTimer(callback_struct.hWnd
	//	, IDT_BPM_TIM
	//	, callback_struct.bpm//SECOND_IN_MILLISECOND / bpm
	//	, (TIMERPROC)callback_bpm_tim
	//);

	return 0;
}

//*****************************************************************************
//*                     consume_sound
//*****************************************************************************
DWORD WINAPI consume_sound(LPVOID lpVoid)
{
	return 0;
}

//*****************************************************************************
//*                     start_play
//*****************************************************************************
using define_code = UINT16;
BOOL start_play(const define_code& play_item
	, CALLBACK_STRUCT& callback_struct
)
//using define_code = UINT16;
//BOOL start_play(const HWND& hDlg
//	, const define_code& play_item
//	, const uint16_t& getcursel_bpm
//)
{
	// bpm is transformed into the time in millisecond
	// in which the metronome is triggered to tick
	callback_struct.bpm = SECOND_IN_MILLISECOND / (60 + callback_struct.bpm * 10);

	//produce_sound(nullptr);
	// create timer
	SetTimer(callback_struct.hWnd
		, IDT_BPM_TIM
		, callback_struct.bpm
		, (TIMERPROC)callback_bpm_tim
	);
	// create producer thread, produces the sound
	//CreateThread(NULL
	//	, 0
	//	, produce_sound
	//	, (LPVOID)&getcursel_bpm
	//	, 0
	//	, NULL
	//);
	// create consumer thread, plays the sound
	//CreateThread(NULL
	//	, 0
	//	, consume_sound
	//	, (LPVOID)nullptr
	//	, 0
	//	, NULL
	//);

	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     onWmInitDialog_DlgProc
//*****************************************************************************
BOOL onWmInitDialog_DlgProc(const HINSTANCE& hInst
	, const HWND& hDlg
)
{
	// add content to the combobox IDC_CB_BPM
	for (auto i = 0; i <= 42; ++i)
	{
		// the available range is from 60 bpm to 480 bpm
		// 480 = 60 + 10 * 42
		SendMessage(GetDlgItem(hDlg, IDC_CB_BPM)
			, CB_ADDSTRING
			, (WPARAM)0
			, (LPARAM)std::to_wstring((int)60 + i * 10).c_str()
		);
	}
	// check the fifth//first// radiobutton
	SendMessage(GetDlgItem(hDlg, IDC_METRONOME)
		, BM_SETCHECK
		, (WPARAM)BST_CHECKED
		, (LPARAM)0
	);
	// set first list item as current selection
	// in the combobox IDC_CB_BPM
	SendMessage(GetDlgItem(hDlg, IDC_CB_BPM)
		, CB_SETCURSEL
		, (WPARAM)0
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
			// change text on button
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
				//start_play(NOISE);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_NOTE)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				//start_play(NOTE);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_SWEEP)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				//start_play(SWEEP);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_CHORD)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				//start_play(CHORD);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_METRONOME)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				CALLBACK_STRUCT callback_struct;
				callback_struct.hWnd = hDlg;
				callback_struct.bpm = static_cast<uint16_t>(
					SendMessage(GetDlgItem(hDlg, IDC_CB_BPM)
					, CB_GETCURSEL
					, (WPARAM)0
					, (LPARAM)0));
				// 60 bpm ... 480 bpm
				start_play(METRONOME
					, callback_struct
				);

				// 60 bpm ... 480 bpm
				//start_play(hDlg
				//	, METRONOME
				//	, SendMessage(GetDlgItem(hDlg, IDC_CB_BPM)
				//		, CB_GETCURSEL
				//		, (WPARAM)0
				//		, (LPARAM)0)
				//);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_MELODY)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				//start_play(MELODY);
			}
		}
		if (wcscmp(pwszBuffer, L"Stop") == 0)
		{
			SendMessage(hWnd
				, WM_SETTEXT
				, (WPARAM)0
				, (LPARAM)L"Start"
			);
			// kill timer
			KillTimer(hDlg, IDT_BPM_TIM);
			// stop func playAudioStream() and let the thread die
			//g_flags = AUDCLNT_BUFFERFLAGS_SILENT;
		}
		return (INT_PTR)TRUE;
	} // eof IDC_START
	} // eof wsitch
	return (INT_PTR)FALSE;
}
*/
