#pragma once
//*****************************************************************************
//*                     note
//*
//* the A string of a guitar is normally tuned to 110 Hz (A2 midi# 45)
//* => the low E string of a guitar is E2, midi# 40
//*
//*****************************************************************************

//*****************************************************************************
//*                     include
//*****************************************************************************
#include "framework.h"
#include "Karplus_Strong_Algo_25-03-2023_v1.h"

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
//*                     extern
//****************************************************************************
extern HWND g_hDlg;

//****************************************************************************
//*                     global
//****************************************************************************
MMRESULT rc = MMSYSERR_NOERROR;
Note g_oNote;

// common
WAVEFORMATEX g_wfx{};

// audio playback
HANDLE g_hAudioPlayback = NULL;
DWORD g_dwAudioPlaybackId = 0;
HWAVEOUT g_hwo{};
DWORD g_nBlock = 0;
VOID* g_pPlaybackBuffer[SAMPLE_RATE / DATABLOCK_SIZE]{};
LPWAVEHDR g_who[SAMPLE_RATE / DATABLOCK_SIZE]{};
VOID* g_pPlaybackBuffer_KS[MAX_DATABLOCK]{};
LPWAVEHDR g_who_KS[MAX_DATABLOCK]{};
float g_fData[SAMPLE_RATE];
DWORD g_cBufferOut = 0;

std::mt19937_64 g_engine;
std::uniform_real_distribution<float> g_float_dist;

std::vector<float> g_ks{};

