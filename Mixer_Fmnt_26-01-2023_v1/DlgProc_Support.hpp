#pragma once
//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"
#include "Mixer_Fmnt_26-01-2023_v1.h"

//*****************************************************************************
//*                     Volume
//*****************************************************************************
//class Volume
//{
//public:
//	float left_volume = 0.f;
//	float right_volume = 0.f;
//};

//*****************************************************************************
//*                     struct
//*****************************************************************************
typedef struct tagVOLUME
{
	float left_volume = 0.f;
	float right_volume = 0.f;
} VOLUME, *PVOLUME;

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
WCHAR wszBuffer[BUFFER_MAX] = { '\0' };

Note g_oNote;
DWORD g_flags = AUDCLNT_BUFFERFLAGS_SILENT;

UINT16 g_play_item = 0;
float g_frequency_hz = PITCH_STANDARD_HZ;
int g_idx_freq_sweep_lo = 45;// 0;
int g_idx_freq_sweep_hi = 93;// 0;

std::vector<std::vector<float>> g_chord{};
int g_idx_chord = 0;

VOLUME g_volume_chnl1;
VOLUME g_volume_chnl2;
VOLUME g_volume_chnl3;
VOLUME g_volume_chnl4;
VOLUME g_volume_chnl5;

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
			sweep_delta = 2.f * sweep_freq * float(M_PI / SAMPLE_RATE);
			
			for (UINT32 i = 0; i < format.Format.nChannels * numFramesAvailable; i++)
			{
				fData[i] = 0.f;
				if ((g_play_item & NOISE) == NOISE)
				{
					if (i % 2 == 0)
						fData[i] +=
						g_volume_chnl1.left_volume * float_dist(engine);
					else
						fData[i] +=
						g_volume_chnl1.right_volume * float_dist(engine);
				}
				if ((g_play_item & NOTE) == NOTE)
				{
					float next_sample = std::sin(phase);
					phase = std::fmod(phase + delta, 2.f * static_cast<float>(M_PI));
					if (i % 2 == 0)
						fData[i] += 
						g_volume_chnl2.left_volume * next_sample;
					else
						fData[i] += 
						g_volume_chnl2.right_volume * next_sample;
				}
				if ((g_play_item & SWEEP) == SWEEP)
				{
					float next_sample = std::sin(sweep_phase);
					sweep_phase = std::fmod(sweep_phase + sweep_delta, 2.f * static_cast<float>(M_PI));
					if (i % 2 == 0)
						fData[i] += 
						g_volume_chnl3.left_volume * next_sample;
					else
						fData[i] += 
						g_volume_chnl3.right_volume * next_sample;
				}
				if ((g_play_item & CHORD) == CHORD)
				{
					float next_sample = std::sin(chord_phase3)
						+ std::sin(chord_phase2)
						+ std::sin(chord_phase1);
					chord_phase3 = std::fmod(chord_phase3 + chord_delta3, 2.f * static_cast<float>(M_PI));
					chord_phase2 = std::fmod(chord_phase2 + chord_delta2, 2.f * static_cast<float>(M_PI));
					chord_phase1 = std::fmod(chord_phase1 + chord_delta1, 2.f * static_cast<float>(M_PI));

					if (i % 2 == 0)
						fData[i] += 
						g_volume_chnl4.left_volume * next_sample;
					else
						fData[i] += 
						g_volume_chnl4.right_volume * next_sample;
				}
			}
		}
		return S_OK;
	}
	//************************************************************************
	//*                 init
	//************************************************************************
	VOID init()
	{
		if ((g_play_item & NOTE) == NOTE)
		{
			delta = 2.f * g_frequency_hz * float(M_PI / SAMPLE_RATE);
		}
		if ((g_play_item & SWEEP) == SWEEP)
		{
			index_sweep = g_idx_freq_sweep_lo;
		}
		if ((g_play_item & CHORD) == CHORD)
		{
			// for now; only for a three note chord
			chord_freq3 = g_chord[g_idx_chord][0];
			chord_delta3 = 2.f * chord_freq3 * float(M_PI / SAMPLE_RATE);
			chord_phase3 = 0.f;

			// the delta for the lower frequencies
			// are a fraction of the highest frequency delta
			chord_freq2 = g_chord[g_idx_chord][1];
			chord_delta2 = chord_delta3 * (chord_freq2 / chord_freq3);
			chord_phase2 = 0.f;

			chord_freq1 = g_chord[g_idx_chord][2];
			chord_delta1 = chord_delta3 * (chord_freq1 / chord_freq3);
			chord_phase1 = 0.f;
		}
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
	float sweep_delta = 0.f;
	float sweep_phase = 0.f;
	// CHORD
	float chord_delta3 = 0.f, chord_delta2 = 0.f, chord_delta1 = 0.f;
	float chord_phase3 = 0.f, chord_phase2 = 0.f, chord_phase1 = 0.f;
	float chord_freq3 = 0.f, chord_freq2 = 0.f, chord_freq1 = 0.f;
};
/*
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
*/

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
	pMySource->init();

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
/*
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
*/

