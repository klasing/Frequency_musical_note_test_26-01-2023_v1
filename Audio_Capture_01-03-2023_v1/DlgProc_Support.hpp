#pragma once

//****************************************************************************
//*                     define
//****************************************************************************
#define RECORD_BUFFER_SIZE		327680L
#define DATABLOCK_SIZE			32768L
#define MAX_BUFFERS				2 

//****************************************************************************
//*                     global
//****************************************************************************
WCHAR wszBuffer[BUFFER_MAX] = { '\0' };

HWAVEIN g_hwi{};
LPWAVEHDR g_whin[MAX_BUFFERS];

VOID* g_pRecordBuffer;

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
		waveInAddBuffer(hwi
			, (LPWAVEHDR)dwParam1
			, sizeof(WAVEHDR)
		);
		break;
	} // eof WIM_DATA
	case WIM_CLOSE:
	{
		OutputDebugString(L"WIM_CLOSE\n");
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
	WAVEFORMATEX wfx{};

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

			wfx.nChannels = 2;
			wfx.nSamplesPerSec = 44'100;
			wfx.wFormatTag = WAVE_FORMAT_PCM;
			wfx.wBitsPerSample = 16;
			wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
			wfx.cbSize = 0;

			if (nDevId == 1)
			{
				// open input device
				rc = waveInOpen(&g_hwi
					, nDevId
					, &wfx
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
			}
		}

		return (INT_PTR)TRUE;
	}
	} // eof switch
	return (INT_PTR)FALSE;
}

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