//*****************************************************************************
//*                     prepare_header1
//*****************************************************************************
BOOL
prepare_header1()
{
	OutputDebugString(L"prepare_header1()\n");

	float g_frequency_hz = g_oNote.aFreq[69];
	float delta = 2.f * g_frequency_hz * float(M_PI / SAMPLE_RATE);
	float phase = 0.f;
	g_nBlock = SAMPLE_RATE / DATABLOCK_SIZE;
	for (UINT32 i = 0; i < g_nBlock; i++)
	{
		g_pPlaybackBuffer[i] = new BYTE[DATABLOCK_SIZE];

		// NOISE
		//for (int j = 0; j < DATABLOCK_SIZE; j++)
		//{
		//	g_fData[j] = 0x7F * g_float_dist(g_engine);
		//	*((BYTE*)g_pPlaybackBuffer[i] + j) = g_fData[j];
		//}
		// PURE SINE WAVE
		for (int j = 0; j < DATABLOCK_SIZE - 1; j += 2)
		{
			float next_sample = 0x7F * std::sin(phase);
			phase = std::fmod(phase + delta, 2.f * static_cast<float>(M_PI));
			*((BYTE*)g_pPlaybackBuffer[i] + j) = next_sample;
			*((BYTE*)g_pPlaybackBuffer[i] + j + 1) = next_sample;
		}

		g_who[i] = new WAVEHDR;
		g_who[i]->lpData = (LPSTR)g_pPlaybackBuffer[i];
		g_who[i]->dwBufferLength = DATABLOCK_SIZE;
		g_who[i]->dwFlags = 0;
		g_who[i]->dwLoops = 0;

		waveOutPrepareHeader(g_hwo, g_who[i], sizeof(WAVEHDR));
	}
	// start playing
	g_cBufferOut = 0;
	while (g_cBufferOut < g_nBlock)
	{
		waveOutWrite(g_hwo, g_who[g_cBufferOut++], sizeof(WAVEHDR));
	}

	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     prepare_header2
//*****************************************************************************
BOOL
prepare_header2(const float& frequency_hz)
{
	OutputDebugString(L"prepare_header2()\n");

	// A4 = 440 Hz, midi# 69
	//float frequency_hz = g_oNote.aFreq[69];
	//  2 channel
	// 16 bit / (channel sample)
	//  2 * 16 = 32 bit / sample
	// (32 bit / sample) / 8 bit = 4 byte / sample
	int nofBytePerSample = 4;
	UINT max_buffer = nofBytePerSample * round(SAMPLE_RATE / frequency_hz);
	float nofOfSecondToPlay = .5;
	g_nBlock = round(nofOfSecondToPlay * frequency_hz);
	// reserve g_nBlock buffer
	std::vector<float>* pVectorBuffer = new std::vector<float>[g_nBlock];
	pVectorBuffer[0].resize(max_buffer);
	// initialize first buffer with random values
	// -1.0 < value < 1.0
	for (UINT i = 0; i < max_buffer; i++)
	{
		pVectorBuffer[0][i] = 2. * g_float_dist(g_engine) - 1.;
	}
	// set first Karplus-Strong waveform into a byte buffer
	g_pPlaybackBuffer_KS[0] = new BYTE[max_buffer];
	for (UINT i = 0; i < max_buffer; i++)
	{
		*((BYTE*)g_pPlaybackBuffer_KS[0] + i) = 0x7F * pVectorBuffer[0][i];
	}
	g_who_KS[0] = new WAVEHDR;
	g_who_KS[0]->lpData = (LPSTR)g_pPlaybackBuffer_KS[0];
	g_who_KS[0]->dwBufferLength = max_buffer;
	g_who_KS[0]->dwFlags = 0;
	g_who_KS[0]->dwLoops = 0;
	waveOutPrepareHeader(g_hwo, g_who_KS[0], sizeof(WAVEHDR));

	// iterate over the following buffer and
// synthesize the Karplus-Strong waveform
	float decay_factor = 0.992;
	float previous_value = 0.;
	for (int j = 1; j < g_nBlock; j++)
	{
		pVectorBuffer[j].resize(max_buffer);
		for (UINT i = 0; i < max_buffer; i++)
		{
			pVectorBuffer[j][i] = decay_factor
				* .5 * (pVectorBuffer[j - 1][i] + previous_value);
			previous_value = pVectorBuffer[j - 1][i];
		}
		// set Karplus-Strong waveform into a byte buffer
		g_pPlaybackBuffer_KS[j] = new BYTE[max_buffer];
		for (UINT i = 0; i < max_buffer; i++)
		{
			*((BYTE*)g_pPlaybackBuffer_KS[j] + i) = 0x7F * pVectorBuffer[j][i];
		}
		g_who_KS[j] = new WAVEHDR;
		g_who_KS[j]->lpData = (LPSTR)g_pPlaybackBuffer_KS[j];
		g_who_KS[j]->dwBufferLength = max_buffer;
		g_who_KS[j]->dwFlags = 0;
		g_who_KS[j]->dwLoops = 0;

		waveOutPrepareHeader(g_hwo, g_who_KS[j], sizeof(WAVEHDR));
	}

	// start playing
	g_cBufferOut = 0;
	while (g_cBufferOut < g_nBlock)
	{
		waveOutWrite(g_hwo, g_who_KS[g_cBufferOut++], sizeof(WAVEHDR));
	}

	// clean up
	pVectorBuffer[0].~vector();
	delete[] pVectorBuffer;

	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     audio_playback
//*****************************************************************************
DWORD WINAPI audio_playback(LPVOID lpVoid)
{
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		switch (msg.message)
		{
		case MM_WOM_OPEN:
		{
			OutputDebugString(L"audio_playback MM_WOM_OPEN\n");

			//prepare_header1();
			prepare_header2(g_oNote.aFreq[40]); // E2
			prepare_header2(g_oNote.aFreq[45]); // A2
			prepare_header2(g_oNote.aFreq[50]); // D3
			prepare_header2(g_oNote.aFreq[55]); // G3
			prepare_header2(g_oNote.aFreq[59]); // B3
			prepare_header2(g_oNote.aFreq[64]); // E4
			prepare_header2(g_oNote.aFreq[59]); // B3
			prepare_header2(g_oNote.aFreq[55]); // G3
			prepare_header2(g_oNote.aFreq[50]); // D3
			prepare_header2(g_oNote.aFreq[45]); // A2
			prepare_header2(g_oNote.aFreq[40]); // E2

			break;
		} // eof MM_WOM_OPEN
		case MM_WOM_DONE:
		{
			OutputDebugString(L"audio_playback MM_WOM_DONE\n");

			waveOutUnprepareHeader(g_hwo, g_who[g_cBufferOut], sizeof(WAVEHDR));
			if (g_cBufferOut == g_nBlock)
			{
				// all blocks are played
				waveOutClose(g_hwo);
			}

			break;
		} // eof MM_WOM_DONE
		case MM_WOM_CLOSE:
		{
			OutputDebugString(L"audio_playback MM_WOM_CLOSE\n");

			// let this thread die
			return 0;
		} // eof MM_WOM_CLOSE
		} // eof switch
	}

	return 0;
}

//*****************************************************************************
//*                     start_audio_playback
//*****************************************************************************
BOOL start_audio_playback()
{
	OutputDebugString(L"start_audio_playback()\n");

	// start thread audio_playback
	g_hAudioPlayback = CreateThread(NULL
		, 0
		, audio_playback
		, (LPVOID)nullptr
		, 0 // run immediately
		, &g_dwAudioPlaybackId
	);
	// open wave out
	rc = waveOutOpen(&g_hwo
		, SPEAKER_HEADPHONE
		, &g_wfx
		, (DWORD)g_dwAudioPlaybackId
		, (DWORD)0
		, CALLBACK_THREAD
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
	OutputDebugString(L"onWmInitDialog_DlgProc\n");

	// initialize waveformat
	g_wfx.nChannels = 2;
	// make compatible with project Mixer_Fmnt_26-01-2023_v1
	g_wfx.nSamplesPerSec = SAMPLE_RATE;
	g_wfx.wFormatTag = WAVE_FORMAT_PCM;
	g_wfx.wBitsPerSample = 16;
	g_wfx.nBlockAlign = g_wfx.nChannels * g_wfx.wBitsPerSample / 8;
	g_wfx.nAvgBytesPerSec = g_wfx.nSamplesPerSec * g_wfx.nBlockAlign;
	g_wfx.cbSize = 0;

	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     onWmSize_DlgProc
//*****************************************************************************
BOOL onWmSize_DlgProc(const HWND& hDlg
)
{
	OutputDebugString(L"onWmSize_DlgProc\n");

	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     onWmPaint_DlgProc
//*****************************************************************************
BOOL onWmPaint_DlgProc(const HWND& hDlg
)
{
	OutputDebugString(L"onWmPaint_DlgProc\n");

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hDlg, &ps);
	// TODO: Add any drawing code that uses hdc here...

	Graphics graphics(hdc);
	Pen      pen(Color(0xFF, 0, 0, 0));
	graphics.DrawLine(&pen, 35, 15, 35, 160);

	graphics.DrawLine(&pen, 35, 20, 500, 20);
	graphics.DrawLine(&pen, 35, 45, 500, 45);
	graphics.DrawLine(&pen, 35, 70, 500, 70);
	graphics.DrawLine(&pen, 35, 95, 500, 95);
	graphics.DrawLine(&pen, 35, 120, 500, 120);
	graphics.DrawLine(&pen, 35, 145, 500, 145);

	EndPaint(hDlg, &ps);

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
	OutputDebugString(L"onWmCommand_DlgProc()\n");

	switch (LOWORD(wParam))
	{
	case IDC_PLUCK:
	{
		OutputDebugString(L"IDC_PLUCK\n");

		start_audio_playback();

		return (INT_PTR)TRUE;
	} // eof IDC_PLUCK
	case IDC_STRUM:
	{
		OutputDebugString(L"IDC_STRUM\n");

		return (INT_PTR)TRUE;
	} // eof IDC_STRUM
	} // eof switch

	return (INT_PTR)FALSE;
}

// waste /////////////////////////////////////////////////////////////////////
//****************************************************************************
//*                     karplus_strong
//*
//* https://www.math.drexel.edu/~dp399/musicmath/Karplus-Strong.html
//****************************************************************************
/*
BOOL
karplus_strong(const float& frequency_hz
	, const INT& nPeriod
)
{
	OutputDebugString(L"karplus_strong()\n");

	std::vector<float> wave_table;
	// initialize ring buffer
	UINT32 max_wavetable = round(SAMPLE_RATE / frequency_hz);
	wave_table.resize(max_wavetable);
	for (UINT32 i = 0; i < max_wavetable; i++)
	{
		wave_table.at(i) = 2. * g_float_dist(g_engine) - 1.;
	}
	// synthesize Karplus-Strong waveform
	UINT32 current_sample = 0;
	float previous_value = 0.f;
	//std::vector<float> samples{};
	while (g_samples.size() < 4 * max_wavetable)
	{
		wave_table.at(current_sample) = .5
			* (wave_table.at(current_sample) + previous_value);
		g_samples.push_back(wave_table[current_sample]);
		previous_value = (current_sample >= 1) ?
			g_samples[current_sample - 1] : 0.;
		++current_sample;
		current_sample = current_sample % max_wavetable;
	}
	return EXIT_SUCCESS;
}
*/
/*
	// test a ringbuffer
	std::vector<float> ring_buffer;
	ring_buffer.resize(MAX_RINGBUFFER);
	for (int i = 0; i < 15; i++)
	{
		int idx = i % MAX_RINGBUFFER;
		ring_buffer.at(idx) = (float)i;
	}
	float energy_decay_factor = .7;// .994;
	// test feedback mechanism
	float value_feedback = 0.;
	do
	{
		value_feedback = energy_decay_factor
			* .5 * (ring_buffer[0] + ring_buffer[1]);
		// remove the first element of the ring_buffer
		ring_buffer.erase(ring_buffer.begin());
		// add the feedback value to the ring_buffer
		ring_buffer.push_back(value_feedback);
	} while (value_feedback > 3.);
*/

