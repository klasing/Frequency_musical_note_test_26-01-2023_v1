#pragma once
//****************************************************************************
//*                     extern
//****************************************************************************
extern HWND g_hDlg;
//****************************************************************************
//*                     define
//****************************************************************************
#define RECORD_BUFFER_SIZE		327680L
#define DATABLOCK_SIZE			32768L
#define MAX_BUFFERS				256	// 47 s 
//****************************************************************************
//*                     global
//****************************************************************************
WCHAR wszBuffer[BUFFER_MAX] = { '\0' };
MMRESULT rc = MMSYSERR_NOERROR;
HRESULT hr = S_OK;
HWAVEIN g_hwi{};
WAVEFORMATEX g_wfx{};
LPWAVEHDR g_whin[MAX_BUFFERS]{};
VOID* g_pRecordBuffer;
//****************************************************************************
//*                     waveInProc
//****************************************************************************
void CALLBACK waveInProc(HWAVEIN hwin
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
		PostMessage(g_hDlg
			, WM_COMMAND
			, (WPARAM)WIM_OPEN
			, (LPARAM)0
		);
		break;
	} // eof WIM_OPEN
	case WIM_DATA:
	{
		PostMessage(g_hDlg
			, WM_COMMAND
			, (WPARAM)WIM_DATA
			, (LPARAM)0
		);
		break;
	} // eof WIM_DATA
	case WIM_CLOSE:
	{
		PostMessage(g_hDlg
			, WM_COMMAND
			, (WPARAM)WIM_CLOSE
			, (LPARAM)0
		);
		break;
	} // eof WIM_CLOSE
	} // eof switch
}
//*****************************************************************************
//*                     start_audio_capture
//*****************************************************************************
BOOL start_audio_capture()
{
	OutputDebugString(L"start_audio_capture()\n");
	// create .wav file
	hr = openWaveFile((LPWSTR)L"wav_file.wav"
		, &g_wfx
		, WAVEFILE_WRITE
	);
	if (hr == S_OK)
	{
		OutputDebugString(L"openWaveFile() OK\n");
	}
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
//*                     stop_audio_capture
//*****************************************************************************
BOOL stop_audio_capture()
{
	OutputDebugString(L"stop_audio_capture()\n");
	rc = waveInStop(g_hwi);
	if (rc == MMSYSERR_NOERROR)
	{
		OutputDebugString(L"waveInStop()\n");
		rc = waveInClose(g_hwi);
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
	UINT nMaxDevices = 0;
	UINT nDevId = 0;
	WAVEINCAPS wic{};

	// open audio device
	nMaxDevices = waveInGetNumDevs();
	swprintf_s(wszBuffer
		, (size_t)BUFFER_MAX
		, L"nof devices: %d\n"
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

			g_wfx.nChannels = 2;
			g_wfx.nSamplesPerSec = 44'100;
			g_wfx.wFormatTag = WAVE_FORMAT_PCM;
			g_wfx.wBitsPerSample = 16;
			g_wfx.nBlockAlign = g_wfx.nChannels * g_wfx.wBitsPerSample / 8;
			g_wfx.nAvgBytesPerSec = g_wfx.nSamplesPerSec * g_wfx.nBlockAlign;
			g_wfx.cbSize = 0;

			// use microphone array, nDevId = 0
			if (nDevId == 0)
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
					OutputDebugString(L"mikes opened\n");
				}
			}
		}
	}
	return EXIT_SUCCESS;
}
//*****************************************************************************
//*                     onWmSize_DlgProc
//*****************************************************************************
BOOL onWmSize_DlgProc(const HWND& hDlg)
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
	switch (LOWORD(wParam))
	{
	case WIM_OPEN:
	{
		OutputDebugString(L"WIM_OPEN\n");
		// start audio capture
		rc = waveInStart(g_hwi);
		return (INT_PTR)TRUE;
	} // eof WIM_OPEN
	case WIM_DATA:
	{
		OutputDebugString(L"WIM_DATA\n");
		return (INT_PTR)TRUE;
	} // eof WIM_DATA
	case WIM_CLOSE:
	{
		OutputDebugString(L"WIM_CLOSE\n");
		UINT nSizeWrote = 0;
		for (int i = 0; i < MAX_BUFFERS; i++)
		{
			hr = writeWaveFile(g_whin[i]->dwBufferLength
				, (BYTE*)g_whin[i]->lpData
				, &nSizeWrote
			);
			if (hr == S_OK)
			{
				OutputDebugString(L"writeWaveFile() OK\n");
			}
		}
		// close .wav file
		hr = closeWaveFile();
		if (hr == S_OK)
		{
			OutputDebugString(L"closeWaveFile() OK\n");
		}
		return (INT_PTR)TRUE;
	} // eof WIM_CLOSE
	case IDC_START_AUDIO_CAPTURE:
	{
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
			stop_audio_capture();
		}
		return (INT_PTR)TRUE;
	} // eof IDC_START_AUDIO_CAPTURE
	} // eof switch
	return (INT_PTR)FALSE;
}

