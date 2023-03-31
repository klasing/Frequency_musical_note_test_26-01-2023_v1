#pragma once
//*****************************************************************************
//*                     note
//*
//* the A string of a guitar is normally tuned to 110 Hz (A midi# 45)
//* => the low E string of a guitar is 82,41 Hz (E midi# 40)
//*
//* E  82,41 Hz midi# 40
//* A 110,00 Hz midi# 45
//* D 146,83 Hz midi# 50
//* G 196,00 Hz midi# 55
//* B 246,94 Hz midi# 59
//* E 329,63 Hz midi# 64
//*
//*  2 channel
//* 16 bit / (channel sample)
//*  2 * 16 = 32 bit / sample
//* (32 bit / sample) / 8 bit = 4 byte / sample
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
//*                     enum
//****************************************************************************
enum nMidi
{
	E = 40,
	A = 45,
	D = 50,
	G = 55,
	B = 59,
	e = 64
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
auto g_pVectorBuffer = std::unique_ptr<std::vector<float>[]>(
	new std::vector<float>[MAX_DATABLOCK]);
BYTE** g_pPlaybackBuffer = nullptr;
LPWAVEHDR g_who[MAX_DATABLOCK]{};
DWORD g_cBufferOut = 0;

std::mt19937_64 g_engine;
std::uniform_real_distribution<float> g_float_dist;

//*****************************************************************************
//*                     audio_playback
//*****************************************************************************
BOOL
pluck_prepare_header(const float& frequency_hz)
{
	OutputDebugString(L"pluck_prepare_header\n");

	float nofOfSecondToPlay = 1.5;
	// g_wfx.nBlockAlign: nof bytes per sample = 4
	UINT max_buffer = g_wfx.nBlockAlign * round(SAMPLE_RATE / frequency_hz);
	g_nBlock = round(nofOfSecondToPlay * frequency_hz);

	// initialize first buffer with random values
	// -1.0 < value < 1.0
	g_pVectorBuffer[0].resize(max_buffer);
	g_pPlaybackBuffer = new BYTE*[MAX_DATABLOCK];
	g_pPlaybackBuffer[0] = new BYTE[max_buffer];
	for (UINT i = 0; i < max_buffer; i++)
	{
		g_pVectorBuffer[0][i] = 2. * g_float_dist(g_engine) - 1.;
		g_pPlaybackBuffer[0][i] = 0x7F * g_pVectorBuffer[0][i];
	}
	g_who[0] = new WAVEHDR;
	g_who[0]->lpData = (LPSTR)g_pPlaybackBuffer[0];
	g_who[0]->dwBufferLength = max_buffer;
	g_who[0]->dwFlags = 0;
	g_who[0]->dwLoops = 0;
	waveOutPrepareHeader(g_hwo, g_who[0], sizeof(WAVEHDR));

	// iterate over the following buffer and
	// synthesize the Karplus-Strong waveform
	float decay_factor = 0.980;
	float previous_value = 0.;
	for (int j = 1; j < g_nBlock; j++)
	{
		g_pVectorBuffer[j].resize(max_buffer);
		g_pPlaybackBuffer[j] = new BYTE[max_buffer];
		for (UINT i = 0; i < max_buffer; i++)
		{
			g_pVectorBuffer[j][i] = decay_factor
				* .5 * (g_pVectorBuffer[j - 1][i] + previous_value);
			previous_value = g_pVectorBuffer[j - 1][i];
			g_pPlaybackBuffer[j][i] = 0x7F * g_pVectorBuffer[j][i];
		}
		g_who[j] = new WAVEHDR;
		g_who[j]->lpData = (LPSTR)g_pPlaybackBuffer[j];
		g_who[j]->dwBufferLength = max_buffer;
		g_who[j]->dwFlags = 0;
		g_who[j]->dwLoops = 0;
		waveOutPrepareHeader(g_hwo, g_who[j], sizeof(WAVEHDR));
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

			// disable button IDC_PAUSE
			EnableWindow(GetDlgItem(g_hDlg, IDC_PLUCK), FALSE);

			//pluck_prepare_header(g_oNote.aFreq[E]);
			pluck_prepare_header(g_oNote.aFreq[A]);
			//pluck_prepare_header(g_oNote.aFreq[D]);
			//pluck_prepare_header(g_oNote.aFreq[G]);
			//pluck_prepare_header(g_oNote.aFreq[B]);
			//pluck_prepare_header(g_oNote.aFreq[e]);

			break;
		} // eof MM_WOM_OPEN
		case MM_WOM_DONE:
		{
			OutputDebugString(L"audio_playback MM_WOM_DONE\n");

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

			// clean up
			for (int i = 0; i < g_nBlock; i++)
			{
				delete[] g_pPlaybackBuffer[i];
			}
			delete[] g_pPlaybackBuffer;

			// enable button IDC_PAUSE
			EnableWindow(GetDlgItem(g_hDlg, IDC_PLUCK), TRUE);

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

// waste //////////////////////////////////////////////////////////////////////
/*
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

//std::vector<float>* g_pVectorBuffer = nullptr;
auto g_pVectorBuffer_ = std::unique_ptr<std::vector<float>[]>(
	new std::vector<float>[MAX_DATABLOCK]);

//BYTE g_playbackBuffer_KS_[MAX_DATABLOCK];
//auto g_pPlaybackBuffer_KS_ = std::unique_ptr<VOID>(
//	new BYTE[MAX_DATABLOCK]);// std::unique_ptr<VOID>);

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
	//g_pVectorBuffer = new std::vector<float>[g_nBlock];
	g_pVectorBuffer_[0].resize(max_buffer);
	//g_pVectorBuffer[0].resize(max_buffer);
	// initialize first buffer with random values
	// -1.0 < value < 1.0
	for (UINT i = 0; i < max_buffer; i++)
	{
		g_pVectorBuffer_[0][i] = 2. * g_float_dist(g_engine) - 1.;
		//g_pVectorBuffer[0][i] = 2. * g_float_dist(g_engine) - 1.;
	}
	// set first Karplus-Strong waveform into a byte buffer
	g_pPlaybackBuffer_KS[0] = new BYTE[max_buffer];
	for (UINT i = 0; i < max_buffer; i++)
	{
		*((BYTE*)g_pPlaybackBuffer_KS[0] + i) = 0x7F * g_pVectorBuffer_[0][i];
		//*((BYTE*)g_pPlaybackBuffer_KS[0] + i) = 0x7F * g_pVectorBuffer[0][i];
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
		g_pVectorBuffer_[j].resize(max_buffer);
		//g_pVectorBuffer[j].resize(max_buffer);
		for (UINT i = 0; i < max_buffer; i++)
		{
			g_pVectorBuffer_[j][i] = decay_factor
				* .5 * (g_pVectorBuffer_[j - 1][i] + previous_value);
			previous_value = g_pVectorBuffer_[j - 1][i];
			//g_pVectorBuffer[j][i] = decay_factor
			//	* .5 * (g_pVectorBuffer[j - 1][i] + previous_value);
			//previous_value = g_pVectorBuffer[j - 1][i];
		}
		// set Karplus-Strong waveform into a byte buffer
		g_pPlaybackBuffer_KS[j] = new BYTE[max_buffer];
		for (UINT i = 0; i < max_buffer; i++)
		{
			*((BYTE*)g_pPlaybackBuffer_KS[j] + i) = 0x7F * g_pVectorBuffer_[j][i];
			//*((BYTE*)g_pPlaybackBuffer_KS[j] + i) = 0x7F * g_pVectorBuffer[j][i];
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

			//waveOutUnprepareHeader(g_hwo, g_who[g_cBufferOut], sizeof(WAVEHDR));
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

			// clean up
			for (auto n = 0; n < g_nBlock; n++)
			{
				g_pVectorBuffer_[n].clear();
				g_pVectorBuffer_[n].resize(0);
				delete[] g_pPlaybackBuffer_KS[n];
				delete g_who_KS[n];
			}
			g_pVectorBuffer_.reset();
			//delete[] g_pVectorBuffer;
			//g_pVectorBuffer = nullptr;

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
*/