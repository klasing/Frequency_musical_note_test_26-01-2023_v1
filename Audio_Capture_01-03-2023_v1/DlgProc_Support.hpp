#pragma once

//****************************************************************************
//*                     special purpose define
//****************************************************************************
#define RECORD_BUFFER_SIZE		32768L
#define DATABLOCK_SIZE			32768L
#define MAX_BUFFERS				2

//****************************************************************************
//*                     global
//****************************************************************************
WCHAR wszBuffer[BUFFER_MAX] = { '\0' };
extern HWND g_hDlg;

MMRESULT rc = MMSYSERR_NOERROR;
HRESULT hr = S_OK;

WAVEFORMATEX g_wfx{};

// audio capture
HWAVEIN g_hwi{};
LPWAVEHDR g_whi[MAX_BUFFERS];
UINT32 g_cBufferIn = 0;
// audio playback
HWAVEOUT g_hwo{};
LPWAVEHDR g_who[MAX_BUFFERS];
VOID* g_pPlaybackBuffer[MAX_BUFFERS];
UINT g_cBufferOut = 0;
//UINT g_nBlock = 0;

//****************************************************************************
//*                     waveInProc
//****************************************************************************
void CALLBACK waveInProc(HWAVEIN hwi
	, UINT uMsg
	, DWORD_PTR dwInstance
	, DWORD_PTR dwParam1
	, DWORD_PTR dwParam2
)
{
	switch (uMsg)
	{
	case WIM_OPEN:
	{
		// not used any further
		PostMessage(g_hDlg
			, WM_COMMAND
			, (WPARAM)uMsg
			, (LPARAM)0
		);
		break;
	} // eof WIM_OPEN
	case WIM_DATA:
	{
		PostMessage(g_hDlg
			, WM_COMMAND
			, (WPARAM)uMsg
			, (LPARAM)0
		);
		break;
	} // eof WIM_DATA
	case WIM_CLOSE:
	{
		PostMessage(g_hDlg
			, WM_COMMAND
			, (WPARAM)uMsg
			, (LPARAM)0
		);
		break;
	} // eof WIM_CLOSE
	} // eof switch

	return;
}

//****************************************************************************
//*                     waveOutProc
//****************************************************************************
void CALLBACK waveOutProc(HWAVEOUT hwo
	, UINT uMsg
	, DWORD_PTR dwInstance
	, DWORD_PTR dwParam1
	, DWORD_PTR dwParam2
)
{
	switch (uMsg)
	{
	case WOM_OPEN:
	{
		// not used any further
		PostMessage(g_hDlg
			, WM_COMMAND
			, (WPARAM)uMsg
			, (LPARAM)0
		);
		break;
	} // eof WOM_OPEN
	case WOM_DONE:
	{
		PostMessage(g_hDlg
			, WM_COMMAND
			, (WPARAM)uMsg
			, (LPARAM)0
		);
		break;
	} // eof WOM_DONE
	case WOM_CLOSE:
	{
		// not used any further
		PostMessage(g_hDlg
			, WM_COMMAND
			, (WPARAM)uMsg
			, (LPARAM)0
		);
		break;
	} // eof WOM_CLOSE
	} // eof switch

	return;
}

//*****************************************************************************
//*                     start_audio_capture
//*****************************************************************************
BOOL start_audio_capture()
{
	/////////////////////////////////////////////
	// make sure the device handle is not invalid
	if (g_hwi == NULL)
	{
		rc = waveInOpen(&g_hwi
			, 1 // TODO: this has to be addressed
			, &g_wfx
			, (DWORD)(VOID*)waveInProc
			, (DWORD)0
			, CALLBACK_FUNCTION
		);
	}
	/////////////////////////////////////////////
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
		// prepare buffer and add to input queue
		rc = waveInPrepareHeader(g_hwi
			, g_whi[i]
			, sizeof(WAVEHDR)
		);
		if (rc == MMSYSERR_NOERROR)
			rc = waveInAddBuffer(g_hwi, g_whi[i], sizeof(WAVEHDR));
	}
	// start audio capture
	waveInStart(g_hwi);

	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     start_playback
