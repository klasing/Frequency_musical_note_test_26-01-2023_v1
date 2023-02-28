#pragma once
//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"
#include "Mixer_Fmnt_26-01-2023_v1.h"

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
//*                     struct
//*****************************************************************************
typedef struct tagVOLUME
{
	float left_volume = 0.f;
	float right_volume = 0.f;
} VOLUME, *PVOLUME;

typedef struct tagRATE
{
	float value = 0.f;
} RATE, *PRATE;

typedef struct tagINIT
{
	Note oNote;
	DWORD flags = AUDCLNT_BUFFERFLAGS_SILENT;
	UINT16 play_item = 0;
	// NOISE
	VOLUME volume_chnl1{};
	// NOTE
	VOLUME volume_chnl2{};
	float frequency_hz = oNote.aFreq[69];
	float delta_note = 0.f;
	float phase_note = 0.f;
	// SWEEP
	VOLUME volume_chnl3{};
	RATE rate_sweep{};
	int idx_freq_sweep_lo = 45;
	int idx_freq_sweep_hi = 93;
	int index_sweep = 0;
	int irate_sweep = 16;
	int cSweep = 0;
	// CHORD
	VOLUME volume_chnl4{};
	std::vector<std::vector<float>> chord{};
	int idx_chord = 0;
	float delta3_chord = 0.f, delta2_chord = 0.f, delta1_chord = 0.f;
	float phase3_chord = 0.f, phase2_chord = 0.f, phase1_chord = 0.f;
	float freq3_chord = 0.f, freq2_chord = 0.f, freq1_chord = 0.f;
	// METRONOME
	VOLUME volume_chnl5{};
	int bpm = 60;
	//************************************************************************
	//*                 init
	//************************************************************************
	VOID init(const UINT16& play_item_)
	{
		if ((play_item & NOTE) == play_item_)
		{
			delta_note = 
				2.f * frequency_hz * float(M_PI / SAMPLE_RATE);
			return;
		}
		if ((play_item & SWEEP) == play_item_)
		{
			index_sweep = idx_freq_sweep_lo;
			irate_sweep = 16 - rate_sweep.value;
			cSweep = 0;
			return;
		}
		if ((play_item & CHORD) == play_item_)
		{
			// for now; only for a three note chord
			freq3_chord = chord[idx_chord][0];
			delta3_chord = 2.f * freq3_chord * float(M_PI / SAMPLE_RATE);
			phase3_chord = 0.f;

			// the delta for the lower frequencies
			// are a fraction of the highest frequency delta
			freq2_chord = chord[idx_chord][1];
			delta2_chord = delta3_chord * (freq2_chord / freq3_chord);
			phase2_chord = 0.f;

			freq1_chord = chord[idx_chord][2];
			delta1_chord = delta3_chord * (freq1_chord / freq3_chord);
			phase1_chord = 0.f;
			return;
		}
	}
} INIT, *PINIT;

//*****************************************************************************
//*                     global
//*****************************************************************************
WCHAR wszBuffer[BUFFER_MAX] = { '\0' };

