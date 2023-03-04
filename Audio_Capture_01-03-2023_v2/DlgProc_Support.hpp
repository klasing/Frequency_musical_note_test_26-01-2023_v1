#pragma once
////****************************************************************************
////*                     include
////****************************************************************************
#include "framework.h"
#include "Audio_Capture_01-03-2023_v2.h"

//****************************************************************************
//*                     extern
//****************************************************************************

//****************************************************************************
//*                     define
//****************************************************************************
#define cEvents	3
//****************************************************************************
//*                     global
//****************************************************************************
WCHAR wszBuffer[BUFFER_MAX] = { '\0' };
HRESULT hr = S_OK;

//*****************************************************************************
//*                     onWmInitDialog_DlgProc
//*****************************************************************************
BOOL onWmInitDialog_DlgProc(const HINSTANCE& hInst
	, const HWND& hDlg
)
{
	// taken from
	// https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee416968(v=vs.85)

	// create and initialize an object that supports
	// the IDirectSoundCapture8 interface
	LPDIRECTSOUNDCAPTURE8 pDSCapture;
	hr = DirectSoundCaptureCreate8(NULL, &pDSCapture, NULL);

	// create capture buffer
	DSCBUFFERDESC dscbd;
	LPDIRECTSOUNDCAPTUREBUFFER pDSCBuffer;
	WAVEFORMATEX wfx = { WAVE_FORMAT_PCM
		, 2, 44100, 176400, 4, 16, 0
	};
	dscbd.dwSize = sizeof(DSCBUFFERDESC);
	dscbd.dwFlags = 0;
	dscbd.dwBufferBytes = wfx.nAvgBytesPerSec;
	dscbd.dwReserved = 0;
	dscbd.lpwfxFormat = &wfx;
	dscbd.dwFXCount = 0;
	dscbd.lpDSCFXDesc = NULL;
	hr = pDSCapture->CreateCaptureBuffer(&dscbd
		, &pDSCBuffer
		, NULL
	);

	// set capture notifications
	LPDIRECTSOUNDNOTIFY8 pDSNotify;
	HANDLE rghEvent[cEvents] = { 0 };
	DSBPOSITIONNOTIFY rgdsbpn[cEvents];
	hr = pDSCBuffer->QueryInterface(IID_IDirectSoundNotify, (LPVOID*)&pDSNotify);
	// create events
	for (int i = 0; i < cEvents; ++i)
	{
		rghEvent[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (NULL == rghEvent[i])
		{
			hr = GetLastError();
			return hr;
		}
	}
	// describe notifications
	rgdsbpn[0].dwOffset = (wfx.nAvgBytesPerSec / 2) - 1;
	rgdsbpn[0].hEventNotify = rghEvent[0];

	rgdsbpn[1].dwOffset = wfx.nAvgBytesPerSec - 1;
	rgdsbpn[1].hEventNotify = rghEvent[1];

	rgdsbpn[2].dwOffset = DSBPN_OFFSETSTOP;
	rgdsbpn[2].hEventNotify = rghEvent[2];
	// create notifications
	hr = pDSNotify->SetNotificationPositions(cEvents, rgdsbpn);
	pDSNotify->Release();

	// using the capture buffer
	pDSCBuffer->Start(DSCBSTART_LOOPING);

	OutputDebugString(L"bla\n");

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
		}
		else
		{
			// change text on button
			SendMessage(hWnd
				, WM_SETTEXT
				, (WPARAM)0
				, (LPARAM)L"Start"
			);
		}
		return (INT_PTR)TRUE;
	} // eof IDC_START_AUDIO_CAPTURE
	} // eof switch
	return (INT_PTR)FALSE;
}