//*****************************************************************************
BOOL start_playback()
{
	//g_nBlock = getSizeWaveFile() / DATABLOCK_SIZE;
	DWORD dwSizeRead = 0;

	/////////////////////////////////////////////
	// make sure the device handle is not invalid
	if (g_hwo == NULL)
	{
		rc = waveOutOpen(&g_hwo
			, 0 // TODO: this has to be addressed
			, &g_wfx
			, (DWORD)(VOID*)waveOutProc
			, (DWORD)0
			, CALLBACK_FUNCTION
		);
	}
	/////////////////////////////////////////////

	for (int i = 0; i < MAX_BUFFERS; i++)
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
	}

	rc = waveOutPrepareHeader(g_hwo, g_who[0], sizeof(WAVEHDR));
	// start audio playback
	rc = waveOutWrite(g_hwo, g_who[0], sizeof(WAVEHDR));

	return EXIT_SUCCESS;
}
//*****************************************************************************
//*                     getAdioCaptureCap
//*****************************************************************************
BOOL getAdioCaptureCap()
{
	// get all devices with audio capture capability
	UINT nMaxDevices = waveInGetNumDevs();
	swprintf_s(wszBuffer
		, (size_t)BUFFER_MAX
		, L"input nof devices: %d\n"
		, nMaxDevices
	);
	OutputDebugString(wszBuffer);

	std::wstring wstrFormats = L"";
	std::wstring wstrTemplate =
		std::wstring(L"manufacturer id....: %d\n") +
		L"product id.........: %d\n"
		L"driver version.....: %d\n"
		L"product name.......: %s\n"
		L"standard formats...: %s\n";
	WAVEINCAPS wic{};
	for (UINT nDevId = 0; nDevId < nMaxDevices; nDevId++)
	{
		rc = waveInGetDevCaps(nDevId, &wic, sizeof(wic));
		if (rc == MMSYSERR_NOERROR)
		{
			if (wic.dwFormats & WAVE_FORMAT_4S08)
			{
				// 44.1 kHz, stereo, 8-bit
				wstrFormats = L"WAVE_FORMAT_4S08";
			}
			if (wic.dwFormats & WAVE_FORMAT_4S16)
			{
				// 44.1 kHz, stereo, 16-bit
				wstrFormats += L" | WAVE_FORMAT_4S16";
			}
			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, wstrTemplate.c_str()
				, wic.wMid
				, wic.wPid
				, (INT16)wic.vDriverVersion
				, wic.szPname
				, wstrFormats.c_str()
			);
			OutputDebugString(wszBuffer);

			// depends on sound settings
			// either one of the two is active
			// 1) use microphone array, nDevId = 0
			//if (nDevId == 0)
			// 2) use stereo mix, nDevId == 1
			if (nDevId == 1)
			{
				// open input device
				rc = waveInOpen(&g_hwi
					, nDevId
					, &g_wfx
					, (DWORD)(VOID*)waveInProc
					, (DWORD)0
					, CALLBACK_FUNCTION
				);
				if (rc == MMSYSERR_NOERROR)
				{
					OutputDebugString(L"mikes are ready to capture\n");
				}
			}
		}
	}

	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     getAudioPlaybackCap
//*****************************************************************************
BOOL getAudioPlaybackCap()
{
	// get all devices with audio playback capability
	UINT nMaxDevices = waveOutGetNumDevs();
	swprintf_s(wszBuffer
		, (size_t)BUFFER_MAX
		, L"output nof devices: %d\n"
		, nMaxDevices
	);
	OutputDebugString(wszBuffer);

	std::wstring wstrFormats = L"";
	std::wstring wstrTemplate =
		std::wstring(L"manufacturer id....: %d\n") +
		L"product id.........: %d\n"
		L"driver version.....: %d\n"
		L"product name.......: %s\n"
		L"standard formats...: %s\n";
	WAVEOUTCAPS woc{};
	for (UINT nDevId = 0; nDevId < nMaxDevices; nDevId++)
	{
		rc = waveOutGetDevCaps(nDevId, &woc, sizeof(woc));
		if (rc == MMSYSERR_NOERROR)
		{
			if (woc.dwFormats & WAVE_FORMAT_4S08)
			{
				// 44.1 kHz, stereo, 8-bit
				wstrFormats = L"WAVE_FORMAT_4S08";
			}
			if (woc.dwFormats & WAVE_FORMAT_4S16)
			{
				// 44.1 kHz, stereo, 16-bit
				wstrFormats += L" | WAVE_FORMAT_4S16";
			}
			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, wstrTemplate.c_str()
				, woc.wMid
				, woc.wPid
				, (INT16)woc.vDriverVersion
				, woc.szPname
				, wstrFormats.c_str()
			);
			OutputDebugString(wszBuffer);

			// use Speaker/Headphone, nDevId = 0
			if (nDevId == 0)
			{
				// open output device
				rc = waveOutOpen(&g_hwo
					, nDevId
					, &g_wfx
					// does not work
					//, (DWORD)g_hDlg
					, (DWORD)(VOID*)waveOutProc
					, (DWORD)0
					// does not work
					//, CALLBACK_WINDOW
					, CALLBACK_FUNCTION
				);
				if (rc == MMSYSERR_NOERROR)
				{
					OutputDebugString(L"speakers are ready to play\n");
				}
			}
		}
	}

	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     onWmInitDialog_DlgProc
