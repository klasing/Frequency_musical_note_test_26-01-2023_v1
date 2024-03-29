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

#define BUFFER_MAX			64

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
DWORD g_flags = 0;

UINT16 g_play_item = 0;
float g_frequency_hz = 0.f;
int g_idx_freq_sweep_lo = 0;
int g_idx_freq_sweep_hi = 0;

std::vector<std::vector<float>> g_chord;
int g_idx_chord = 0;

int g_bpm = 0;

float g_left_volume = 1.f;
float g_right_volume = 1.f;

//*****************************************************************************
//*                     MyAudioSource
//*****************************************************************************
class MyAudioSource
{
public:
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
	)
	{
		const UINT16 formatTag = EXTRACT_WAVEFORMATEX_ID(&format.SubFormat);
		if (formatTag == WAVE_FORMAT_IEEE_FLOAT)
		{
			float* fData = (float*)pData;
			switch (g_play_item)
			{
			case NOISE:
			{
				for (UINT32 i = 0; i < format.Format.nChannels * numFramesAvailable; i++)
				{
					if (i % 2 == 0)
						fData[i] = g_left_volume * float_dist(engine);
					else
						fData[i] = g_right_volume * float_dist(engine);
				}
				break;
			} // eof NOISE
			case NOTE:
			{
				for (UINT32 i = 0; i < format.Format.nChannels * numFramesAvailable; i++)
				{
					float next_sample = std::sin(phase);
					phase = std::fmod(phase + delta, 2.f * static_cast<float>(M_PI));
					if (i % 2 == 0)
						fData[i] = g_left_volume * next_sample;
					else
						fData[i] = g_right_volume * next_sample;
				}
				break;
			} // eof NOTE
			case SWEEP:
			{
				// this count controls the sweep rate
				if (cSweep == sweep_rate)
				{
					// upper note index for the sweep
					if (up_down > 0 && index_sweep < g_idx_freq_sweep_hi)
						++index_sweep;
					else
					{
						up_down = -1;
						// lower note index for the sweep
						if (up_down < 0 && index_sweep > g_idx_freq_sweep_lo)
							--index_sweep;
						else
							up_down = 1;
					}

					cSweep = 0;
				}
				else
					++cSweep;

				sweep_freq = g_oNote.aFreq[index_sweep];
				delta = 2.f * sweep_freq * float(M_PI / SAMPLE_RATE);
				for (UINT32 i = 0; i < format.Format.nChannels * numFramesAvailable; i++)
				{
					float next_sample = std::sin(phase);
					phase = std::fmod(phase + delta, 2.f * static_cast<float>(M_PI));
					if (i % 2 == 0)
						fData[i] = g_left_volume * next_sample;
					else
						fData[i] = g_right_volume * next_sample;
				}

				break;
			} // eof SWEEP
			case CHORD:
			{
				for (UINT32 i = 0; i < format.Format.nChannels * numFramesAvailable; i++)
				{
					float next_sample = std::sin(phase3) 
						+ std::sin(phase2) 
						+ std::sin(phase1);
					phase3 = std::fmod(phase3 + delta3, 2.f * static_cast<float>(M_PI));
					phase2 = std::fmod(phase2 + delta2, 2.f * static_cast<float>(M_PI));
					phase1 = std::fmod(phase1 + delta1, 2.f * static_cast<float>(M_PI));

					if (i % 2 == 0)
						fData[i] = g_left_volume * next_sample;
					else
						fData[i] = g_right_volume * next_sample;
				}
				break;
			} // eof CHORD
			case METRONOME:
			{
				for (UINT32 i = 0; i < format.Format.nChannels * numFramesAvailable; i++)
				{
					float next_sample = std::sin(phase_metronome);
					phase_metronome = std::fmod(
						phase_metronome + delta_metronome, 2.f * static_cast<float>(M_PI));

					if (i < PERIOD_ONE_TICK && cSample < PERIOD_ONE_TICK)
					{
						if (i % 2 == 0)
							fData[i] = g_left_volume * next_sample;
						else
							fData[i] = g_right_volume * next_sample;
					}
					else
						fData[i] = 0.f;

					cSample = ++cSample 
						% (int)(2 * SAMPLE_RATE * (60. / (float)g_bpm));
				}
				break;
			} // eof METRONOME
			} // eof switch
		}
		return S_OK;
	}

	//************************************************************************
	//*                 init
	//************************************************************************
	VOID init()
	{
		switch (g_play_item)
		{
		case NOTE:
		{
			delta = 2.f * g_frequency_hz * float(M_PI / SAMPLE_RATE);
			phase = 0.f;
			break;
		} // eof NOTE
		case SWEEP:
		{
			index_sweep = g_idx_freq_sweep_lo;
			break;
		} // eof SWEEP
		case CHORD:
		{
			// for now; only for a three note chord
			chord_freq3 = g_chord[g_idx_chord][0];
			delta3 = 2.f * chord_freq3 * float(M_PI / SAMPLE_RATE);
			phase3 = 0.f;

			// the delta for the lower frequencies
			// are a fraction of the highest frequency delta
			chord_freq2 = g_chord[g_idx_chord][1];
			delta2 = delta3 * (chord_freq2 / chord_freq3);
			phase2 = 0.f;

			chord_freq1 = g_chord[g_idx_chord][2];
			delta1 = delta3 * (chord_freq1 / chord_freq3);
			phase1 = 0.f;

			break;
		} // eof CHORD
		case METRONOME:
		{
			break;
		} // eof METRONOME
		} // eof switch
	}
