#pragma once
//*****************************************************************************
//*                     include
//*****************************************************************************
#include "framework.h"
#include "Audio_Capture_01-03-2023_v2.h"

//****************************************************************************
//*                     global
//****************************************************************************
WCHAR wszBuffer[BUFFER_MAX] = { '\0' };
extern HWND g_hDlg;
WAVEFORMATEX g_wfx{};
// audio capture
HWAVEIN g_hwi{};
LPWAVEHDR g_whi[MAX_BUFFERS]{};
UINT32 g_cBufferIn = 0;
// audio playback
HWAVEOUT g_hwo{};
LPWAVEHDR g_who[PLAY_MAX_BUFFERS]{};
//LPWAVEHDR g_who[MAX_BUFFERS]{};
VOID* g_pPlaybackBuffer[PLAY_MAX_BUFFERS]{};
//VOID* g_pPlaybackBuffer[MAX_BUFFERS]{};
UINT g_cBufferOut = 0;
UINT g_nBlock = 0;
UINT g_cFreeBuffer = 0;

//*****************************************************************************
//*                     start_audio_capture
//*****************************************************************************
BOOL start_audio_capture()
{
	OutputDebugString(L"start_audio_capture()\n");
	// trigger a MM_WIM_OPEN message, which
	// results in calling func onWimOpen_DlgProc()
	rc = waveInOpen(&g_hwi
		, STEREO_MIX
		, &g_wfx
		, (DWORD)g_hDlg
		, (DWORD)0
		, CALLBACK_WINDOW
	);
	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     start_audio_playback
//*****************************************************************************
BOOL start_audio_playback()
{
	OutputDebugString(L"start_audio_playback()\n");
	// trigger a MM_WOM_OPEN message, which
	// results in calling func onWomOpen_DlgProc()
	rc = waveOutOpen(&g_hwo
		, SPEAKER_HEADPHONE
		, &g_wfx
		, (DWORD)g_hDlg
		, (DWORD)0
		, CALLBACK_WINDOW
	);
	return EXIT_SUCCESS;
}
//if (rc == MMSYSERR_ALLOCATED) OutputDebugString(L"MMSYSERR_ALLOCATED\n");
//if (rc == MMSYSERR_BADDEVICEID) OutputDebugString(L"MMSYSERR_BADDEVICEID\n");
//if (rc == MMSYSERR_NODRIVER) OutputDebugString(L"MMSYSERR_NODRIVER\n");
//if (rc == MMSYSERR_NOMEM) OutputDebugString(L"MMSYSERR_NOMEM\n");
//if (rc == WAVERR_BADFORMAT) OutputDebugString(L"WAVERR_BADFORMAT\n");
//if (rc == WAVERR_SYNC) OutputDebugString(L"WAVERR_SYNC\n");

//*****************************************************************************
//*                     onWmInitDialog_DlgProc
//*****************************************************************************
BOOL onWmInitDialog_DlgProc(const HINSTANCE& hInst
	, const HWND& hDlg
)
{
	// initialize waveformat
	g_wfx.nChannels = 2;
	// 44.100 samples/s
	g_wfx.nSamplesPerSec = 44'100;
	//g_wfx.nSamplesPerSec = 48'000;
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
	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     onWimOpen_DlgProc
//*****************************************************************************
BOOL onWimOpen_DlgProc()
{
	OutputDebugString(L"onWimOpen_DlgProc()\n");
	// open .wav file
	openWaveFile((const LPWSTR)L"wav_file.wav"
		, &g_wfx
		, WAVEFILE_WRITE
	);
	for (int i = 0; i < MAX_BUFFERS; i++)
	{
		// allocate buffer
		g_whi[i] = new WAVEHDR;
		if (g_whi[i])
		{
			g_whi[i]->lpData = new char[DATABLOCK_SIZE];
			g_whi[i]->dwBufferLength = DATABLOCK_SIZE;
			g_whi[i]->dwFlags = 0;
		}
		// prepare buffer
		waveInPrepareHeader(g_hwi
			, g_whi[i]
			, sizeof(WAVEHDR)
		);
		// add to input queue
		waveInAddBuffer(g_hwi, g_whi[i], sizeof(WAVEHDR));
	}
	// start audio capture
	waveInStart(g_hwi);
	return EXIT_SUCCESS;
}
//*****************************************************************************
//*                     onWimData_DlgProc
//*****************************************************************************
BOOL onWimData_DlgProc()
{
	OutputDebugString(L"onWimData_DlgProc()\n");
	UINT nSizeWrote = 0;
	writeWaveFile(g_whi[g_cBufferIn]->dwBufferLength
		, (BYTE*)g_whi[g_cBufferIn]->lpData
		, &nSizeWrote
	);
	// prepare buffer and add to input queue
	waveInPrepareHeader(g_hwi
		, g_whi[g_cBufferIn]
		, sizeof(WAVEHDR)
	);
	waveInAddBuffer(g_hwi
		, g_whi[g_cBufferIn], sizeof(WAVEHDR)
	);
	// point to the next buffer
	g_cBufferIn = ++g_cBufferIn % MAX_BUFFERS;
	return EXIT_SUCCESS;
}
//*****************************************************************************
//*                     onWimClose_DlgProc
//*****************************************************************************
BOOL onWimClose_DlgProc()
{
	OutputDebugString(L"onWimClose_DlgProc()\n");
	closeWaveFile();
	for (int i = 0; i < MAX_BUFFERS; i++)
	{
		g_whi[i] = NULL;
	}
	g_hwi = NULL;
	return EXIT_SUCCESS;
}
//*****************************************************************************
//*                     onWomOpen_DlgProc
//*****************************************************************************
BOOL onWomOpen_DlgProc()
{
	OutputDebugString(L"onWomOpen_DlgProc()\n");
	// open .wav file
	openWaveFile((const LPWSTR)L"wav_file.wav"
		, &g_wfx
		, WAVEFILE_READ
	);

	g_nBlock = ((getSizeWaveFile() / DATABLOCK_SIZE) > PLAY_MAX_BUFFERS) ?
		PLAY_MAX_BUFFERS :
		getSizeWaveFile() / DATABLOCK_SIZE;

	DWORD dwSizeRead = 0;
	for (int i = 0; i < g_nBlock; i++)
	{
		g_pPlaybackBuffer[i] = new BYTE[DATABLOCK_SIZE];
		hr = readWaveFile((BYTE*)g_pPlaybackBuffer[i]
			, DATABLOCK_SIZE
			, &dwSizeRead
		);

		g_ck.dwDataOffset += dwSizeRead;

		g_who[i] = new WAVEHDR;
		g_who[i]->lpData = (LPSTR)g_pPlaybackBuffer[i];
		g_who[i]->dwBufferLength = DATABLOCK_SIZE;
		g_who[i]->dwFlags = 0;
		g_who[i]->dwLoops = 0;

		waveOutPrepareHeader(g_hwo, g_who[i], sizeof(WAVEHDR));
	}

	while (g_cBufferOut < g_nBlock)
	{
		waveOutWrite(g_hwo, g_who[g_cBufferOut++], sizeof(WAVEHDR));
	}
	return EXIT_SUCCESS;
}
//*****************************************************************************
//*                     onWomDone_DlgProc
//*****************************************************************************
BOOL onWomDone_DlgProc()
{
	swprintf_s(wszBuffer
		, (size_t)BUFFER_MAX
		, L"free: %d\n"
		, ++g_cFreeBuffer
	);
	OutputDebugString(wszBuffer);
	//OutputDebugString(L"onWomDone_DlgProc()\n");
	waveOutClose(g_hwo);
	return EXIT_SUCCESS;
}
//*****************************************************************************
//*                     onWomClose_DlgProc
//*****************************************************************************
BOOL onWomClose_DlgProc()
{
	OutputDebugString(L"onWomClose_DlgProc()\n");
	for (int i = 0; i < g_nBlock; i++)
	{
		g_whi[i] = NULL;
	}
	g_hwi = NULL;
	g_cBufferOut = 0;

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
	switch (LOWORD(wParam))
	{
	case IDC_START_AUDIO_CAPTURE:
	{
		OutputDebugString(L"IDC_START_AUDIO_CAPTURE\n");
		HWND hWnd = GetDlgItem(hDlg, IDC_START_AUDIO_CAPTURE);
		SendMessage(hWnd
			, WM_GETTEXT
			, (WPARAM)BUFFER_MAX
			, (LPARAM)wszBuffer
		);
		if (wcscmp(wszBuffer, L"Start") == 0)
		{
			// change text on button
			SendMessage(hWnd
				, WM_SETTEXT
				, (WPARAM)0
				, (LPARAM)L"Stop"
			);
			start_audio_capture();
		}
		else
		{
			// change text on button
			SendMessage(hWnd
				, WM_SETTEXT
				, (WPARAM)0
				, (LPARAM)L"Start"
			);
			// stop audio capture
			waveInStop(g_hwi);
			// mark all pending buffers as done
			waveInReset(g_hwi);
			// trigger a MM_WIM_CLOSE message, which
			// results in calling func onWimClose_DlgProc()
			waveInClose(g_hwi);
		}
		return (INT_PTR)TRUE;
	} // eof IDC_START_AUDIO_CAPTURE
	case IDC_PLAYBACK:
	{
		OutputDebugString(L"IDC_PLAYBACK\n");
		start_audio_playback();
		return (INT_PTR)TRUE;
	} // eof IDC_PLAYBACK
	} // eof switch
	return (INT_PTR)FALSE;
}