//*****************************************************************************
BOOL onWmInitDialog_DlgProc(const HINSTANCE& hInst
	, const HWND& hDlg
)
{
	// initialize waveformat
	g_wfx.nChannels = 2;
	// 44.100 samples gives a slightly better result
	g_wfx.nSamplesPerSec = 44'100;// 48'000;
	g_wfx.wFormatTag = WAVE_FORMAT_PCM;
	g_wfx.wBitsPerSample = 16;
	g_wfx.nBlockAlign = g_wfx.nChannels * g_wfx.wBitsPerSample / 8;
	g_wfx.nAvgBytesPerSec = g_wfx.nSamplesPerSec * g_wfx.nBlockAlign;
	g_wfx.cbSize = 0;
	// get audio capture capability
	getAdioCaptureCap();
	// get audio playback capability
	getAudioPlaybackCap();

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
	, const LPARAM& lParam
)
{
	DWORD dwSizeRead = 0;
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
			// create .wav file
			hr = openWaveFile((LPWSTR)L"wav_file.wav"
				, &g_wfx
				, WAVEFILE_WRITE
			);
			// start audio capture
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
			rc = waveInStop(g_hwi);
			// mark all pending buffers as done
			rc = waveInReset(g_hwi);
			rc = waveInClose(g_hwi);
		}

		return (INT_PTR)TRUE;
	}
	case IDC_PLAYBACK:
	{
		OutputDebugString(L"IDC_PLAYBACK\n");

		// open .wav file
		hr = openWaveFile((LPWSTR)L"wav_file.wav"
			, &g_wfx
			, WAVEFILE_READ
		);
		// start playback
		start_playback();

		return (INT_PTR)TRUE;
	} // eof IDC_PLAYBACK

	// messages from audio callback procs
	// 1) audio capture
	case WIM_OPEN:
	{
		return (INT_PTR)TRUE;
	} // eof WIM_OPEN
	case WIM_DATA:
	{
		UINT nSizeWrote = 0;
		hr = writeWaveFile(g_whi[g_cBufferIn]->dwBufferLength
			, (BYTE*)g_whi[g_cBufferIn]->lpData
			, &nSizeWrote
		);
		// prepare buffer and add to input queue
		rc = waveInPrepareHeader(g_hwi
			, g_whi[g_cBufferIn]
			, sizeof(WAVEHDR)
		);
		rc = waveInAddBuffer(g_hwi
			, g_whi[g_cBufferIn], sizeof(WAVEHDR)
		);
		// point to the next buffer
		g_cBufferIn = ++g_cBufferIn % MAX_BUFFERS;

		return (INT_PTR)TRUE;
	} // eof WIM_DATA
	case WIM_CLOSE:
	{
		// prevent click at the end of a capture
		Sleep(2000);
		hr = closeWaveFile();
		// set variables to default
		g_hwi = NULL;
		g_whi[0] = NULL;
		g_whi[1] = NULL;
		return (INT_PTR)TRUE;
	} // eof WIM_CLOSE

	// 2) audio playback
	case WOM_OPEN:
	{
		return (INT_PTR)TRUE;
	} // eof WOM_OPEN
	case WOM_DONE:
	{
		if (g_ck.dwDataOffset >= getSizeWaveFile())
		{
			waveOutClose(g_hwo);
			return (INT_PTR)TRUE;
		}

		waveOutUnprepareHeader(g_hwo, g_who[g_cBufferOut], sizeof(WAVEHDR));
		hr = readWaveFile((BYTE*)g_pPlaybackBuffer[g_cBufferOut]
			, DATABLOCK_SIZE
			, &dwSizeRead
		);
		// advance to next data block in wave file
		g_ck.dwDataOffset += dwSizeRead;
		g_who[g_cBufferOut]->lpData = (LPSTR)g_pPlaybackBuffer[g_cBufferOut];

		g_cBufferOut = ++g_cBufferOut % MAX_BUFFERS;
		waveOutPrepareHeader(g_hwo, g_who[g_cBufferOut], sizeof(WAVEHDR));
		waveOutWrite(g_hwo, g_who[g_cBufferOut], sizeof(WAVEHDR));

		return (INT_PTR)TRUE;
	} // eof WOM_DONE
	case WOM_CLOSE:
	{
		hr = closeWaveFile();
		// set variables to default
		g_hwo = NULL;
		g_who[0] = NULL;
		g_who[1] = NULL;
		return (INT_PTR)TRUE;
	} // eof WOM_CLOSE
	} // eof switch
	
	return (INT_PTR)FALSE;
}

