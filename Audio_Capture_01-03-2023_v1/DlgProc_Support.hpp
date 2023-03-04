#pragma once

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
#define RECORD_BUFFER_SIZE		327680L
#define DATABLOCK_SIZE			32768L
#define MAX_BUFFERS				2
//#define MAX_BUFFERS				256	// 47 s 
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

UINT32 g_cBuffer = 0;

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
			g_wfx.nSamplesPerSec = 48'000;// 44'100;
			// does not work WAVE_FORMAT_IEEE_FLOAT
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
	HRESULT hr = S_OK;
	switch (LOWORD(wParam))
	{
	case WIM_DATA:
	{
		OutputDebugString(L"WIM_DATA\n");
		UINT nSizeWrote = 0;
		hr = writeWaveFile(g_whin[g_cBuffer]->dwBufferLength
			, (BYTE*)g_whin[g_cBuffer]->lpData
			, &nSizeWrote
		);
		// prepare buffer and add to input queue
		MMRESULT rc = waveInPrepareHeader(g_hwi
			, g_whin[g_cBuffer]
			, sizeof(WAVEHDR)
		);
		rc = waveInAddBuffer(g_hwi
			, g_whin[g_cBuffer], sizeof(WAVEHDR)
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
			, g_cBuffer
			, mmtime.u.sample
		);
		g_cBuffer = ++g_cBuffer % MAX_BUFFERS;
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
	} // eof switch
	return (INT_PTR)FALSE;
}