// waste /////////////////////////////////////////////////////////////////////
/*
// capturing waveforms might be a better example
// available at
// https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee416968(v=vs.85)
// something about strereo mix
// https://www.howtogeek.com/39532/how-to-enable-stereo-mix-in-windows-7-to-record-audio/
//****************************************************************************
//*                     define
//****************************************************************************
#define RECORD_BUFFER_SIZE		327680L
#define DATABLOCK_SIZE			32768L
#define MAX_BUFFERS				256	// 47 s 
//#define MAX_BUFFERS				128	// 23 s 
//#define MAX_BUFFERS				64	// 11 s 

//****************************************************************************
//*                     global
//****************************************************************************
WCHAR wszBuffer[BUFFER_MAX] = { '\0' };

HWAVEIN g_hwi{};
LPWAVEHDR g_whin[MAX_BUFFERS];

VOID* g_pRecordBuffer;

WAVEFORMATEX g_wfx{};

extern HWND g_hDlg;

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
	//MMRESULT rc = MMSYSERR_NOERROR;
	//HRESULT hr = S_OK;
	//UINT nSizeWrote = 0;
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
		//LPWAVEHDR lpWHDR = (LPWAVEHDR)dwParam1;
		//hr = writeWaveFile(lpWHDR->dwBufferLength
		//	, (BYTE*)lpWHDR->lpData
		//	, &nSizeWrote
		//);
		//if (hr == S_OK)
		//{
		//	OutputDebugString(L"writeWaveFile() OK\n");
		//}

		//// prepare buffer
		//rc = waveInPrepareHeader(g_hwi
		//	, lpWHDR
		//	, sizeof(WAVEHDR)
		//);
		//if (rc == MMSYSERR_NOERROR)
		//{
		//	OutputDebugString(L"waveInPrepareHeader() OK\n");
		//}
		//// add to input queue
		//rc = waveInAddBuffer(hwi
		//	, lpWHDR
		//	, sizeof(WAVEHDR)
		//);
		//if (rc == MMSYSERR_NOERROR)
		//{
		//	OutputDebugString(L"waveInAddBuffer() OK\n");
		//}
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

	// open audio device
	nMaxDevices = waveInGetNumDevs();
	swprintf_s(wszBuffer
		, (size_t)BUFFER_MAX
		, L"nof devices: %d\n"
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

			g_wfx.nChannels = 2;
			g_wfx.nSamplesPerSec = 44'100;
			g_wfx.wFormatTag = WAVE_FORMAT_PCM;
			g_wfx.wBitsPerSample = 16;
			g_wfx.nBlockAlign = g_wfx.nChannels * g_wfx.wBitsPerSample / 8;
			g_wfx.nAvgBytesPerSec = g_wfx.nSamplesPerSec * g_wfx.nBlockAlign;
			g_wfx.cbSize = 0;

			// use microphone array, nDevId = 0
			if (nDevId == 0)
			// use stereo mix, when microphone is off, nDevId == 0
			//if (nDevId == 0)
			// use stereo mix, when microphone is on, nDevId == 1
			//if (nDevId == 1)
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
					OutputDebugString(L"mikes opened\n");
				}
			}
		}
	}

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
	HRESULT hr = S_OK;
	switch (LOWORD(wParam))
	{
	case WIM_DATA:
	{
		OutputDebugString(L"WIM_DATA\n");
		//MMRESULT rc = MMSYSERR_NOERROR;
		//HRESULT hr = S_OK;
		//UINT nSizeWrote = 0;
		//LPWAVEHDR lpWHDR = (LPWAVEHDR)lParam;
		//hr = writeWaveFile(lpWHDR->dwBufferLength
		//	, (BYTE*)lpWHDR->lpData
		//	, &nSizeWrote
		//);
		//if (hr == S_OK)
		//{
		//	OutputDebugString(L"writeWaveFile() OK\n");
		//}

		//// prepare buffer
		//rc = waveInPrepareHeader(g_hwi
		//	, lpWHDR
		//	, sizeof(WAVEHDR)
		//);
		//if (rc == MMSYSERR_NOERROR)
		//{
		//	OutputDebugString(L"waveInPrepareHeader() OK\n");
		//}
		//// add to input queue
		//rc = waveInAddBuffer(g_hwi
		//	, lpWHDR
		//	, sizeof(WAVEHDR)
		//);
		//if (rc == MMSYSERR_NOERROR)
		//{
		//	OutputDebugString(L"waveInAddBuffer() OK\n");
		//}

		return (INT_PTR)TRUE;
	} // eof WIM_DATA
	case WIM_CLOSE:
	{
		OutputDebugString(L"WIM_CLOSE\n");
		HRESULT hr = S_OK;
		UINT nSizeWrote = 0;
		for (int i = 0; i < MAX_BUFFERS; i++)
		{
			hr = writeWaveFile(g_whin[i]->dwBufferLength
				, (BYTE*)g_whin[i]->lpData
				, &nSizeWrote
			);
			if (hr == S_OK)
			{
				OutputDebugString(L"writeWaveFile() OK\n");
			}
		}
		// close .wav file
		hr = closeWaveFile();
		if (hr == S_OK)
		{
			OutputDebugString(L"closeWaveFile() OK\n");
		}

		return (INT_PTR)TRUE;
	} // eof WIM_CLOSE
	case IDC_START_AUDIO_CAPTURE:
	{
		OutputDebugString(L"IDC_START_AUDIO_CAPTURE\n");

		// create .wav file
		hr = openWaveFile((LPWSTR)L"wav_file.wav"
			, &g_wfx
			, WAVEFILE_WRITE
		);
		if (hr == S_OK)
		{
			OutputDebugString(L"openWaveFile() OK\n");
		}

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
				rc = waveInClose(g_hwi);
			}
		}

		return (INT_PTR)TRUE;
	}
	} // eof switch
	return (INT_PTR)FALSE;
}
*/

