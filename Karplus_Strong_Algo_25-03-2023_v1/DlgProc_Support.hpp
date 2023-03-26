#pragma once
//*****************************************************************************
//*                     note
//*****************************************************************************

//*****************************************************************************
//*                     include
//*****************************************************************************
#include "framework.h"
#include "Karplus_Strong_Algo_25-03-2023_v1.h"

//*****************************************************************************
//*                     define
//*****************************************************************************
#define MAX_RINGBUFFER 10

//****************************************************************************
//*                     extern
//****************************************************************************
extern HWND g_hDlg;

//****************************************************************************
//*                     global
//****************************************************************************

//*****************************************************************************
//*                     onWmInitDialog_DlgProc
//*****************************************************************************
BOOL onWmInitDialog_DlgProc(const HINSTANCE& hInst
	, const HWND& hDlg
)
{
	OutputDebugString(L"onWmInitDialog_DlgProc\n");

	float energy_decay_factor = 1.;
	// test a ringbuffer
	std::vector<float> ring_buffer;
	ring_buffer.resize(MAX_RINGBUFFER);
	for (int i = 0; i < 15; i++)
	{
		int idx = i % MAX_RINGBUFFER;
		ring_buffer.at(idx) = (float)i;
	}
	// test feedback mechanism
	float value_feedback = 0.;
	value_feedback = energy_decay_factor
		* .5 * (ring_buffer[0] + ring_buffer[1]);
	ring_buffer.erase(ring_buffer.begin());
	ring_buffer.push_back(value_feedback);

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
//*                     onWmPaint_DlgProc
//*****************************************************************************
BOOL onWmPaint_DlgProc(const HWND& hDlg
)
{
	OutputDebugString(L"onWmPaint_DlgProc\n");

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hDlg, &ps);
	// TODO: Add any drawing code that uses hdc here...

	Graphics graphics(hdc);
	Pen      pen(Color(0xFF, 0, 0, 0));
	graphics.DrawLine(&pen, 30, 15, 30, 160);

	graphics.DrawLine(&pen, 30, 20, 500, 20);
	graphics.DrawLine(&pen, 30, 45, 500, 45);
	graphics.DrawLine(&pen, 30, 70, 500, 70);
	graphics.DrawLine(&pen, 30, 95, 500, 95);
	graphics.DrawLine(&pen, 30, 120, 500, 120);
	graphics.DrawLine(&pen, 30, 145, 500, 145);

	EndPaint(hDlg, &ps);

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
	OutputDebugString(L"onWmCommand_DlgProc()\n");

	switch (LOWORD(wParam))
	{
	case IDC_PLUCK:
	{
		OutputDebugString(L"IDC_PLUCK\n");

		return (INT_PTR)TRUE;
	} // eof IDC_PLUCK
	case IDC_STRUM:
	{
		OutputDebugString(L"IDC_STRUM\n");

		return (INT_PTR)TRUE;
	} // eof IDC_STRUM
	} // eof switch

	return (INT_PTR)FALSE;
}
