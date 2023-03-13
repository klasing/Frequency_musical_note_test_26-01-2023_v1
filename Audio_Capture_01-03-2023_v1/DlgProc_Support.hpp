#pragma once
//*****************************************************************************
//*                     include
//*****************************************************************************
#include "framework.h"
#include "Audio_Capture_01-03-2023_v1.h"

//****************************************************************************
//*                     global
//****************************************************************************
WCHAR wszBuffer[BUFFER_MAX] = { '\0' };
extern HWND g_hDlg;
WAVEFORMATEX g_wfx{};
// audio capture
HANDLE hAudioCapture = NULL;
DWORD g_dwAudioCaptureId = 0;
HWAVEIN g_hwi{};
LPWAVEHDR g_whi[MAX_BUFFERS]{};
UINT32 g_cBufferIn = 0;
BOOL g_bStopAudioCapture = FALSE;
// audio playback
HANDLE hAudioPlayback = NULL;
DWORD g_dwAudioPlaybackId = 0;
HWAVEOUT g_hwo{};
LPWAVEHDR g_who[PLAY_MAX_BUFFERS]{};
DWORD g_nBlock = 0;
DWORD g_cBufferOut = 0;
VOID* g_pPlaybackBuffer[PLAY_MAX_BUFFERS]{};
DWORD g_dwSizeRead = 0;

//*****************************************************************************
//*                     audio_capture
//*****************************************************************************
DWORD WINAPI audio_capture(LPVOID lpVoid)
{
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		switch (msg.message)
		{
		case MM_WIM_OPEN:
		{
			OutputDebugString(L"audio_capture MM_WIM_OPEN\n");
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
			break;
		} // eof MM_WIM_OPEN
		case MM_WIM_DATA:
		{
			OutputDebugString(L"audio_capture MM_WIM_DATA\n");
			if (!g_bStopAudioCapture)
			{
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
			}
			else
			{
				// mark all pending buffers as done
				waveInReset(g_hwi);
				// stop audio capture
				waveInStop(g_hwi);
				// trigger MM_WIM_CLOSE message
				waveInClose(g_hwi);
			}
			break;
		} // eof MM_WIM_DATA
		case MM_WIM_CLOSE:
		{
			OutputDebugString(L"audio_capture MM_WIM_CLOSE\n");
			closeWaveFile();
			for (int i = 0; i < MAX_BUFFERS; i++)
			{
				g_whi[i] = NULL;
			}
			g_hwi = NULL;
			g_bStopAudioCapture = FALSE;
			break;
		} // eof MM_WIM_CLOSE
		} // eof switch
	}

	return 0;
}

//*****************************************************************************
//*                     start_audio_capture
//*****************************************************************************
BOOL start_audio_capture()
{
	OutputDebugString(L"start_audio_capture()\n");
	// start thread audio_capture
	hAudioCapture = CreateThread(NULL
		, 0
		, audio_capture
		, (LPVOID)nullptr
		, 0 // run immediately
		, &g_dwAudioCaptureId
	);
	rc = waveInOpen(&g_hwi
		, STEREO_MIX
		, &g_wfx
		, (DWORD)g_dwAudioCaptureId
		, (DWORD)0
		, CALLBACK_THREAD
	);
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
			// open .wav file
			openWaveFile((const LPWSTR)L"wav_file.wav"
				, &g_wfx
				, WAVEFILE_READ
			);

			g_nBlock = getSizeWaveFile() / DATABLOCK_SIZE;
			for (UINT32 i = 0; i < g_nBlock; i++)
			{
				g_pPlaybackBuffer[i] = new BYTE[DATABLOCK_SIZE];
				hr = readWaveFile((BYTE*)g_pPlaybackBuffer[i]
					, DATABLOCK_SIZE
					, &g_dwSizeRead
				);

				g_ck.dwDataOffset += g_dwSizeRead;

				g_who[i] = new WAVEHDR;
				g_who[i]->lpData = (LPSTR)g_pPlaybackBuffer[i];
				g_who[i]->dwBufferLength = DATABLOCK_SIZE;
				g_who[i]->dwFlags = 0;
				g_who[i]->dwLoops = 0;

				waveOutPrepareHeader(g_hwo, g_who[i], sizeof(WAVEHDR));
			}

			g_cBufferOut = 0;
			while (g_cBufferOut < g_nBlock)
			{
				waveOutWrite(g_hwo, g_who[g_cBufferOut++], sizeof(WAVEHDR));
			}

			break;
		} // eof MM_WOM_OPEN
		case MM_WOM_DONE:
		{
			OutputDebugString(L"audio_playback MM_WOM_DONE\n");
			if (g_cBufferOut == g_nBlock) 
				// all blocks are played
				waveOutClose(g_hwo);
			break;
		} // eof MM_WOM_DONE
		case MM_WOM_CLOSE:
		{
			OutputDebugString(L"audio_playback MM_WOM_CLOSE\n");
			for (int i = 0; i < g_nBlock; i++)
			{
				g_who[i] = NULL;
			}
			g_hwo = NULL;
			g_cBufferOut = 0;
			closeWaveFile();
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
	hAudioPlayback = CreateThread(NULL
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
	OutputDebugString(L"onWmSize_DlgProc\n");
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
			// force audio capture to stop
			g_bStopAudioCapture = TRUE;
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

