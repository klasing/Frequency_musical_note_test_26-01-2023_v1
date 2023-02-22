#pragma once
//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"
#include "Mixer_Fmnt_26-01-2023_v1.h"

//*****************************************************************************
//*                     global
//*****************************************************************************
WCHAR wszBuffer[BUFFER_MAX];

//*****************************************************************************
//*                     onWmInitDialog_DlgProc
//*****************************************************************************
BOOL onWmInitDialog_DlgProc(const HINSTANCE& hInst
	, const HWND& hDlg
)
{

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
//*                     onWmHscroll_DlgProc
//*****************************************************************************
BOOL onWmHscroll_DlgProc(const HWND& hDlg
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
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_LVOLUME_CHNL1))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_LVOLUME_CHNL1)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_LVOLUME_CHNL1"
				, track_pos
				, (track_pos == 0) ? 0. : std::exp(track_pos / 100.f) / M_E
			);
			OutputDebugString(wszBuffer);
			return EXIT_SUCCESS;
		}
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_RVOLUME_CHNL1))
		{
			track_pos = SendMessage(GetDlgItem(hDlg, IDC_RVOLUME_CHNL1)
				, TBM_GETPOS
				, (WPARAM)0
				, (LPARAM)0);

			swprintf_s(wszBuffer
				, (size_t)BUFFER_MAX
				, L"%s %d %f\n"
				, L"IDC_RVOLUME_CHNL1"
				, track_pos
				, (track_pos == 0) ? 0. : std::exp(track_pos / 100.f) / M_E
			);
			OutputDebugString(wszBuffer);
			return EXIT_SUCCESS;
		}
	} // eof TB_LINEDOWN | TB_LINEUP | TB_THUMBTRACK
	} // eof switch

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
	} // eof wsitch

	return (INT_PTR)FALSE;
}

//*****************************************************************************
//*                     onWmNotify_DlgProc
//*****************************************************************************
//BOOL onWmNotify_DlgProc(const HWND& hDlg
//	, const WPARAM& wParam
//	, const LPARAM& lParam
//)
//{
//	OutputDebugString(L"WM_NOTIFY\n");
//	return EXIT_SUCCESS;
//}
//	switch (((LPNMHDR)lParam)->code)
//	{
//	case NM_RELEASEDCAPTURE:
//	{
//		OutputDebugString(L"NM_RELEASEDCAPTURE\n");
//		switch (((LPNMHDR)lParam)->idFrom)
//		{
//		case IDC_LVOLUME_CHNL1:
//		{
//			OutputDebugString(L"IDC_LVOLUME_CHNL1\n");
//			break;
//		} // eof IDC_SLIDER_LVOLUME
//		case IDC_RVOLUME_CHNL1:
//		{
//			OutputDebugString(L"IDC_RVOLUME_CHNL1\n");
//			break;
//		} // eof IDC_SLIDER_RVOLUME
//		} // eof switch
//		break;
//	}
//	} // eof switch

//BOOL onWmHscroll_DlgProc(const HWND& hDlg
//	, const WPARAM& wParam
//	, const LPARAM& lParam
//)
//{
//	OutputDebugString(L"WM_HSCROLL\n");
//	switch (LOWORD(wParam))
//	{
//	case TB_LINEDOWN:
//	{
//		OutputDebugString(L"TB_LINEDOWN\n");
//		break;
//	} // eof TB_LINEDOWN
//	case TB_LINEUP:
//	{
//		OutputDebugString(L"TB_LINEUP\n");
//		break;
//	} // eof TB_LINEUP
//	case TB_THUMBTRACK:
//	{
//		OutputDebugString(L"TB_THUMBTRACK\n");
//		break;
//	} // eof TB_THUMBTRACK
//	} // eof switch
//
//	return EXIT_SUCCESS;
//}
//	int track_pos = 0;
//	if (LOWORD(wParam) == TB_THUMBTRACK)
//	{
//		if ((HWND)lParam == GetDlgItem(hDlg, IDC_LVOLUME_CHNL1))
//		{
//			track_pos = SendMessage(GetDlgItem(hDlg, IDC_LVOLUME_CHNL1)
//				, TBM_GETPOS
//				, (WPARAM)0
//				, (LPARAM)0);
//
//			swprintf_s(wszBuffer
//				, (size_t)BUFFER_MAX
//				, L"%s %d %f\n"
//				, L"IDC_LVOLUME_CHNL1"
//				, track_pos
//				, (track_pos == 0) ? 0. : std::exp(track_pos / 100.f) / M_E
//			);
//			OutputDebugString(wszBuffer);
//			return EXIT_SUCCESS;
//		}
//		if ((HWND)lParam == GetDlgItem(hDlg, IDC_RVOLUME_CHNL1))
//		{
//			track_pos = SendMessage(GetDlgItem(hDlg, IDC_LVOLUME_CHNL1)
//				, TBM_GETPOS
//				, (WPARAM)0
//				, (LPARAM)0);
//
//			swprintf_s(wszBuffer
//				, (size_t)BUFFER_MAX
//				, L"%s %d %f\n"
//				, L"IDC_RVOLUME_CHNL1"
//				, track_pos
//				, (track_pos == 0) ? 0. : std::exp(track_pos / 100.f) / M_E
//			);
//			OutputDebugString(wszBuffer);
//			return EXIT_SUCCESS;
//		}
//	}
//	if (LOWORD(wParam) == TB_LINEDOWN)
//	{
//		OutputDebugString(L" TB_LINEDOWN\n");
//	}
//
//	return EXIT_SUCCESS;
//}

