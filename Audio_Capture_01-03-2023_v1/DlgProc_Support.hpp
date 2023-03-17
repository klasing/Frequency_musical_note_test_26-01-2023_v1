#pragma once
//*****************************************************************************
//*                     note
//*
//* TODO:
//* waveOutReset function
//* waveOutPause/waveOutRestart function
//* waveOutSetPitch function
//* waveOutSetPlaybackRate function
//* waveOutSetVolume function
//*****************************************************************************

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
DWORD g_dwVolume = 0;
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
		case IDC_RESET:
		{
			OutputDebugString(L"audio_playback IDC_RESET\n");
			// close wave out, triggers a MM_WOM_CLOSE message
			waveOutReset(g_hwo);
			break;
		} // eof IDC_PLAYBACK
		case IDC_PAUSE:
		{
			OutputDebugString(L"audio_playback IDC_PAUSE\n");
			// pause wave out
			waveOutPause(g_hwo);
			break;
		} // eof IDC_PAUSE
		case IDC_RESTART:
		{
			OutputDebugString(L"audio_playback IDC_RESTART\n");
			// continue wave out, after a pause
			waveOutRestart(g_hwo);
			break;
		} // eof IDC_RESTART
		case IDC_PITCH:
		{
			OutputDebugString(L"audio_playback IDC_PITCH\n");
			waveOutSetPitch(g_hwo
				, MAKELPARAM(0, 0xF)
			);
			break;
		} // eof IDC_PITCH
		case IDC_PLAYRATE:
		{
			OutputDebugString(L"audio_playback IDC_PLAYRATE\n");
			waveOutSetPlaybackRate(g_hwo
				, MAKELPARAM(0, 0xF)
			);
			break;
		} // eof IDC_PLAYRATE
		case IDC_LVOLUME:
		{
			OutputDebugString(L"audio_playback IDC_LVOLUME\n");
			waveOutSetVolume(g_hwo
				, MAKELPARAM(msg.lParam, msg.wParam)
			);
			break;
		} // eof IDC_LVOLUME
		case IDC_RVOLUME:
		{
			OutputDebugString(L"audio_playback IDC_RVOLUME\n");
			waveOutSetVolume(g_hwo
				, MAKELPARAM(msg.lParam, msg.wParam)
			);
			break;
		} // eof IDC_RVOLUME
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
	// get the device capabilities
	WAVEOUTCAPS woc{};
	waveOutGetDevCaps((UINT_PTR)g_hwo
		, &woc
		, sizeof(woc)
	);
	(woc.dwSupport & WAVECAPS_PITCH) ?
		OutputDebugString(L"support WAVECAPS_PITCH\n") :
		OutputDebugString(L"no support WAVECAPS_PITCH\n");
	(woc.dwSupport & WAVECAPS_PLAYBACKRATE) ?
		OutputDebugString(L"support WAVECAPS_PLAYBACKRATE\n") :
		OutputDebugString(L"no support WAVECAPS_PLAYBACKRATE\n");

	// set max. volume left and right 
	waveOutSetVolume(g_hwo, 0xFFFF'FFFF);
	// adjust slider to max. value
	SendMessage(GetDlgItem(g_hDlg, IDC_LVOLUME)
		, TBM_SETPOS
		, (WPARAM)TRUE
		, (LPARAM)100
	);
	SendMessage(GetDlgItem(g_hDlg, IDC_RVOLUME)
		, TBM_SETPOS
		, (WPARAM)TRUE
		, (LPARAM)100
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
	g_wfx.nSamplesPerSec = 48'000;
	//g_wfx.nSamplesPerSec = 44'100;
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
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_PITCH))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_PITCH)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0
			);

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d\n"
				, L"IDC_PITCH"
				, track_pos
			);
			OutputDebugString(wszBuffer);

			PostThreadMessage(g_dwAudioPlaybackId
				, IDC_PITCH
				, (WPARAM)0
				, (LPARAM)0
			);

			return (INT_PTR)TRUE;
		}
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_PLAYRATE))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_PLAYRATE)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0
			);

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d\n"
				, L"IDC_PLAYRATE"
				, track_pos
			);
			OutputDebugString(wszBuffer);

			PostThreadMessage(g_dwAudioPlaybackId
				, IDC_PLAYRATE
				, (WPARAM)0
				, (LPARAM)0
			);

			return (INT_PTR)TRUE;
		}
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_LVOLUME))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_LVOLUME)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0
			);

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d\n"
				, L"IDC_LVOLUME"
				, track_pos
			);
			OutputDebugString(wszBuffer);

			DWORD dwVolume = 0;
			waveOutGetVolume(g_hwo, &dwVolume);
			PostThreadMessage(g_dwAudioPlaybackId
				, IDC_LVOLUME
				, (WPARAM)(dwVolume & 0xFFFF'0000) >> 16
				, (LPARAM)((FLOAT)track_pos / 100.f * 0xFFFF)
			);

			return (INT_PTR)TRUE;
		} 
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_RVOLUME))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_RVOLUME)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0
			);

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d\n"
				, L"IDC_RVOLUME"
				, track_pos
			);
			OutputDebugString(wszBuffer);

			DWORD dwVolume = 0;
			waveOutGetVolume(g_hwo, &dwVolume);
			PostThreadMessage(g_dwAudioPlaybackId
				, IDC_RVOLUME
				, (WPARAM)((FLOAT)track_pos / 100.f * 0xFFFF)
				, (LPARAM)dwVolume & 0x0000'FFFF
			);

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
	HWND hWnd = NULL;
	switch (LOWORD(wParam))
	{
	case IDC_START_AUDIO_CAPTURE:
	{
		OutputDebugString(L"IDC_START_AUDIO_CAPTURE\n");
		hWnd = GetDlgItem(hDlg, IDC_START_AUDIO_CAPTURE);
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
		hWnd = GetDlgItem(hDlg, IDC_PLAYBACK);
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
				, (LPARAM)L"Reset"
			);
			start_audio_playback();
		}
		else
		{
			// change text on button
			SendMessage(hWnd
				, WM_SETTEXT
				, (WPARAM)0
				, (LPARAM)L"Start"
			);
			// force audio playback to reset
			PostThreadMessage(g_dwAudioPlaybackId
				, IDC_RESET
				, (WPARAM)0
				, (LPARAM)0
			);
		}
		return (INT_PTR)TRUE;
	} // eof IDC_PLAYBACK
	case IDC_PAUSE:
	{
		OutputDebugString(L"IDC_PAUSE\n");
		hWnd = GetDlgItem(hDlg, IDC_PAUSE);
		SendMessage(hWnd
			, WM_GETTEXT
			, (WPARAM)BUFFER_MAX
			, (LPARAM)wszBuffer
		);
		if (wcscmp(wszBuffer, L"Pause") == 0)
		{
			// change text on button
			SendMessage(hWnd
				, WM_SETTEXT
				, (WPARAM)0
				, (LPARAM)L"Restart"
			);
			// send audio playback a message to pause
			PostThreadMessage(g_dwAudioPlaybackId
				, IDC_PAUSE
				, (WPARAM)0
				, (LPARAM)0
			);
		}
		else
		{
			// change text on button
			SendMessage(hWnd
				, WM_SETTEXT
				, (WPARAM)0
				, (LPARAM)L"Pause"
			);
			// send audio playback a message to restart
			PostThreadMessage(g_dwAudioPlaybackId
				, IDC_RESTART
				, (WPARAM)0
				, (LPARAM)0
			);
		}
		return (INT_PTR)TRUE;
	} // eof IDC_PAUSE
	} // eof switch
	
	return (INT_PTR)FALSE;
}