//*****************************************************************************
//*                     start_play
//*****************************************************************************
using define_code = UINT16;
BOOL start_play()
{
	OutputDebugString(L"start_play()\n");

	// reset flag to enable playing
	g_flags = 0;

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
BOOL onWmHscroll_DlgProc(const HWND& hDlg
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
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_LVOLUME_CHNL1))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_LVOLUME_CHNL1)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			g_volume_chnl1.left_volume =
				track_pos / 100.f;
				//(track_pos == 0) ? 0. : std::exp(track_pos / 100.f) / M_E;

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_LVOLUME_CHNL1"
				, track_pos
				, g_volume_chnl1.left_volume
			);
			OutputDebugString(wszBuffer);

			return EXIT_SUCCESS;
		}
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_RVOLUME_CHNL1))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_RVOLUME_CHNL1)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			g_volume_chnl1.right_volume =
				track_pos / 100.f;
				//(track_pos == 0) ? 0. : std::exp(track_pos / 100.f) / M_E;

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_RVOLUME_CHNL1"
				, track_pos
				, g_volume_chnl1.right_volume
			);
			OutputDebugString(wszBuffer);

			return EXIT_SUCCESS;
		}

		if ((HWND)lParam == GetDlgItem(hDlg, IDC_LVOLUME_CHNL2))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_LVOLUME_CHNL2)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			g_volume_chnl2.left_volume =
				track_pos / 100.f;
			//(track_pos == 0) ? 0. : std::exp(track_pos / 100.f) / M_E;

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_LVOLUME_CHNL2"
				, track_pos
				, g_volume_chnl2.left_volume
			);
			OutputDebugString(wszBuffer);

			return EXIT_SUCCESS;
		}
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_RVOLUME_CHNL2))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_RVOLUME_CHNL2)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			g_volume_chnl2.right_volume =
				track_pos / 100.f;
			//(track_pos == 0) ? 0. : std::exp(track_pos / 100.f) / M_E;

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_RVOLUME_CHNL2"
				, track_pos
				, g_volume_chnl2.right_volume
			);
			OutputDebugString(wszBuffer);

			return EXIT_SUCCESS;
		}
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_LVOLUME_CHNL3))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_LVOLUME_CHNL3)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			g_volume_chnl3.left_volume =
				track_pos / 100.f;
			//(track_pos == 0) ? 0. : std::exp(track_pos / 100.f) / M_E;

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_LVOLUME_CHNL3"
				, track_pos
				, g_volume_chnl3.left_volume
			);
			OutputDebugString(wszBuffer);

			return EXIT_SUCCESS;
		}
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_RVOLUME_CHNL3))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_RVOLUME_CHNL3)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			g_volume_chnl3.right_volume =
				track_pos / 100.f;
			//(track_pos == 0) ? 0. : std::exp(track_pos / 100.f) / M_E;

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_RVOLUME_CHNL3"
				, track_pos
				, g_volume_chnl3.right_volume
			);
			OutputDebugString(wszBuffer);

			return EXIT_SUCCESS;
		}
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_LVOLUME_CHNL4))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_LVOLUME_CHNL4)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			g_volume_chnl4.left_volume =
				track_pos / 100.f;
			//(track_pos == 0) ? 0. : std::exp(track_pos / 100.f) / M_E;

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_LVOLUME_CHNL4"
				, track_pos
				, g_volume_chnl4.left_volume
			);
			OutputDebugString(wszBuffer);

			return EXIT_SUCCESS;
		}
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_RVOLUME_CHNL4))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_RVOLUME_CHNL4)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			g_volume_chnl4.right_volume =
				track_pos / 100.f;
			//(track_pos == 0) ? 0. : std::exp(track_pos / 100.f) / M_E;

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_RVOLUME_CHNL4"
				, track_pos
				, g_volume_chnl4.right_volume
			);
			OutputDebugString(wszBuffer);

			return EXIT_SUCCESS;
		}
	} // eof TB_LINEDOWN | TB_LINEUP | TB_THUMBTRACK
	} // eof switch

	return EXIT_SUCCESS;
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
				g_play_item |= NOISE;
			}
			else
			{
				g_play_item &= ~NOISE;
			}

			swprintf_s(wszBuffer, (size_t)BUFFER_MAX, L"g_play_item: %d\n", g_play_item);
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
				g_play_item |= NOTE;
			}
			else
			{
				g_play_item &= ~NOTE;
			}

			swprintf_s(wszBuffer, (size_t)BUFFER_MAX, L"g_play_item: %d\n", g_play_item);
			OutputDebugString(wszBuffer);

			return (INT_PTR)TRUE;
		} // eof BN_CLICKED
		}  // eof switch
		break;
	} // eof IDC_NOTE
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
				// get index low frequency sweep
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
				// get index high frequency sweep
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
				// the lower sweep frequency must be lower
				// than the high sweep frequency
				if (g_idx_freq_sweep_hi > g_idx_freq_sweep_lo)
					g_play_item |= SWEEP;
			}
			else
			{
				g_play_item &= ~SWEEP;
			}
			
			return (INT_PTR)TRUE;
		} // eof BN_CLICKED
		}  // eof switch
		break;
	} // eof IDC_SWEEP
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
				g_idx_chord = SendMessage(GetDlgItem(hDlg, IDC_CB_CHORD)
					, CB_GETCURSEL
					, (WPARAM)0
					, (LPARAM)0
				);
				g_play_item |= CHORD;
			}
			else
			{
				g_play_item &= ~CHORD;
			}

			return (INT_PTR)TRUE;
		} // eof BN_CLICKED
		}  // eof switch
		break;
	} // eof IDC_CHORD
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
			start_play();
		}
		if (wcscmp(wszBuffer, L"Stop") == 0)
		{
			OutputDebugString(L"Stop\n");
			// change text on button
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
	} // eof switch

	//switch (HIWORD(wParam))
	//{
	//case CBN_SELCHANGE:
	//{
	//	// the user changed the selection in one of the comboboxes
	//	// LOWORD(wParam) contains the control identifier of the combobox
	//	// lParam contains a handle to the combobox
	//	switch (LOWORD(wParam))
	//	{
	//	case IDC_CB_NOTE:
	//	{
	//		OutputDebugString(L"CBN_SELCHANGE IDC_CB_NOTE\n");
	//		// TODO:
	//		return (INT_PTR)TRUE;
	//	} // eof IDC_CB_NOTE
	//	case IDC_CB_SWEEP_HI:
	//	{
	//		OutputDebugString(L"CBN_SELCHANGE IDC_CB_SWEEP_HI\n");
	//		// TODO:
	//		return (INT_PTR)TRUE;
	//	} // eof IDC_CB_SWEEP_HI
	//	case IDC_CB_SWEEP_LO:
	//	{
	//		OutputDebugString(L"CBN_SELCHANGE IDC_CB_SWEEP_LO\n");
	//		// TODO:
	//		return (INT_PTR)TRUE;
	//	} // eof IDC_CB_SWEEP_LO
	//	} // eof switch
	//	return (INT_PTR)FALSE;
	//} // eof CBN_EDITCHANGE

	//} // eof switch

	return (INT_PTR)FALSE;
}

