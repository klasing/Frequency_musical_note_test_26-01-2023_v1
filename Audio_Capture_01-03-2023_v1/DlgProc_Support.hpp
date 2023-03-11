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
HWAVEIN g_hwi{};
LPWAVEHDR g_whi[MAX_BUFFERS]{};
UINT32 g_cBufferIn = 0;
// audio playback
HWAVEOUT g_hwo{};
LPWAVEHDR g_who[PLAY_MAX_BUFFERS]{};
VOID* g_pPlaybackBuffer[PLAY_MAX_BUFFERS]{};
DWORD g_dwSizeWaveFile = 0;
DWORD g_dwSizeRead = 0;
DWORD g_nBlock = 0;
UINT g_cBufferOut = 0;
UINT g_cFreeBuffer = 0;
HANDLE hFeederPlayback = NULL;
DWORD g_dwFeederPlaybackId = 0;

//*****************************************************************************
//*                     feeder_playback
//*****************************************************************************
DWORD WINAPI feeder_playback(LPVOID lpVoid)
{
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		switch (msg.message)
		{
		case MM_WOM_OPEN:
		{
			OutputDebugString(L"feeder_playback MM_WOM_OPEN\n");
			break;
		} // eof MM_WOM_OPEN
		case MM_WOM_DONE:
		{
			OutputDebugString(L"feeder_playback MM_WOM_DONE\n");
			if (g_cBufferOut == g_nBlock) waveOutClose(g_hwo);
			break;
		} // eof MM_WOM_DONE
		case MM_WOM_CLOSE:
		{
			OutputDebugString(L"feeder_playback MM_WOM_CLOSE\n");
			for (int i = 0; i < g_nBlock; i++)
			{
				g_who[i] = NULL;
			}
			g_hwo = NULL;
			g_cBufferOut = 0;
			closeWaveFile();
			break;
		} // eof MM_WOM_CLOSE
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
	// start thread feeder_playback
	hFeederPlayback = CreateThread(NULL
		, 0
		, feeder_playback
		, (LPVOID)nullptr
		, 0 // run immediately
		, &g_dwFeederPlaybackId
	);

	rc = waveOutOpen(&g_hwo
		, SPEAKER_HEADPHONE
		, &g_wfx
		, (DWORD)g_dwFeederPlaybackId
		, (DWORD)0
		, CALLBACK_THREAD
	);
	// open .wav file
	openWaveFile((const LPWSTR)L"wav_file.wav"
		, &g_wfx
		, WAVEFILE_READ
	);

	g_dwSizeWaveFile = getSizeWaveFile();
	g_nBlock = ((g_dwSizeWaveFile / DATABLOCK_SIZE) > PLAY_MAX_BUFFERS) ?
		PLAY_MAX_BUFFERS :
		g_dwSizeWaveFile / DATABLOCK_SIZE;

	for (int i = 0; i < g_nBlock; i++)
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
	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     onWmInitDialog_DlgProc
//*****************************************************************************
BOOL onWmInitDialog_DlgProc(const HINSTANCE& hInst
	, const HWND& hDlg
)
{
	BYTE* ringBuffer = nullptr;
	VOID* secondaryView = nullptr;
	ringBuffer = (BYTE*)createRingBuffer(0x10000, &secondaryView);

	// test the scenario
	// 1) the ringbuffer was fully filled
	// 2) the producer has filled the first chunk a second time
	// 3) the consumer wraps around, and reads the first chunk
	// - the consumer now runs after the producer, so
	//   the producer is the head and the consumer is the tail
	// - when the producer wraps around, it will no longer be the head
	BOOL bProducerIsHead = TRUE;
	UINT32 iProducer = 0;
	UINT32 iConsumer = 0;
	ringBuffer[iProducer++] = 'a';
	BYTE b = ringBuffer[iConsumer];

	UnmapViewOfFile(ringBuffer);
	UnmapViewOfFile(secondaryView);
	ringBuffer = nullptr;
	secondaryView = nullptr;

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