/*
// available at
// https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee419050(v=vs.85)?redirectedfrom=MSDN
HRESULT RecordCapturedData()
	{
	  HRESULT hr;
	  VOID* pbCaptureData  = NULL;
	  DWORD dwCaptureLength;
	  VOID* pbCaptureData2 = NULL;
	  DWORD dwCaptureLength2;
	  VOID* pbPlayData   = NULL;
	  UINT  dwDataWrote;
	  DWORD dwReadPos;
	  LONG lLockSize;

	  if (NULL == g_pDSBCapture)
		  return S_FALSE;
	  if (NULL == g_pWaveFile)
		  return S_FALSE;

	  if (FAILED (hr = g_pDSBCapture->GetCurrentPosition(
		NULL, &dwReadPos)))
		  return hr;

	  // Lock everything between the private cursor
	  // and the read cursor, allowing for wraparound.

	  lLockSize = dwReadPos - g_dwNextCaptureOffset;
	  if( lLockSize < 0 ) lLockSize += g_dwCaptureBufferSize;

	  if( lLockSize == 0 ) return S_FALSE;

	  if (FAILED(hr = g_pDSBCapture->Lock(
			g_dwNextCaptureOffset, lLockSize,
			&pbCaptureData, &dwCaptureLength,
			&pbCaptureData2, &dwCaptureLength2, 0L)))
		return hr;

	  // Write the data. This is done in two steps
	  // to account for wraparound.

	  if (FAILED( hr = g_pWaveFile->Write( dwCaptureLength,
		  (BYTE*)pbCaptureData, &dwDataWrote)))
		return hr;

	  if (pbCaptureData2 != NULL)
	  {
		if (FAILED(hr = g_pWaveFile->Write(
			dwCaptureLength2, (BYTE*)pbCaptureData2,
			&dwDataWrote)))
		  return hr;
	  }

	  // Unlock the capture buffer.

	 g_pDSBCapture->Unlock( pbCaptureData, dwCaptureLength,
		pbCaptureData2, dwCaptureLength2  );

	  // Move the capture offset forward.

	  g_dwNextCaptureOffset += dwCaptureLength;
	  g_dwNextCaptureOffset %= g_dwCaptureBufferSize;
	  g_dwNextCaptureOffset += dwCaptureLength2;
	  g_dwNextCaptureOffset %= g_dwCaptureBufferSize;

	  return S_OK;
	}
*/

