#pragma once
//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"
#include "Audio_Capture_01-03-2023_v1.h"
//****************************************************************************
//*                     global
//****************************************************************************
HRESULT hr;
HMMIO g_hmmio = NULL;
DWORD g_dwFlags = 0;
DWORD g_dwSizeWaveFile = 0;
//****************************************************************************
//*                     openWaveFile
//****************************************************************************
HRESULT openWaveFile(const LPWSTR& lpwszFileName
	, const DWORD& dwFlags
)
{
	// open .wav file, using default (8k) buffer size
	if (dwFlags == WAVEFILE_READ)
	{
		// for reading 
		g_hmmio = mmioOpen(lpwszFileName
			, NULL
			, MMIO_ALLOCBUF | MMIO_READ);
		if (g_hmmio == NULL) return E_FAIL;
		g_dwFlags = dwFlags;
	}
	else
	{
		// for writing
		g_hmmio = mmioOpen(lpwszFileName
			, NULL
			, MMIO_ALLOCBUF | MMIO_READWRITE | MMIO_CREATE);
		if (g_hmmio == NULL) return E_FAIL;
		g_dwFlags = dwFlags;
	}
	return S_OK;
}
//****************************************************************************
//*                     getSizeWaveFile
//****************************************************************************
DWORD getSizeWaveFile()
{
	return g_dwSizeWaveFile;
}
//*****************************************************************************
//*                     onWmInitDialog_DlgProc
//*****************************************************************************
BOOL onWmInitDialog_DlgProc(const HINSTANCE& hInst
	, const HWND& hDlg
)
{
	return EXIT_SUCCESS;
}
//****************************************************************************
//*                     closeWaveFile
//****************************************************************************
HRESULT closeWaveFile()
{
	if (g_dwFlags == WAVEFILE_READ)
	{
		// close .wav file that was opened for reading
		mmioClose(g_hmmio, 0);
		g_hmmio = NULL;
	}
	else
	{
		// close .wav file that was opened for writing
	}
	return S_OK;
}
//*****************************************************************************
//*                     start_audio_capture
//*****************************************************************************
BOOL start_audio_capture()
{
	OutputDebugString(L"start_audio_capture()\n");
	return EXIT_SUCCESS;
}
//*****************************************************************************
//*                     start_playback
//*****************************************************************************
BOOL start_playback()
{
	OutputDebugString(L"start_playback()\n");
	return EXIT_SUCCESS;
}
//*****************************************************************************
//*                     onWmInitDialog_DlgProc
//*****************************************************************************
BOOL onWmInitDialog_DlgProc(const HINSTANCE& hInst
	, const HWND& hDlg
)
{
	OutputDebugString(L"onWmInitDialog_DlgProc()\n");
	return EXIT_SUCCESS;
}
//*****************************************************************************
//*                     onWmSize_DlgProc
//*****************************************************************************
BOOL onWmSize_DlgProc(const HWND& hDlg
)
{
	OutputDebugString(L"onWmSize_DlgProc()\n");
	return EXIT_SUCCESS;
}
//*****************************************************************************
//*                     onWimOpen_DlgProc
//*****************************************************************************
BOOL onWimOpen_DlgProc()
{
	OutputDebugString(L"onWimOpen_DlgProc()\n");
	return EXIT_SUCCESS;
}
//*****************************************************************************
//*                     onWimData_DlgProc
//*****************************************************************************
BOOL onWimData_DlgProc()
{
	OutputDebugString(L"onWimData_DlgProc()\n");
	return EXIT_SUCCESS;
}
//*****************************************************************************
//*                     onWimClose_DlgProc
//*****************************************************************************
BOOL onWimClose_DlgProc()
{
	OutputDebugString(L"onWimClose_DlgProc()\n");
	return EXIT_SUCCESS;
}
//*****************************************************************************
//*                     onWomOpen_DlgProc
//*****************************************************************************
BOOL onWomOpen_DlgProc()
{
	OutputDebugString(L"onWomOpen_DlgProc()\n");
	return EXIT_SUCCESS;
}
//*****************************************************************************
//*                     onWomDone_DlgProc
//*****************************************************************************
BOOL onWomDone_DlgProc()
{
	OutputDebugString(L"onWomDone_DlgProc()\n");
	return EXIT_SUCCESS;
}
//*****************************************************************************
//*                     onWomClose_DlgProc
//*****************************************************************************
BOOL onWomClose_DlgProc()
{
	OutputDebugString(L"onWomClose_DlgProc()\n");
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
		return (INT_PTR)TRUE;
	} // eof IDC_START_AUDIO_CAPTURE
	case IDC_PLAYBACK:
	{
		OutputDebugString(L"IDC_PLAYBACK\n");
		return (INT_PTR)TRUE;
	} // eof IDC_PLAYBACK
	} // eof switch
	return (INT_PTR)FALSE;
}