std::unique_ptr<INIT> g_pinit = std::unique_ptr<INIT>(new INIT);

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

			// must be outside the -UINT32 i- loop
			// this count controls the sweep rate
			if ((g_pinit->play_item & SWEEP) == SWEEP)
			{
				if (g_pinit->cSweep == g_pinit->irate_sweep)
				{
					// upper note index for the sweep
					if (up_down > 0 && g_pinit->index_sweep < g_pinit->idx_freq_sweep_hi)
						++g_pinit->index_sweep;
					else
					{
						up_down = -1;
						// lower note index for the sweep
						if (up_down < 0 && g_pinit->index_sweep > g_pinit->idx_freq_sweep_lo)
							--g_pinit->index_sweep;
						else
							up_down = 1;
					}

					g_pinit->cSweep = 0;
				}
				else
					++g_pinit->cSweep;

				freq_sweep = g_pinit->oNote.aFreq[g_pinit->index_sweep];
				delta_sweep = 2.f * freq_sweep * float(M_PI / SAMPLE_RATE);
			}

			for (UINT32 i = 0; i < format.Format.nChannels * numFramesAvailable; i++)
			{
				fData[i] = 0.f;
				if ((g_pinit->play_item & NOISE) == NOISE)
				{
					if (i % 2 == 0)
						fData[i] +=
						g_pinit->volume_chnl1.left_volume * float_dist(engine);
					else
						fData[i] +=
						g_pinit->volume_chnl1.right_volume * float_dist(engine);
				}
				if ((g_pinit->play_item & NOTE) == NOTE)
				{
					float next_sample = std::sin(g_pinit->phase_note);
					g_pinit->phase_note = 
						std::fmod(g_pinit->phase_note
							+ g_pinit->delta_note, 2.f * static_cast<float>(M_PI));
					if (i % 2 == 0)
						fData[i] +=
						g_pinit->volume_chnl2.left_volume * next_sample;
					else
						fData[i] +=
						g_pinit->volume_chnl2.right_volume * next_sample;
				}
				if ((g_pinit->play_item & SWEEP) == SWEEP)
				{
					float next_sample = std::sin(phase_sweep);
					phase_sweep = std::fmod(phase_sweep + delta_sweep, 2.f * static_cast<float>(M_PI));
					if (i % 2 == 0)
						fData[i] +=
						g_pinit->volume_chnl3.left_volume * next_sample;
					else
						fData[i] +=
						g_pinit->volume_chnl3.right_volume * next_sample;
				}
				if ((g_pinit->play_item & CHORD) == CHORD)
				{
					float next_sample = std::sin(g_pinit->phase3_chord)
						+ std::sin(g_pinit->phase2_chord)
						+ std::sin(g_pinit->phase1_chord);
					g_pinit->phase3_chord =
						std::fmod(g_pinit->phase3_chord + g_pinit->delta3_chord, 2.f * static_cast<float>(M_PI));
					g_pinit->phase2_chord =
						std::fmod(g_pinit->phase2_chord + g_pinit->delta2_chord, 2.f * static_cast<float>(M_PI));
					g_pinit->phase1_chord =
						std::fmod(g_pinit->phase1_chord + g_pinit->delta1_chord, 2.f * static_cast<float>(M_PI));

					if (i % 2 == 0)
						fData[i] +=
						g_pinit->volume_chnl4.left_volume * next_sample;
					else
						fData[i] +=
						g_pinit->volume_chnl4.right_volume * next_sample;
				}
				if ((g_pinit->play_item & METRONOME) == METRONOME)
				{
					float next_sample = std::sin(metronome_phase);
					metronome_phase = std::fmod(
						metronome_phase + metronome_delta, 2.f * static_cast<float>(M_PI));

					if (i < PERIOD_ONE_TICK && cSample < PERIOD_ONE_TICK)
					{
						if (i % 2 == 0)
							fData[i] +=
							g_pinit->volume_chnl5.left_volume * next_sample;
						else
							fData[i] +=
							g_pinit->volume_chnl5.right_volume * next_sample;
					}
					else
						fData[i] += 0.f;

					cSample = ++cSample
						% (int)(2 * SAMPLE_RATE * (60. / (float)g_pinit->bpm));
				}
			}
		}
		return S_OK;
	}