private:
	WAVEFORMATEXTENSIBLE format;
	// NOISE
	std::mt19937_64 engine;
	std::uniform_real_distribution<float> float_dist;
	// NOTE
	float delta = 0.f;
	float phase = 0.f;
	// SWEEP
	int sweep_rate = 16;
	int cSweep = 0;
	int index_sweep = 0;
	int up_down = 1;
	float sweep_freq = 0.f;
	// CHORD
	float delta3 = 0.f, delta2 = 0.f, delta1 = 0.f;
	float phase3 = 0.f, phase2 = 0.f, phase1 = 0.f;
	float chord_freq3 = 0.f, chord_freq2 = 0.f, chord_freq1 = 0.f;
	// METRONOME
	float freq_metronome = g_oNote.aFreq[45];
	float delta_metronome = 2.f * freq_metronome * float(M_PI / SAMPLE_RATE);
	float phase_metronome = 0.f;
	int cSample = 0;
	const int PERIOD_ONE_TICK = 9091;
};

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

	std::unique_ptr<MyAudioSource> pMySource = std::unique_ptr<MyAudioSource>
		(new MyAudioSource);
	if (g_play_item == NOTE
		|| g_play_item == SWEEP
		|| g_play_item == CHORD
		|| g_play_item == METRONOME) pMySource->init();

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
	// func CoUninitialize() is called when WndProc
	// is handling the WM_NCDESTROY message

	return 0;
}