/*
// Global variables.

HANDLE hData  = NULL;  // handle of waveform data memory
HPSTR  lpData = NULL;  // pointer to waveform data memory

void WriteWaveData(void)
{
	HWAVEOUT    hWaveOut;
	HGLOBAL     hWaveHdr;
	LPWAVEHDR   lpWaveHdr;
	HMMIO       hmmio;
	UINT        wResult;
	HANDLE      hFormat;
	WAVEFORMAT  *pFormat;
	DWORD       dwDataSize;

	// Open a waveform device for output using window callback.

	if (waveOutOpen((LPHWAVEOUT)&hWaveOut, WAVE_MAPPER,
					(LPWAVEFORMAT)pFormat,
					(LONG)hwndApp, 0L, CALLBACK_WINDOW))
	{
		MessageBox(hwndApp,
				   "Failed to open waveform output device.",
				   NULL, MB_OK | MB_ICONEXCLAMATION);
		LocalUnlock(hFormat);
		LocalFree(hFormat);
		mmioClose(hmmio, 0);
		return;
	}

	// Allocate and lock memory for the waveform data.

	hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, dwDataSize );
	if (!hData)
	{
		MessageBox(hwndApp, "Out of memory.",
				   NULL, MB_OK | MB_ICONEXCLAMATION);
		mmioClose(hmmio, 0);
		return;
	}
	if ((lpData = GlobalLock(hData)) == NULL)
	{
		MessageBox(hwndApp, "Failed to lock memory for data chunk.",
				   NULL, MB_OK | MB_ICONEXCLAMATION);
		GlobalFree(hData);
		mmioClose(hmmio, 0);
		return;
	}

	// Read the waveform data subchunk.

	if(mmioRead(hmmio, (HPSTR) lpData, dwDataSize) != (LRESULT)dwDataSize)
	{
		MessageBox(hwndApp, "Failed to read data chunk.",
				   NULL, MB_OK | MB_ICONEXCLAMATION);
		GlobalUnlock(hData);
		GlobalFree(hData);
		mmioClose(hmmio, 0);
		return;
	}

	// Allocate and lock memory for the header.

	hWaveHdr = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
		(DWORD) sizeof(WAVEHDR));
	if (hWaveHdr == NULL)
	{
		GlobalUnlock(hData);
		GlobalFree(hData);
		MessageBox(hwndApp, "Not enough memory for header.",
			NULL, MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	lpWaveHdr = (LPWAVEHDR) GlobalLock(hWaveHdr);
	if (lpWaveHdr == NULL)
	{
		GlobalUnlock(hData);
		GlobalFree(hData);
		MessageBox(hwndApp,
			"Failed to lock memory for header.",
			NULL, MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	// After allocation, set up and prepare header.

	lpWaveHdr->lpData = lpData;
	lpWaveHdr->dwBufferLength = dwDataSize;
	lpWaveHdr->dwFlags = 0L;
	lpWaveHdr->dwLoops = 0L;
	waveOutPrepareHeader(hWaveOut, lpWaveHdr, sizeof(WAVEHDR));

	// Now the data block can be sent to the output device. The
	// waveOutWrite function returns immediately and waveform
	// data is sent to the output device in the background.

	wResult = waveOutWrite(hWaveOut, lpWaveHdr, sizeof(WAVEHDR));
	if (wResult != 0)
	{
		waveOutUnprepareHeader(hWaveOut, lpWaveHdr,
							   sizeof(WAVEHDR));
		GlobalUnlock( hData);
		GlobalFree(hData);
		MessageBox(hwndApp, "Failed to write block to device",
				   NULL, MB_OK | MB_ICONEXCLAMATION);
		return;
	}
}*/
// waste /////////////////////////////////////////////////////////////////////
/*
//****************************************************************************
//*                     notes
//*
//* RecordCapturedData() available at
//* https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee419050(v=vs.85)?redirectedfrom=MSDN
//* capturing waveforms might be a better example, available at
//* https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee416968(v=vs.85)
//* something about strereo mix
//* https://www.howtogeek.com/39532/how-to-enable-stereo-mix-in-windows-7-to-record-audio/
//****************************************************************************

//****************************************************************************
//*                     define
//****************************************************************************
#define RECORD_BUFFER_SIZE		32768L
#define DATABLOCK_SIZE			32768L
#define MAX_BUFFERS				2

//****************************************************************************
//*                     global
//****************************************************************************
WCHAR wszBuffer[BUFFER_MAX] = { '\0' };
extern HWND g_hDlg;

WAVEFORMATEX g_wfx{};
// audio capture
HWAVEIN g_hwi{};
LPWAVEHDR g_whin[MAX_BUFFERS];
VOID* g_pRecordBuffer;
UINT32 g_cBufferIn = 0;

// audio playback
HWAVEOUT g_hwo{};
LPWAVEHDR g_whout[MAX_BUFFERS];
VOID* g_pPlaybackBuffer[MAX_BUFFERS];
UINT32 g_cBufferOut = 0;

//****************************************************************************
//*                     waveInProc
//****************************************************************************
void CALLBACK waveInProc(HWAVEIN hwi
	, UINT uMsg
	, DWORD_PTR dwInstance
	, DWORD_PTR dwParam1
	, DWORD_PTR dwParam2
)
{
	OutputDebugString(L"waveInProc()\n");
	switch (uMsg)
	{
	case WIM_OPEN:
	{
		OutputDebugString(L"WIM_OPEN\n");
		break;
	} // eof WIM_OPEN
	case WIM_DATA:
	{
		OutputDebugString(L"WIM_DATA\n");
		PostMessage(g_hDlg
			, WM_COMMAND
			, (WPARAM)WIM_DATA
			, (LPARAM)dwParam1
		);

		break;
	} // eof WIM_DATA
	case WIM_CLOSE:
	{
		OutputDebugString(L"WIM_CLOSE\n");
		PostMessage(g_hDlg
			, WM_COMMAND
			, (WPARAM)WIM_CLOSE
			, (LPARAM)0
		);
		break;
	} // eof WIM_CLOSE
	} // eof swich
}

//****************************************************************************
//*                     waveOutProc
//****************************************************************************
void CALLBACK waveOutProc(HWAVEOUT hwo
	, UINT uMsg
	, DWORD_PTR dwInstance
	, DWORD_PTR dwParam1
	, DWORD_PTR dwParam2
)
{
	//OutputDebugString(L"waveOutProc()\n");
	switch (uMsg)
	{
	case WOM_OPEN:
	{
		OutputDebugString(L"WOM_OPEN\n");
		break;
	} // eof WOM_OPEN
	case WOM_DONE:
	{
		//OutputDebugString(L"WOM_DONE\n");
		PostMessage(g_hDlg
			, WM_COMMAND
			, (WPARAM)WOM_DONE
			, (LPARAM)dwParam1
		);

		break;
	} // eof WOM_DONE
	case WOM_CLOSE:
	{
		OutputDebugString(L"WOM_CLOSE\n");
		PostMessage(g_hDlg
			, WM_COMMAND
			, (WPARAM)WOM_CLOSE
			, (LPARAM)0
		);
		break;
	} // eof WOM_CLOSE
	} // eof swich
}

//*****************************************************************************
//*                     start_audio_capture
//*****************************************************************************
BOOL start_audio_capture()
{
	MMRESULT rc = MMSYSERR_NOERROR;

	for (int i = 0; i < MAX_BUFFERS; i++)
	{
		// allocate buffers
		g_whin[i] = new WAVEHDR;
		if (g_whin[i])
		{
			g_whin[i]->lpData = new char[DATABLOCK_SIZE];
			g_whin[i]->dwBufferLength = DATABLOCK_SIZE;
			g_whin[i]->dwFlags = 0;
		}
		// allocate record buffer with enough space
		// to hold ten data buffer blocks of waveform sound data
		g_pRecordBuffer = new BYTE[RECORD_BUFFER_SIZE];

		// prepare buffer and add to input queue
		rc = waveInPrepareHeader(g_hwi
			, g_whin[i]
			, sizeof(WAVEHDR)
		);
		// rc = 11: MMSYSERR_INVALPARAM
		if (rc == MMSYSERR_NOERROR)
		{
			rc = waveInAddBuffer(g_hwi
				, g_whin[i], sizeof(WAVEHDR)
			);
			OutputDebugString(L"buffer added\n");
		}
	}

	// start audio capture
	rc = waveInStart(g_hwi);
	if (rc == MMSYSERR_NOERROR)
	{
		OutputDebugString(L"waveInStart()\n");
	}

	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     start_playback
//*****************************************************************************
BOOL start_playback()
{
	MMRESULT rc = MMSYSERR_NOERROR;
	HRESULT hr = S_OK;
	DWORD dwSizeRead = 0;

	for (int i = 0; i < MAX_BUFFERS; i++)
	{
		g_pPlaybackBuffer[i] = new BYTE[DATABLOCK_SIZE];
		hr = readWaveFile((BYTE*)g_pPlaybackBuffer[i]
			, DATABLOCK_SIZE
			, &dwSizeRead
		);

		g_ck.dwDataOffset += dwSizeRead;

		g_whout[i] = new WAVEHDR;
		g_whout[i]->lpData = (LPSTR)g_pPlaybackBuffer[i];
		g_whout[i]->dwBufferLength = DATABLOCK_SIZE;
		g_whout[i]->dwFlags = 0;
		g_whout[i]->dwLoops = 0;
	}

	waveOutPrepareHeader(g_hwo, g_whout[0], sizeof(WAVEHDR));
	// start audio playback
	rc = waveOutWrite(g_hwo, g_whout[0], sizeof(WAVEHDR));
	if (rc == MMSYSERR_NOERROR)
	{
		//OutputDebugString(L"waveOutWrite()\n");
	}

	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     onWmInitDialog_DlgProc
//*****************************************************************************
BOOL onWmInitDialog_DlgProc(const HINSTANCE& hInst
	, const HWND& hDlg
)
{
	MMRESULT rc = MMSYSERR_NOERROR;
	UINT nMaxDevices = 0;
	UINT nDevId = 0;
	WAVEINCAPS wic{};


	// input device
	// open audio device
	nMaxDevices = waveInGetNumDevs();
	swprintf_s(wszBuffer
		, (size_t)BUFFER_MAX
		, L"input nof devices: %d\n"
		, nMaxDevices
	);
	OutputDebugString(wszBuffer);

	std::wstring wstrFormats = L"";
	std::wstring wstrTemplate =
		std::wstring(L"manufacturer id....: %d\n") +
		L"product id.........: %d\n"
		L"driver version.....: %d\n"
		L"product name.......: %s\n"
		L"standard formats...: %s\n";

	for (nDevId = 0; nDevId < nMaxDevices; nDevId++)
	{
		rc = waveInGetDevCaps(nDevId, &wic, sizeof(wic));
		if (rc == MMSYSERR_NOERROR)
		{
			if (wic.dwFormats & WAVE_FORMAT_4S08)
			{
				// 44.1 kHz, mono, 8-bit
				wstrFormats = L"WAVE_FORMAT_4S08";
			}
			if (wic.dwFormats & WAVE_FORMAT_4S16)
			{
				// 44.1 kHz, mono, 16-bit
				wstrFormats += L" | WAVE_FORMAT_4S16";
			}

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, wstrTemplate.c_str()
				, wic.wMid
				, wic.wPid
				, (INT16)wic.vDriverVersion
				, wic.szPname
				, wstrFormats.c_str()
			);

			OutputDebugString(wszBuffer);

			// depends on sound settings
			// either one of the two is active
			// 1) use microphone array, nDevId = 0
			//if (nDevId == 0)
			// 2) use stereo mix, nDevId == 1
			if (nDevId == 1)
			{
				g_wfx.nChannels = 2;
				g_wfx.nSamplesPerSec = 48'000;// 44'100;
				// does not work WAVE_FORMAT_IEEE_FLOAT
				g_wfx.wFormatTag = WAVE_FORMAT_PCM;
				g_wfx.wBitsPerSample = 16;
				g_wfx.nBlockAlign = g_wfx.nChannels * g_wfx.wBitsPerSample / 8;
				g_wfx.nAvgBytesPerSec = g_wfx.nSamplesPerSec * g_wfx.nBlockAlign;
				g_wfx.cbSize = 0;

				// open input device
				rc = waveInOpen(&g_hwi
					, nDevId
					, &g_wfx
					, (DWORD)(VOID*)waveInProc
					, (DWORD)0
					, CALLBACK_FUNCTION
				);
				if (rc == MMSYSERR_NOERROR)
				{
					OutputDebugString(L"mikes opened\n");
				}
			}
		}
	}

	UINT nMaxDevicesOut = 0;
	UINT nDevIdOut = 0;
	WAVEOUTCAPS woc{};
	// output device
	// open audio device
	nMaxDevicesOut = waveOutGetNumDevs();
	swprintf_s(wszBuffer
		, (size_t)BUFFER_MAX
		, L"output nof devices: %d\n"
		, nMaxDevicesOut
	);
	OutputDebugString(wszBuffer);

	for (nDevIdOut = 0; nDevIdOut < nMaxDevicesOut; nDevIdOut++)
	{
		rc = waveOutGetDevCaps(nDevIdOut, &woc, sizeof(woc));
		if (rc == MMSYSERR_NOERROR)
		{
			if (wic.dwFormats & WAVE_FORMAT_4S08)
			{
				// 44.1 kHz, mono, 8-bit
				wstrFormats = L"WAVE_FORMAT_4S08";
			}
			if (wic.dwFormats & WAVE_FORMAT_4S16)
			{
				// 44.1 kHz, mono, 16-bit
				wstrFormats += L" | WAVE_FORMAT_4S16";
			}

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, wstrTemplate.c_str()
				, woc.wMid
				, woc.wPid
				, (INT16)woc.vDriverVersion
				, woc.szPname
				, wstrFormats.c_str()
			);

			OutputDebugString(wszBuffer);

			// use Speaker/Headphone, nDevId = 0
			if (nDevIdOut == 0)
			{
				g_wfx.nChannels = 2;
				g_wfx.nSamplesPerSec = 48'000;// 44'100;
				// does not work WAVE_FORMAT_IEEE_FLOAT
				g_wfx.wFormatTag = WAVE_FORMAT_PCM;
				g_wfx.wBitsPerSample = 16;
				g_wfx.nBlockAlign = g_wfx.nChannels * g_wfx.wBitsPerSample / 8;
				g_wfx.nAvgBytesPerSec = g_wfx.nSamplesPerSec * g_wfx.nBlockAlign;
				g_wfx.cbSize = 0;

				// open output device
				rc = waveOutOpen(&g_hwo
					, nDevIdOut
					, &g_wfx
					, (DWORD)(VOID*)waveOutProc
					, (DWORD)0
					, CALLBACK_FUNCTION
				);
				if (rc == MMSYSERR_NOERROR)
				{
					OutputDebugString(L"speakers are ready to play\n");
				}
			}
		}
	}
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
	, const LPARAM& lParam
)
{
	DWORD dwSizeRead;
	HRESULT hr = S_OK;
	switch (LOWORD(wParam))
	{
	case WIM_DATA:
	{
		OutputDebugString(L"WIM_DATA\n");
		UINT nSizeWrote = 0;
		hr = writeWaveFile(g_whin[g_cBufferIn]->dwBufferLength
			, (BYTE*)g_whin[g_cBufferIn]->lpData
			, &nSizeWrote
		);
		// prepare buffer and add to input queue
		MMRESULT rc = waveInPrepareHeader(g_hwi
			, g_whin[g_cBufferIn]
			, sizeof(WAVEHDR)
		);
		rc = waveInAddBuffer(g_hwi
			, g_whin[g_cBufferIn], sizeof(WAVEHDR)
		);
		MMTIME mmtime;
		mmtime.wType = TIME_SAMPLES;
		rc = waveInGetPosition(g_hwi
			, &mmtime
			, sizeof(MMTIME)
		);
		swprintf_s(wszBuffer
			, (size_t)BUFFER_MAX
			, L"buffer: %d total sample: %d\n"
			, g_cBufferIn
			, mmtime.u.sample
		);
		g_cBufferIn = ++g_cBufferIn % MAX_BUFFERS;
		OutputDebugString(wszBuffer);
		return (INT_PTR)TRUE;
	} // eof WIM_DATA
	case WIM_CLOSE:
	{
		OutputDebugString(L"WIM_CLOSE\n");
		HRESULT hr = S_OK;
		//UINT nSizeWrote = 0;
		//for (int i = 0; i < MAX_BUFFERS; i++)
		//{
		//	hr = writeWaveFile(g_whin[i]->dwBufferLength
		//		, (BYTE*)g_whin[i]->lpData
		//		, &nSizeWrote
		//	);
		//	if (hr == S_OK)
		//	{
		//		OutputDebugString(L"writeWaveFile() OK\n");
		//	}
		//}
		// close .wav file
		hr = closeWaveFile();
		if (hr == S_OK)
		{
			OutputDebugString(L"closeWaveFile() OK\n");
		}

		return (INT_PTR)TRUE;
	} // eof WIM_CLOSE
	case WOM_OPEN:
	{
		return (INT_PTR)TRUE;
	} // eof WOM_OPEN
	case WOM_DONE:
	{
		if (g_ck.dwDataOffset >= getSizeWaveFile())
		{
			waveOutClose(g_hwo);
			return (INT_PTR)TRUE;
		}

		hr = readWaveFile((BYTE*)g_pPlaybackBuffer[g_cBufferOut]
			, DATABLOCK_SIZE
			, &dwSizeRead
		);
		// advance to next data block in wave file
		g_ck.dwDataOffset += dwSizeRead;
		g_whout[g_cBufferOut]->lpData = (LPSTR)g_pPlaybackBuffer[g_cBufferOut];

		g_cBufferOut = ++g_cBufferOut % MAX_BUFFERS;
		waveOutPrepareHeader(g_hwo, g_whout[g_cBufferOut], sizeof(WAVEHDR));
		waveOutWrite(g_hwo, g_whout[g_cBufferOut], sizeof(WAVEHDR));

		return (INT_PTR)TRUE;
	} // eof WOM_DONE
	case WOM_CLOSE:
	{
		return (INT_PTR)TRUE;
	} // eof WOM_CLOSE
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
			OutputDebugString(L"Start\n");
			// change text on button
			SendMessage(hWnd
				, WM_SETTEXT
				, (WPARAM)0
				, (LPARAM)L"Stop"
			);
			// create .wav file
			hr = openWaveFile((LPWSTR)L"wav_file.wav"
				, &g_wfx
				, WAVEFILE_WRITE
			);
			if (hr == S_OK)
			{
				OutputDebugString(L"openWaveFile() OK\n");
			}

			start_audio_capture();
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

			MMRESULT rc = waveInStop(g_hwi);
			if (rc == MMSYSERR_NOERROR)
			{
				OutputDebugString(L"waveInStop()\n");
				// does not work
				//rc = waveInClose(g_hwi);
				hr = closeWaveFile();
				if (hr == S_OK)
				{
					OutputDebugString(L"closeWaveFile() OK\n");
				}
			}
		}

		return (INT_PTR)TRUE;
	}
	case IDC_PLAYBACK:
	{
		// open wave file
		hr = openWaveFile((LPWSTR)L"wav_file.wav"
			, &g_wfx
			, WAVEFILE_READ
		);
		if (hr == S_OK)
		{
			OutputDebugString(L"openWaveFile() OK\n");
		}

		start_playback();

		return (INT_PTR)TRUE;
	} // eof IDC_PLAYBACK
	} // eof switch
	return (INT_PTR)FALSE;
}
*/
// waste /////////////////////////////////////////////////////////////////////
/*
// Global variables.

HANDLE hData  = NULL;  // handle of waveform data memory
HPSTR  lpData = NULL;  // pointer to waveform data memory

void WriteWaveData(void)
{
	HWAVEOUT    hWaveOut;
	HGLOBAL     hWaveHdr;
	LPWAVEHDR   lpWaveHdr;
	HMMIO       hmmio;
	UINT        wResult;
	HANDLE      hFormat;
	WAVEFORMAT  *pFormat;
	DWORD       dwDataSize;

	// Open a waveform device for output using window callback.

	if (waveOutOpen((LPHWAVEOUT)&hWaveOut, WAVE_MAPPER,
					(LPWAVEFORMAT)pFormat,
					(LONG)hwndApp, 0L, CALLBACK_WINDOW))
	{
		MessageBox(hwndApp,
				   "Failed to open waveform output device.",
				   NULL, MB_OK | MB_ICONEXCLAMATION);
		LocalUnlock(hFormat);
		LocalFree(hFormat);
		mmioClose(hmmio, 0);
		return;
	}

	// Allocate and lock memory for the waveform data.

	hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, dwDataSize );
	if (!hData)
	{
		MessageBox(hwndApp, "Out of memory.",
				   NULL, MB_OK | MB_ICONEXCLAMATION);
		mmioClose(hmmio, 0);
		return;
	}
	if ((lpData = GlobalLock(hData)) == NULL)
	{
		MessageBox(hwndApp, "Failed to lock memory for data chunk.",
				   NULL, MB_OK | MB_ICONEXCLAMATION);
		GlobalFree(hData);
		mmioClose(hmmio, 0);
		return;
	}

	// Read the waveform data subchunk.

	if(mmioRead(hmmio, (HPSTR) lpData, dwDataSize) != (LRESULT)dwDataSize)
	{
		MessageBox(hwndApp, "Failed to read data chunk.",
				   NULL, MB_OK | MB_ICONEXCLAMATION);
		GlobalUnlock(hData);
		GlobalFree(hData);
		mmioClose(hmmio, 0);
		return;
	}

	// Allocate and lock memory for the header.

	hWaveHdr = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
		(DWORD) sizeof(WAVEHDR));
	if (hWaveHdr == NULL)
	{
		GlobalUnlock(hData);
		GlobalFree(hData);
		MessageBox(hwndApp, "Not enough memory for header.",
			NULL, MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	lpWaveHdr = (LPWAVEHDR) GlobalLock(hWaveHdr);
	if (lpWaveHdr == NULL)
	{
		GlobalUnlock(hData);
		GlobalFree(hData);
		MessageBox(hwndApp,
			"Failed to lock memory for header.",
			NULL, MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	// After allocation, set up and prepare header.

	lpWaveHdr->lpData = lpData;
	lpWaveHdr->dwBufferLength = dwDataSize;
	lpWaveHdr->dwFlags = 0L;
	lpWaveHdr->dwLoops = 0L;
	waveOutPrepareHeader(hWaveOut, lpWaveHdr, sizeof(WAVEHDR));

	// Now the data block can be sent to the output device. The
	// waveOutWrite function returns immediately and waveform
	// data is sent to the output device in the background.

	wResult = waveOutWrite(hWaveOut, lpWaveHdr, sizeof(WAVEHDR));
	if (wResult != 0)
	{
		waveOutUnprepareHeader(hWaveOut, lpWaveHdr,
							   sizeof(WAVEHDR));
		GlobalUnlock( hData);
		GlobalFree(hData);
		MessageBox(hwndApp, "Failed to write block to device",
				   NULL, MB_OK | MB_ICONEXCLAMATION);
		return;
	}
}
*/