//*****************************************************************************
//*                     onWmNotify_DlgProc
//*****************************************************************************
//BOOL onWmNotify_DlgProc(const HWND& hDlg
//	, const WPARAM& wParam
//	, const LPARAM& lParam
//)
//{
//	OutputDebugString(L"WM_NOTIFY\n");
//	return EXIT_SUCCESS;
//}
//	switch (((LPNMHDR)lParam)->code)
//	{
//	case NM_RELEASEDCAPTURE:
//	{
//		OutputDebugString(L"NM_RELEASEDCAPTURE\n");
//		switch (((LPNMHDR)lParam)->idFrom)
//		{
//		case IDC_LVOLUME_CHNL1:
//		{
//			OutputDebugString(L"IDC_LVOLUME_CHNL1\n");
//			break;
//		} // eof IDC_SLIDER_LVOLUME
//		case IDC_RVOLUME_CHNL1:
//		{
//			OutputDebugString(L"IDC_RVOLUME_CHNL1\n");
//			break;
//		} // eof IDC_SLIDER_RVOLUME
//		} // eof switch
//		break;
//	}
//	} // eof switch

//BOOL onWmHscroll_DlgProc(const HWND& hDlg
//	, const WPARAM& wParam
//	, const LPARAM& lParam
//)
//{
//	OutputDebugString(L"WM_HSCROLL\n");
//	switch (LOWORD(wParam))
//	{
//	case TB_LINEDOWN:
//	{
//		OutputDebugString(L"TB_LINEDOWN\n");
//		break;
//	} // eof TB_LINEDOWN
//	case TB_LINEUP:
//	{
//		OutputDebugString(L"TB_LINEUP\n");
//		break;
//	} // eof TB_LINEUP
//	case TB_THUMBTRACK:
//	{
//		OutputDebugString(L"TB_THUMBTRACK\n");
//		break;
//	} // eof TB_THUMBTRACK
//	} // eof switch
//
//	return EXIT_SUCCESS;
//}
//	int track_pos = 0;
//	if (LOWORD(wParam) == TB_THUMBTRACK)
//	{
//		if ((HWND)lParam == GetDlgItem(hDlg, IDC_LVOLUME_CHNL1))
//		{
//			track_pos = SendMessage(GetDlgItem(hDlg, IDC_LVOLUME_CHNL1)
//				, TBM_GETPOS
//				, (WPARAM)0
//				, (LPARAM)0);
//
//			swprintf_s(wszBuffer
//				, (size_t)BUFFER_MAX
//				, L"%s %d %f\n"
//				, L"IDC_LVOLUME_CHNL1"
//				, track_pos
//				, (track_pos == 0) ? 0. : std::exp(track_pos / 100.f) / M_E
//			);
//			OutputDebugString(wszBuffer);
//			return EXIT_SUCCESS;
//		}
//		if ((HWND)lParam == GetDlgItem(hDlg, IDC_RVOLUME_CHNL1))
//		{
//			track_pos = SendMessage(GetDlgItem(hDlg, IDC_LVOLUME_CHNL1)
//				, TBM_GETPOS
//				, (WPARAM)0
//				, (LPARAM)0);
//
//			swprintf_s(wszBuffer
//				, (size_t)BUFFER_MAX
//				, L"%s %d %f\n"
//				, L"IDC_RVOLUME_CHNL1"
//				, track_pos
//				, (track_pos == 0) ? 0. : std::exp(track_pos / 100.f) / M_E
//			);
//			OutputDebugString(wszBuffer);
//			return EXIT_SUCCESS;
//		}
//	}
//	if (LOWORD(wParam) == TB_LINEDOWN)
//	{
//		OutputDebugString(L" TB_LINEDOWN\n");
//	}
//
//	return EXIT_SUCCESS;
//}