//*****************************************************************************
//*                     start_play
//*****************************************************************************
using define_code = UINT16;
BOOL start_play(const define_code& play_item)
{
	OutputDebugString(L"start_play()\n");

	// reset flag to enable playing
	g_flags = 0;

	// set global to convey into thread space
	g_play_item = play_item;

	CreateThread(NULL
		, 0
		, playAudioStream
		, (LPVOID)nullptr
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
	WCHAR wszBuffer[16] = { '\0' };
	for (auto i = 0; i < 128; ++i)
	{
		swprintf_s(wszBuffer, (size_t)16, L"%3d - %6.1f", i, g_oNote.aFreq[i]);
		// 1) add content to the combobox IDC_CB_NOTE
		SendMessage(GetDlgItem(hDlg, IDC_CB_NOTE)
			, CB_ADDSTRING
			, (WPARAM)0
			, (LPARAM)wszBuffer
		);
		// 2) add content to the combobox IDC_CB_SWEEP_LO
		SendMessage(GetDlgItem(hDlg, IDC_CB_SWEEP_LO)
			, CB_ADDSTRING
			, (WPARAM)0
			, (LPARAM)wszBuffer
		);
		// 3) add content to the combobox IDC_CB_SWEEP_HI
		SendMessage(GetDlgItem(hDlg, IDC_CB_SWEEP_HI)
			, CB_ADDSTRING
			, (WPARAM)0
			, (LPARAM)wszBuffer
		);
	}
	// 1) set 440.0 Hz (A4) list item as current selection
	// in the combobox IDC_CB_NOTE
	SendMessage(GetDlgItem(hDlg, IDC_CB_NOTE)
		, CB_SETCURSEL
		, (WPARAM)69
		, (LPARAM)0
	);
	// 2) set 110.0 Hz (A2) list item as current selection
	// in the combobox IDC_CB_SWEEP_LO
	SendMessage(GetDlgItem(hDlg, IDC_CB_SWEEP_LO)
		, CB_SETCURSEL
		, (WPARAM)45
		, (LPARAM)0
	);
	// 3) set 1760.0 Hz (A6) list item as current selection
	// in the combobox IDC_CB_SWEEP_HI
	SendMessage(GetDlgItem(hDlg, IDC_CB_SWEEP_HI)
		, CB_SETCURSEL
		, (WPARAM)93
		, (LPARAM)0
	);

	// add content to the combobox IDC_CB_CHORD
	SendMessage(GetDlgItem(hDlg, IDC_CB_CHORD)
		, CB_ADDSTRING
		, (WPARAM)0
		, (LPARAM)L"one"
	);
	SendMessage(GetDlgItem(hDlg, IDC_CB_CHORD)
		, CB_ADDSTRING
		, (WPARAM)0
		, (LPARAM)L"two"
	);
	// set first list item as current selection
	// in the combobox IDC_CB_CHORD
	SendMessage(GetDlgItem(hDlg, IDC_CB_CHORD)
		, CB_SETCURSEL
		, (WPARAM)0 // list item index
		, (LPARAM)0
	);

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
	// set first list item as current selection
	// in the combobox IDC_CB_BPM
	SendMessage(GetDlgItem(hDlg, IDC_CB_BPM)
		, CB_SETCURSEL
		, (WPARAM)0
		, (LPARAM)0
	);
	// check the fifth//first// radiobutton
	SendMessage(GetDlgItem(hDlg, IDC_METRONOME)
		, BM_SETCHECK
		, (WPARAM)BST_CHECKED
		, (LPARAM)0
	);

	// set trackbar control IDC_SLIDER_LVOLUME to max value
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER_LVOLUME)
		, TBM_SETPOS
		, (WPARAM)TRUE
		, (LPARAM)100
	);
	// set trackbar control IDC_SLIDER_RVOLUME to max value
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER_RVOLUME)
		, TBM_SETPOS
		, (WPARAM)TRUE
		, (LPARAM)100
	);

	// set notes for three note chord
	std::vector<float> chord;
	chord.push_back(g_oNote.aFreq[60]);
	chord.push_back(g_oNote.aFreq[57]);
	chord.push_back(g_oNote.aFreq[53]);
	g_chord.push_back(chord);
	chord.clear();
	chord.push_back(g_oNote.aFreq[62]);
	chord.push_back(g_oNote.aFreq[59]);
	chord.push_back(g_oNote.aFreq[55]);
	g_chord.push_back(chord);

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
//*                     onWmNotify_DlgProc
//*****************************************************************************
//BOOL onWmNotify_DlgProc(const HWND& hDlg
//	, const LPARAM& lParam
//)
//{
//	switch (((LPNMHDR)lParam)->code)
//	{
//	case NM_RELEASEDCAPTURE:
//	{
//		switch (((LPNMHDR)lParam)->idFrom)
//		{
//		case IDC_SLIDER_LVOLUME:
//		{
//			g_left_volume = (float)(SendMessage(GetDlgItem(hDlg, IDC_SLIDER_LVOLUME)
//				, TBM_GETPOS
//				, (WPARAM)0
//				, (LPARAM)0
//			) / 100.f);
//
//			break;
//		} // eof IDC_SLIDER_LVOLUME
//		case IDC_SLIDER_RVOLUME:
//		{
//			g_right_volume = (float)(SendMessage(GetDlgItem(hDlg, IDC_SLIDER_RVOLUME)
//				, TBM_GETPOS
//				, (WPARAM)0
//				, (LPARAM)0
//			) / 100.f);
//
//			break;
//		} // eof IDC_SLIDER_RVOLUME
//		} // eof switch
//		break;
//	}
//	} // eof switch
//
//	return EXIT_SUCCESS;
//}
//*****************************************************************************
//*                     onWmHscroll_DlgProc
//*****************************************************************************
BOOL onWmHscroll_DlgProc(const HWND& hDlg
	, const WPARAM& wParam
	, const LPARAM& lParam
)
{
	if (LOWORD(wParam) == TB_THUMBTRACK)
	{
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER_LVOLUME))
		{
			g_left_volume = (float)(HIWORD(wParam) / 100.f);
		}
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER_RVOLUME))
		{
			g_right_volume = (float)(HIWORD(wParam) / 100.f);
		}
	}

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
		std::wstring wstrFrequency = L"";
		WCHAR wszBuffer[BUFFER_MAX] = { '\0' };
		HWND hWnd = GetDlgItem(hDlg, IDC_START);
		SendMessage(hWnd
			, WM_GETTEXT
			, (WPARAM)BUFFER_MAX
			, (LPARAM)wszBuffer
		);
		if (wcscmp(wszBuffer, L"Start") == 0)
		{
			OutputDebugString(L"Start\n");
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
				OutputDebugString(L"NOISE\n");
				start_play(NOISE);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_NOTE)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				SendMessage(GetDlgItem(hDlg, IDC_CB_NOTE)
					, WM_GETTEXT
					, (WPARAM)BUFFER_MAX
					, (LPARAM)wszBuffer
				);
				wstrFrequency = wszBuffer;
				wstrFrequency = wstrFrequency.erase(0
					, wstrFrequency.find(L" - ") + 3
				);
				g_frequency_hz = _wtof(wstrFrequency.c_str());
				OutputDebugString(L"NOTE\n");
				start_play(NOTE);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_SWEEP)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				// get sweep low frequency
				SendMessage(GetDlgItem(hDlg, IDC_CB_SWEEP_LO)
					, WM_GETTEXT
					, (WPARAM)BUFFER_MAX
					, (LPARAM)wszBuffer
				);
				wstrFrequency = wszBuffer;
				wstrFrequency = wstrFrequency.erase(wstrFrequency.find(L" - ")
					, wstrFrequency.length()
				);
				g_idx_freq_sweep_lo = _wtoi(wstrFrequency.c_str());
				// get sweep high frequency
				SendMessage(GetDlgItem(hDlg, IDC_CB_SWEEP_HI)
					, WM_GETTEXT
					, (WPARAM)BUFFER_MAX
					, (LPARAM)wszBuffer
				);
				wstrFrequency = wszBuffer;
				wstrFrequency = wstrFrequency.erase(wstrFrequency.find(L" - ")
					, wstrFrequency.length()
				);
				g_idx_freq_sweep_hi = _wtoi(wstrFrequency.c_str());
				OutputDebugString(L"SWEEP\n");
				if (g_idx_freq_sweep_hi > g_idx_freq_sweep_lo) start_play(SWEEP);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_CHORD)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				g_idx_chord = SendMessage(GetDlgItem(hDlg, IDC_CB_CHORD)
					, CB_GETCURSEL
					, (WPARAM)0
					, (LPARAM)0
				);
				OutputDebugString(L"CHORD\n");
				start_play(CHORD);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_METRONOME)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				SendMessage(GetDlgItem(hDlg, IDC_CB_BPM)
					, WM_GETTEXT
					, (WPARAM)BUFFER_MAX
					, (LPARAM)wszBuffer
				);
				std::wstring wstrBpm = wszBuffer;
				g_bpm = _wtoi(wstrBpm.c_str());
				OutputDebugString(L"METRONOME\n");
				start_play(METRONOME);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_MELODY)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				OutputDebugString(L"MELODY\n");
				start_play(MELODY);
			}
		}
		if (wcscmp(wszBuffer, L"Stop") == 0)
		{
			SendMessage(hWnd
				, WM_SETTEXT
				, (WPARAM)0
				, (LPARAM)L"Start"
			);
			OutputDebugString(L"Stop\n");
			// stop func playAudioStream() and let the thread die
			g_flags = AUDCLNT_BUFFERFLAGS_SILENT;
		}
		return (INT_PTR)TRUE;
	} // eof IDC_START
	} // eof wsitch
	return (INT_PTR)FALSE;
}