private:
	WAVEFORMATEXTENSIBLE format;
	// NOISE
	std::mt19937_64 engine;
	std::uniform_real_distribution<float> float_dist;
	// SWEEP
	//int cSweep = 0;
	int up_down = 1;
	float freq_sweep = 0.f;
	float phase_sweep = 0.f;
	float delta_sweep = 0.f;
	// METRONOME
	const int PERIOD_ONE_TICK = 9091;
	float metronome_freq = g_pinit->oNote.aFreq[45];
	float metronome_delta = 2.f * metronome_freq * float(M_PI / SAMPLE_RATE);
	float metronome_phase = 0.f;
	int cSample = 0;

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
	OutputDebugString(L"playAudioStream()\n");
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

	hr = CoInitialize(nullptr);

	hr = CoCreateInstance(CLSID_MMDeviceEnumerator
		, NULL
		, CLSCTX_ALL
		, IID_IMMDeviceEnumerator
		, (void**)&pEnumerator
	);

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
		, g_pinit->flags
		//, g_flags
	);
	EXIT_ON_ERROR(hr);

	// calculate the actual duration of the allocated buffer
	hnsActualDuration = REFTIMES_PER_SEC
		* bufferFrameCount / pwfx->nSamplesPerSec;

	// start playing
	hr = pAudioClient->Start();
	EXIT_ON_ERROR(hr);
	// each loop fills about half of the shared buffer
	while (TRUE)
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
			, g_pinit->flags
			//, g_flags
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
BOOL start_play()
{
	OutputDebugString(L"start_play()\n");

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
	for (auto i = 0; i < 128; ++i)
	{
		swprintf_s(wszBuffer
			, (size_t)16
			, L"%3d - %6.1f"
			, i
			, g_pinit->oNote.aFreq[i]
		);
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
	// set range sweep rate
	LPARAM lparam = MAKELPARAM(0, 15);
	SendMessage(GetDlgItem(hDlg, IDC_RATE_SWEEP)
		, TBM_SETRANGE
		, (WPARAM)FALSE
		, (LPARAM)lparam
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

	// set notes for three note chord
	std::vector<float> chord;
	chord.push_back(g_pinit->oNote.aFreq[60]);
	chord.push_back(g_pinit->oNote.aFreq[57]);
	chord.push_back(g_pinit->oNote.aFreq[53]);
	g_pinit->chord.push_back(chord);
	chord.clear();
	chord.push_back(g_pinit->oNote.aFreq[62]);
	chord.push_back(g_pinit->oNote.aFreq[59]);
	chord.push_back(g_pinit->oNote.aFreq[55]);
	g_pinit->chord.push_back(chord);

	// add content to the combobox IDC_CB_BPM
	for (auto i = 0; i <= 21; ++i)
	{
		// the available range is from 60 bpm to 270 bpm
		// 270 = 60 + 10 * 21
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

	// start thread
	start_play();
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
//*                     onWmHscroll_DlgProc
//*****************************************************************************
INT_PTR onWmHscroll_DlgProc(const HWND& hDlg
	, const WPARAM& wParam
	, const LPARAM& lParam
)
{
	int track_pos = 0;
	switch (LOWORD(wParam))
	{
	case TB_LINEDOWN:
	case TB_LINEUP:
	case TB_THUMBTRACK:
	{
		// NOISE
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_LVOLUME_CHNL1))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_LVOLUME_CHNL1)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			g_pinit->volume_chnl1.left_volume = track_pos / 100.f;

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_LVOLUME_CHNL1"
				, track_pos
				, g_pinit->volume_chnl1.left_volume
			);
			OutputDebugString(wszBuffer);

			return (INT_PTR)TRUE;
		}
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_RVOLUME_CHNL1))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_RVOLUME_CHNL1)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			g_pinit->volume_chnl1.right_volume = track_pos / 100.f;

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_RVOLUME_CHNL1"
				, track_pos
				, g_pinit->volume_chnl1.right_volume
			);
			OutputDebugString(wszBuffer);

			return (INT_PTR)TRUE;
		}
		// NOTE
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_LVOLUME_CHNL2))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_LVOLUME_CHNL2)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			g_pinit->volume_chnl2.left_volume = track_pos / 100.f;

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_LVOLUME_CHNL2"
				, track_pos
				, g_pinit->volume_chnl2.left_volume
			);
			OutputDebugString(wszBuffer);

			return (INT_PTR)TRUE;
		}
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_RVOLUME_CHNL2))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_RVOLUME_CHNL2)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			g_pinit->volume_chnl2.right_volume = track_pos / 100.f;

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_RVOLUME_CHNL2"
				, track_pos
				, g_pinit->volume_chnl2.right_volume
			);
			OutputDebugString(wszBuffer);

			return (INT_PTR)TRUE;
		}
		// SWEEP
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_LVOLUME_CHNL3))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_LVOLUME_CHNL3)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			g_pinit->volume_chnl3.left_volume = track_pos / 100.f;

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_LVOLUME_CHNL3"
				, track_pos
				, g_pinit->volume_chnl3.left_volume
			);
			OutputDebugString(wszBuffer);

			return (INT_PTR)TRUE;
		}
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_RVOLUME_CHNL3))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_RVOLUME_CHNL3)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			g_pinit->volume_chnl3.right_volume = track_pos / 100.f;

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_RVOLUME_CHNL3"
				, track_pos
				, g_pinit->volume_chnl3.right_volume
			);
			OutputDebugString(wszBuffer);

			return (INT_PTR)TRUE;
		}
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_RATE_SWEEP))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_RATE_SWEEP)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			g_pinit->rate_sweep.value = track_pos;

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_RATE_SWEEP"
				, track_pos
				, 16 - g_pinit->rate_sweep.value
			);
			// update rate sweep
			g_pinit->init(SWEEP);
			OutputDebugString(wszBuffer);

			return (INT_PTR)TRUE;
		}
		// CHORD
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_LVOLUME_CHNL4))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_LVOLUME_CHNL4)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			g_pinit->volume_chnl4.left_volume = track_pos / 100.f;

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_LVOLUME_CHNL4"
				, track_pos
				, g_pinit->volume_chnl4.left_volume
			);
			OutputDebugString(wszBuffer);

			return (INT_PTR)TRUE;
		}
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_RVOLUME_CHNL4))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_RVOLUME_CHNL4)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			g_pinit->volume_chnl4.right_volume = track_pos / 100.f;

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_RVOLUME_CHNL4"
				, track_pos
				, g_pinit->volume_chnl4.right_volume
			);
			OutputDebugString(wszBuffer);

			return (INT_PTR)TRUE;
		}
		// METRONOME
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_LVOLUME_CHNL5))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_LVOLUME_CHNL5)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			g_pinit->volume_chnl5.left_volume = track_pos / 100.f;

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_LVOLUME_CHNL5"
				, track_pos
				, g_pinit->volume_chnl5.left_volume
			);
			OutputDebugString(wszBuffer);

			return (INT_PTR)TRUE;
		}
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_RVOLUME_CHNL5))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_RVOLUME_CHNL5)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			g_pinit->volume_chnl5.right_volume = track_pos / 100.f;

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_RVOLUME_CHNL5"
				, track_pos
				, g_pinit->volume_chnl5.right_volume
			);
			OutputDebugString(wszBuffer);

			return (INT_PTR)TRUE;
		}
	} // eof TB_LINEDOWN | TB_LINEUP | TB_THUMBTRACK
	} // eof switch
	
	return (INT_PTR)FALSE;
}