// waste ///////////////////////////////////////////////////////////////////////
////****************************************************************************
////*                     include
////****************************************************************************
//#include "framework.h"
//#include "Audio_Capture_01-03-2023_v1.h"
//
////****************************************************************************
////*                     define
////****************************************************************************
//#define RECORD_BUFFER_SIZE		327680L
//#define DATABLOCK_SIZE			32768L
//#define MAX_BUFFERS				2 
//
////****************************************************************************
////*                     global
////****************************************************************************
//WCHAR wszBuffer[BUFFER_MAX] = { '\0' };
//HWAVEIN hwi{};
//LPWAVEHDR whin[MAX_BUFFERS];
//
//VOID* pRecordBuffer;
//
////****************************************************************************
////*                     waveInProc
////****************************************************************************
//void CALLBACK waveInProc(HWAVEIN hwi
//	, UINT uMsg
//	, DWORD_PTR dwInstance
//	, DWORD_PTR dwParam1
//	, DWORD_PTR dwParam2
//)
//{
//	OutputDebugString(L"waveInProc()\n");
//	switch (uMsg)
//	{
//	case WIM_OPEN:
//	{
//		OutputDebugString(L"WIM_OPEN\n");
//		break;
//	} // eof WIM_OPEN
//	case WIM_DATA:
//	{
//		OutputDebugString(L"WIM_DATA\n");
//		// dwParam1 is a pointer to a WAVEHDR
//		if ((((WAVEHDR*)dwParam1)->dwFlags & WHDR_DONE) == WHDR_DONE)
//		{
//			OutputDebugString(L"WHDR_DONE\n");
//			//...
//			// a waveform-audio data block has been saved and 
//			// can now be reused
//			// prepare buffer and add to input queue
//			waveInPrepareHeader((HWAVEIN)hwi
//				, (LPWAVEHDR)dwParam1
//				, sizeof(WAVEHDR)
//			);
//			waveInAddBuffer(hwi
//				, (LPWAVEHDR)dwParam1
//				, sizeof(WAVEHDR)
//			);
//			((LPWAVEHDR)dwParam1)->dwFlags = 0;
//		}
//		break;
//	} // eof WIM_DATA
//	case WIM_CLOSE:
//	{
//		OutputDebugString(L"WIM_CLOSE\n");
//		break;
//	} // eof WIM_CLOSE
//	} // eof swich
//}
//
////****************************************************************************
////*                     open_input_device
////****************************************************************************
//BOOL open_input_device()
//{
//	MMRESULT rc = MMSYSERR_NOERROR;
//	WAVEINCAPS wic{};
//	WAVEFORMATEX wfx{};
//	UINT nDevId = 0;
//	UINT nMaxDevices = 0;
//
//	nMaxDevices = waveInGetNumDevs();
//	swprintf_s(wszBuffer
//		, (size_t)BUFFER_MAX
//		, L"nof devices: %d\n"
//		, nMaxDevices
//	);
//	OutputDebugString(wszBuffer);
//
//	std::wstring wstrFormats = L"";
//	std::wstring wstrTemplate =
//		std::wstring(L"manufacturer id....: %d\n") +
//		L"product id.........: %d\n"
//		L"driver version.....: %d\n"
//		L"product name.......: %s\n"
//		L"standard formats...: %s\n";
//
//	for (nDevId = 0; nDevId < nMaxDevices; nDevId++)
//	{
//		rc = waveInGetDevCaps(nDevId, &wic, sizeof(wic));
//		if (rc == MMSYSERR_NOERROR)
//		{
//			if (wic.dwFormats & WAVE_FORMAT_4S08)
//			{
//				// 44.1 kHz, mono, 8-bit
//				wstrFormats = L"WAVE_FORMAT_4S08";
//			}
//			if (wic.dwFormats & WAVE_FORMAT_4S16)
//			{
//				// 44.1 kHz, mono, 16-bit
//				wstrFormats += L" | WAVE_FORMAT_4S16";
//			}
//
//			swprintf_s(wszBuffer
//				, (size_t)BUFFER_MAX
//				, wstrTemplate.c_str()
//				, wic.wMid
//				, wic.wPid
//				, (INT16)wic.vDriverVersion
//				, wic.szPname
//				, wstrFormats.c_str()
//			);
//
//			OutputDebugString(wszBuffer);
//
//			wfx.nChannels = 2;
//			wfx.nSamplesPerSec = 44'100;
//			wfx.wFormatTag = WAVE_FORMAT_PCM;
//			wfx.wBitsPerSample = 16;
//			wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
//			wfx.cbSize = 0;
//
//			if (nDevId == 1)
//			{
//				// open input device
//				rc = waveInOpen(&hwi
//					, nDevId
//					, &wfx
//					, (DWORD)(VOID*)waveInProc
//					, (DWORD)0
//					, CALLBACK_FUNCTION
//				);
//				if (rc == MMSYSERR_NOERROR)
//				{
//					OutputDebugString(L"mikes opened\n");
//				}
//			}
//		}
//		else
//		{
//
//		}
//	}
//	
//	//OutputDebugString(L"bla\n");
//	return EXIT_SUCCESS;
//}
//
////*****************************************************************************
////*                     onWmInitDialog_DlgProc
////*****************************************************************************
//BOOL onWmInitDialog_DlgProc(const HINSTANCE& hInst
//	, const HWND& hDlg
//)
//{
//	MMRESULT rc = MMSYSERR_NOERROR;
//	MMTIME mmtime;
//
//	HRESULT hr = CoInitialize(nullptr);
//	open_input_device();
//	
//	for (int i = 0; i < MAX_BUFFERS; i++)
//	{
//		// allocate buffers
//		whin[i] = new WAVEHDR;
//		if (whin[i])
//		{
//			whin[i]->lpData = new char[DATABLOCK_SIZE];
//			whin[i]->dwBufferLength = DATABLOCK_SIZE;
//			whin[i]->dwFlags = 0;
//		}
//		// allocate record buffer with enough space
//		// to hold ten data buffer blocks of waveform sound data
//		pRecordBuffer = new BYTE[RECORD_BUFFER_SIZE];
//
//		// prepare buffer and add to input queue
//		rc = waveInPrepareHeader(hwi
//			, whin[i]
//			, sizeof(WAVEHDR)
//		);
//		// rc = 11: MMSYSERR_INVALPARAM
//		if (rc == MMSYSERR_NOERROR)
//		{
//			rc = waveInAddBuffer(hwi
//			, whin[i]
//			, sizeof(WAVEHDR)
//			);
//			OutputDebugString(L"buffer added\n");
//		}
//	}
//
//	// start recording
//	rc = waveInStart(hwi);
//	if (rc == MMSYSERR_NOERROR)
//	{
//		OutputDebugString(L"waveInStart() OK\n");
//	}
//	else
//	{
//		switch (rc)
//		{
//		case MMSYSERR_INVALHANDLE:
//		{
//			OutputDebugString(L"MMSYSERR_INVALHANDLE\n");
//			break;
//		} // eof MMSYSERR_INVALHANDLE
//		case MMSYSERR_NODRIVER:
//		{
//			OutputDebugString(L"MMSYSERR_NODRIVER\n");
//			break;
//		} // eof MMSYSERR_NODRIVER
//		case MMSYSERR_NOMEM:
//		{
//			OutputDebugString(L"MMSYSERR_NOMEM\n");
//			break;
//		} // eof MMSYSERR_NOMEM
//		default:
//			OutputDebugString(L"ERROR unknown\n");
//		} // eof switch
//	}
//	//while (TRUE)
//	//{
//	//	SleepEx(1000, FALSE);
//	//}
//
//	// test func waveInGetPosition() 
//	//mmtime.wType = TIME_SAMPLES;
//	//rc = waveInGetPosition(hwi, &mmtime, sizeof(MMTIME));
//	//if (rc != MMSYSERR_NOERROR)
//	//{
//	//	OutputDebugString(L"error waveInGetPosition()\n");
//	//}
//
//	return EXIT_SUCCESS;
//}

////****************************************************************************
////*                     include
////****************************************************************************
//#include "framework.h"
//#include "Audio_Capture_01-03-2023_v1.h"
////****************************************************************************
////*                     define
////****************************************************************************
//#define RECORD_BUFFER_SIZE		327680L
//#define DATABLOCK_SIZE			32768L
//#define MAX_BUFFERS				2
////****************************************************************************
////*                     global
////****************************************************************************
//HWAVEIN hwi;
//LPWAVEHDR whin[MAX_BUFFERS];
////CFile* AudioFile
//
//VOID* pRecordBuffer;
//DWORD nRecordBufferPos;
//DWORD nRecordBufferLen;
////****************************************************************************
////*                     prototype
////****************************************************************************
//BOOL TestOpenInputDevice(HWND hWnd);
