#pragma once
//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"
#include "Fmnt_26-01-2023_v3.h"

//*****************************************************************************
//*                     onWmInitDialog_DlgProc
//*****************************************************************************
BOOL onWmInitDialog_DlgProc(const HINSTANCE& hInst
	, const HWND& hDlg
)
{
	// check the //first// radiobutton
	SendMessage(GetDlgItem(hDlg, IDC_METRONOME)
		, BM_SETCHECK
		, (WPARAM)BST_CHECKED
		, (LPARAM)0
	);

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
)
{
	switch (LOWORD(wParam))
	{
	case IDC_START:
	{
		WCHAR pwszBuffer[64] = { '\0' };
		HWND hWnd = GetDlgItem(hDlg, IDC_START);
		SendMessage(hWnd
			, WM_GETTEXT
			, (WPARAM)64
			, (LPARAM)pwszBuffer
		);
		if (wcscmp(pwszBuffer, L"Start") == 0)
		{
			// change text on button
			SendMessage(hWnd
				, WM_SETTEXT
				, (WPARAM)0
				, (LPARAM)L"Stop"
			);
			if (SendMessage(GetDlgItem(hDlg, IDC_NOISE)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				//start_play(NOISE);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_NOTE)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				//start_play(NOTE);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_SWEEP)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				//start_play(SWEEP);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_CHORD)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				//start_play(CHORD);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_METRONOME)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				// 60 bpm ... 480 bpm
				//start_play(METRONOME);
			}
			if (SendMessage(GetDlgItem(hDlg, IDC_MELODY)
				, BM_GETSTATE
				, (WPARAM)0
				, (LPARAM)0) == BST_CHECKED)
			{
				//start_play(MELODY);
			}
		}
		if (wcscmp(pwszBuffer, L"Stop") == 0)
		{
			SendMessage(hWnd
				, WM_SETTEXT
				, (WPARAM)0
				, (LPARAM)L"Start"
			);
			// stop func playAudioStream() and let the thread die
			//g_flags = AUDCLNT_BUFFERFLAGS_SILENT;
		}
		return (INT_PTR)TRUE;
	} // eof IDC_START
	} // eof wsitch
	return (INT_PTR)FALSE;
}