//*****************************************************************************
//*                     onWmCommand_DlgProc
//*****************************************************************************
INT_PTR onWmCommand_DlgProc(const HWND& hDlg
	, const WPARAM& wParam
	, const LPARAM& lParam
)
{
	std::wstring wstrFrequency = L"";
	switch (LOWORD(wParam))
	{
	case IDC_NOISE:
	{
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
		{
			if (SendMessage((HWND)lParam
				, BM_GETCHECK
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				g_pinit->play_item |= NOISE;
			}
			else
			{
				g_pinit->play_item &= ~NOISE;
			}
			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"play_item: %d\n"
				, g_pinit->play_item
			);
			OutputDebugString(wszBuffer);
			return (INT_PTR)TRUE;
		} // eof BN_CLICKED
		}  // eof switch
		break;
	} // eof IDC_NOISE
	case IDC_NOTE:
	{
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
		{
			if (SendMessage((HWND)lParam
				, BM_GETCHECK
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				g_pinit->play_item |= NOTE;
			}
			else
			{
				g_pinit->play_item &= ~NOTE;
			}
			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"play_item: %d\n"
				, g_pinit->play_item
			);
			OutputDebugString(wszBuffer);
			g_pinit->init(NOTE);
			return (INT_PTR)TRUE;
		} // eof BN_CLICKED
		}  // eof switch
		break;
	} // eof IDC_NOTE
	case IDC_CB_NOTE:
	{
		OutputDebugString(L"IDC_CB_NOTE\n");
		switch (HIWORD(wParam))
		{
		case CBN_SELCHANGE:
		{
			OutputDebugString(L"CBN_SELCHANGE\n");
			SendMessage(GetDlgItem(hDlg, IDC_CB_NOTE)
				, WM_GETTEXT
				, (WPARAM)BUFFER_MAX
				, (LPARAM)wszBuffer
			);
			wstrFrequency = wszBuffer;
			wstrFrequency = wstrFrequency.erase(0
				, wstrFrequency.find(L" - ") + 3
			);
			g_pinit->frequency_hz = _wtof(wstrFrequency.c_str());
			g_pinit->init(NOTE);
			return (INT_PTR)TRUE;
		} // eof CBN_SELCHANGE
		} // eof switch
		break;
	} // eof IDC_CB_NOTE
	case IDC_SWEEP:
	{
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
		{
			if (SendMessage((HWND)lParam
				, BM_GETCHECK
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				g_pinit->play_item |= SWEEP;
			}
			else
			{
				g_pinit->play_item &= ~SWEEP;
			}
			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"play_item: %d\n"
				, g_pinit->play_item
			);
			OutputDebugString(wszBuffer);
			g_pinit->init(SWEEP);
			return (INT_PTR)TRUE;
		} // eof BN_CLICKED
		}  // eof switch
		break;
	} // eof IDC_NOTE
	case IDC_CB_SWEEP_LO:
	{
		OutputDebugString(L"IDC_CB_SWEEP_LO\n");
		switch (HIWORD(wParam))
		{
		case CBN_SELCHANGE:
		{
			OutputDebugString(L"CBN_SELCHANGE\n");
			SendMessage(GetDlgItem(hDlg, IDC_CB_SWEEP_LO)
				, WM_GETTEXT
				, (WPARAM)BUFFER_MAX
				, (LPARAM)wszBuffer
			);
			wstrFrequency = wszBuffer;
			wstrFrequency = wstrFrequency.erase(wstrFrequency.find(L" - ")
				, wstrFrequency.length()
			);
			g_pinit->idx_freq_sweep_lo = _wtoi(wstrFrequency.c_str());
			g_pinit->init(SWEEP);
			return (INT_PTR)TRUE;
		} // eof CBN_SELCHANGE
		} // eof switch
		break;
	} // eof IDC_CB_SWEEP_LO
	case IDC_CB_SWEEP_HI:
	{
		OutputDebugString(L"IDC_CB_SWEEP_HI\n");
		switch (HIWORD(wParam))
		{
		case CBN_SELCHANGE:
		{
			OutputDebugString(L"CBN_SELCHANGE\n");
			SendMessage(GetDlgItem(hDlg, IDC_CB_SWEEP_HI)
				, WM_GETTEXT
				, (WPARAM)BUFFER_MAX
				, (LPARAM)wszBuffer
			);
			wstrFrequency = wszBuffer;			
			wstrFrequency = wstrFrequency.erase(wstrFrequency.find(L" - ")
				, wstrFrequency.length()
			);
			g_pinit->idx_freq_sweep_hi = _wtoi(wstrFrequency.c_str());
			g_pinit->init(SWEEP);
			return (INT_PTR)TRUE;
		} // eof CBN_SELCHANGE
		} // eof switch
		break;
	} // eof IDC_CB_SWEEP_HI
	case IDC_CHORD:
	{
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
		{
			if (SendMessage((HWND)lParam
				, BM_GETCHECK
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				g_pinit->play_item |= CHORD;
			}
			else
			{
				g_pinit->play_item &= ~CHORD;
			}
			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"play_item: %d\n"
				, g_pinit->play_item
			);
			OutputDebugString(wszBuffer);
			g_pinit->init(CHORD);
			return (INT_PTR)TRUE;
		} // eof BN_CLICKED
		}  // eof switch
		break;
	} // eof IDC_CHORD
	case IDC_CB_CHORD:
	{
		OutputDebugString(L"IDC_CB_CHORD\n");
		switch (HIWORD(wParam))
		{
		case CBN_SELCHANGE:
		{
			OutputDebugString(L"CBN_SELCHANGE\n");
			g_pinit->idx_chord = SendMessage(GetDlgItem(hDlg, IDC_CB_CHORD)
				, CB_GETCURSEL
				, (WPARAM)0
				, (LPARAM)0
			);
			g_pinit->init(CHORD);
			return (INT_PTR)TRUE;
		} // eof CBN_SELCHANGE
		} // eof switch
		break;
	} // eof IDC_CB_CHORD
	case IDC_METRONOME:
	{
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
		{
			if (SendMessage((HWND)lParam
				, BM_GETCHECK
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				g_pinit->play_item |= METRONOME;
			}
			else
			{
				g_pinit->play_item &= ~METRONOME;
			}
			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"play_item: %d\n"
				, g_pinit->play_item
			);
			OutputDebugString(wszBuffer);
			// no init necessary
			return (INT_PTR)TRUE;
		} // eof BN_CLICKED
		}  // eof switch
		break;
	} // eof IDC_METRONOME
	case IDC_CB_BPM:
	{
		OutputDebugString(L"IDC_CB_BPM\n");
		switch (HIWORD(wParam))
		{
		case CBN_SELCHANGE:
		{
			OutputDebugString(L"CBN_SELCHANGE\n");
			SendMessage(GetDlgItem(hDlg, IDC_CB_BPM)
				, WM_GETTEXT
				, (WPARAM)BUFFER_MAX
				, (LPARAM)wszBuffer
			);
			std::wstring wstrBpm = wszBuffer;
			g_pinit->bpm = _wtoi(wstrBpm.c_str());
			return (INT_PTR)TRUE;
		} // eof CBN_SELCHANGE
		} // eof switch
		break;
	} // eof IDC_CB_BPM
	case IDC_START:
	{
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
			// enable sound playing
			g_pinit->flags = 0;
		}
		else
		{
			OutputDebugString(L"Stop\n");
			// change text on button
			SendMessage(hWnd
				, WM_SETTEXT
				, (WPARAM)0
				, (LPARAM)L"Start"
			);
			// disable sound playing
			g_pinit->flags = AUDCLNT_BUFFERFLAGS_SILENT;
		}
		return (INT_PTR)TRUE;
	} // eof IDC_START
	} // eof switch
	return (INT_PTR)FALSE;
}
