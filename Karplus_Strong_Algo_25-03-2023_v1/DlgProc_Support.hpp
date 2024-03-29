#pragma once
//*****************************************************************************
//*                     include
//*****************************************************************************
#include "framework.h"
#include "Karplus_Strong_Algo_25-03-2023_v1.h"

//****************************************************************************
//*                     global
//****************************************************************************
int aIdxNoteOpenStringg[NOF_STRING_GUITAR] = { 4, 11, 7, 2, 9, 4 };
WCHAR wchOctave[NOF_NOTE_OCTAVE] = { 'C', ' ', 'D', ' ', 'E', 'F', ' ', 'G', ' ', 'A', ' ', 'B' };
HFONT g_hFont = CreateFont(12	// cHeight
	, 0							// cWidth
	, 0							// cEscapement
	, 0							// cOrientation
	, FW_DONTCARE				// cWeight
	, FALSE						// bItalic
	, FALSE						// bUnderline
	, FALSE						// bStrikeOut
	, DEFAULT_CHARSET			// iCharSet
	, OUT_OUTLINE_PRECIS		// iOutPrecision
	, CLIP_DEFAULT_PRECIS		// iClipPrecision
	, CLEARTYPE_QUALITY			// iQuality
	, VARIABLE_PITCH			// iPitchAndFamily
	, NULL						// pszFaceName
);

//*****************************************************************************
//*                     onWmInitDialog_DlgProc
//*****************************************************************************
BOOL onWmInitDialog_DlgProc(const HINSTANCE& hInst
	, const HWND& hDlg
)
{
	OutputDebugString(L"onWmInitDialog_DlgProc\n");

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

		return (INT_PTR)TRUE;
	} // eof IDC_STRUM
	} // eof switch

	return (INT_PTR)FALSE;
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

	// horizontal line, representing guitar string
	int y = 20;
	for (int i = 0; i < NOF_STRING_GUITAR; i++)
	{
		graphics.DrawLine(&pen, 35, y, 500, y);
		y += 25;
	}
	// vertical line, representing fret on guitar
	// circle around note
	int x_vert_line = 35, y_top_circle = 0;
	for (int i = 0; i < NOF_NOTE_OCTAVE; i++)
	{
		// represent fret on guitar
		graphics.DrawLine(&pen, x_vert_line, 15, x_vert_line, 160);
		y_top_circle = 12;
		for (int j = 0; j < NOF_STRING_GUITAR; j++)
		{
			// draw circle around note
			Ellipse(hdc
				, x_vert_line + 10
				, y_top_circle
				, x_vert_line + 26
				, y_top_circle + 16
			);
			y_top_circle += 25;
		}
		x_vert_line += 35;
	}
	// set letter in circle, representing the note on a string
	// at a fret position
	std::wstring wstr = L" ";
	SelectObject(hdc, g_hFont);
	RECT rect;
	rect.left = 45;
	rect.top = 12 + 2;
	rect.right = rect.left + 16;
	rect.bottom = rect.top + 16;
	for (int i = 0; i < 12; i++)
	{
		rect.top = 12 + 2;
		rect.bottom = rect.top + 16;
		for (int j = 0; j < NOF_STRING_GUITAR; j++)
		{
			wstr[0] = wchOctave[++aIdxNoteOpenStringg[j] % NOF_NOTE_OCTAVE];
			if (wstr[0] != L' ')
			{
				DrawText(hdc, (LPCWSTR)wstr.c_str(), 1, &rect, DT_CENTER);
			}
			rect.top += 23 + 2;
			rect.bottom = rect.top + 16;
		}
		rect.left += 35;
		rect.right = rect.left + 16;
	}

	EndPaint(hDlg, &ps);

	return EXIT_SUCCESS;
}

// waste //////////////////////////////////////////////////////////////////////
/*
//*****************************************************************************
//*                     note
//*
//* the A string of a guitar is normally tuned to 110 Hz (A midi# 45)
//* => the low E string of a guitar is 82,41 Hz (E midi# 40)
//*
//* E  82,41 Hz midi# 40
//* A 110,00 Hz midi# 45
//* D 146,83 Hz midi# 50
//* G 196,00 Hz midi# 55
//* B 246,94 Hz midi# 59
//* E 329,63 Hz midi# 64
//*
//*  2 channel
//* 16 bit / (channel sample)
//*  2 * 16 = 32 bit / sample
//* (32 bit / sample) / 8 bit = 4 byte / sample
//*****************************************************************************

//*****************************************************************************
//*                     include
//*****************************************************************************
#include "framework.h"
#include "Karplus_Strong_Algo_25-03-2023_v1.h"

//*****************************************************************************
//*                     Note
//*****************************************************************************
class Note
{
public:
	//************************************************************************
	//*                 <<constructor>>
	//************************************************************************
	Note()
	{
		for (int i = 0; i < 128; i++)
		{
			aFreq[i] = PITCH_STANDARD_HZ
				* std::pow(2.0f, float(i - 69) / 12.0f);
		}
	}
	FLOAT aFreq[128] = { 0 };
};

//****************************************************************************
//*                     enum
//****************************************************************************
enum nMidi
{
	E = 40,
	A = 45,
	D = 50,
	G = 55,
	B = 59,
	e = 64
};

//****************************************************************************
//*                     extern
//****************************************************************************
extern HWND g_hDlg;

//****************************************************************************
//*                     global
//****************************************************************************
MMRESULT rc = MMSYSERR_NOERROR;
Note g_oNote;
// common
WAVEFORMATEX g_wfx{};
// audio playback
HANDLE g_hAudioPlayback = NULL;
DWORD g_dwAudioPlaybackId = 0;
HWAVEOUT g_hwo{};

DWORD g_nBlock = 0;
auto g_pVectorBuffer = std::unique_ptr<std::vector<float>[]>(
	new std::vector<float>[MAX_DATABLOCK]);
BYTE** g_pPlaybackBuffer = nullptr;
LPWAVEHDR g_who[MAX_DATABLOCK]{};
DWORD g_cBufferOut = 0;

std::mt19937_64 g_engine;
std::uniform_real_distribution<float> g_float_dist;

INT16 mode_play = 0;

//****************************************************************************
//*                     StringG
//****************************************************************************
class StringG
{
public:
	VOID init(nMidi f
		, const float& cSecondsToPlay
		, const float& decay_factor
	)  
	{
		frequency_hz = g_oNote.aFreq[f];
		dwBlock = round(cSecondsToPlay * frequency_hz);
		max_buffer = g_wfx.nBlockAlign * round(SAMPLE_RATE / frequency_hz);
		vectorBuffer[0].resize(max_buffer);
		ppPlaybackBuffer = new BYTE*[dwBlock];
		for (int i = 0; i < dwBlock; i++)
		{
			ppPlaybackBuffer[i] = new BYTE[max_buffer];
		}
		// initialize first buffer with random values
		// -1.0 < value < 1.0
		for (int i = 0; i < max_buffer; i++)
		{
			vectorBuffer[0][i] = 2. * g_float_dist(g_engine) - 1.;
			// transfer into playback buffer
			ppPlaybackBuffer[0][i] = 0x7F * vectorBuffer[0][i];
		}
		// iterate over the following buffer and
		// synthesize the Karplus-Strong waveform
		//float decay_factor = 0.980;
		float previous_value = 0.;
		for (int j = 1; j < dwBlock; j++)
		{
			vectorBuffer[j].resize(max_buffer);
			ppPlaybackBuffer[j] = new BYTE[max_buffer];
			for (int i = 0; i < max_buffer; i++)
			{
				vectorBuffer[j][i] = decay_factor
					* .5 * (vectorBuffer[j - 1][i] + previous_value);
				previous_value = vectorBuffer[j - 1][i];
				// transfer into playback buffer
				ppPlaybackBuffer[j][i] = 0x7F * vectorBuffer[j][i];
			}
		}
	}
//private:
	float frequency_hz = 0.;
	UINT max_buffer = 0;
	DWORD dwBlock = 0;
	std::vector<float> vectorBuffer[MAX_DATABLOCK]{};
	BYTE** ppPlaybackBuffer{};
};

//****************************************************************************
//*                     Guitar
//****************************************************************************
class Guitar
{
public:
	VOID init()
	{
		aoStringg[0].init(E, 1.0, 0.980);
		aoStringg[1].init(A, 1.0, 0.982);
		aoStringg[2].init(D, 1.0, 0.984);
		aoStringg[3].init(G, 1.0, 0.986);
		aoStringg[4].init(B, 1.0, 0.988);
		aoStringg[5].init(e, 1.0, 0.992);
	}
	VOID strum_prepare_header(const BOOL& bDownStrum
		, const UINT& rate_strum
	)
	{
		OutputDebugString(L"g_oGuiter.strum_prepare_header()\n");
		int aSeqString[6]{};
		if (bDownStrum)
		{
			aSeqString[0] = 0;
			aSeqString[1] = 1;
			aSeqString[2] = 2;
			aSeqString[3] = 3;
			aSeqString[4] = 4;
			aSeqString[5] = 5;
		}
		else
		{
			aSeqString[0] = 5;
			aSeqString[1] = 4;
			aSeqString[2] = 3;
			aSeqString[3] = 2;
			aSeqString[4] = 1;
			aSeqString[5] = 0;

		}
		
		for (int i = 0; i < CSTRINGG; i++)
		{
			// find max
			if (aoStringg[i].dwBlock
				* aoStringg[i].max_buffer
				+ i * rate_strum > dwStrumBufferLength)
			{
				dwStrumBufferLength = aoStringg[i].dwBlock
					* aoStringg[i].max_buffer
					+ i * rate_strum;
			}
		}
		pStrumBuffer = new BYTE[dwStrumBufferLength];

		// E-string
		for (int j = 0; j < aoStringg[0].dwBlock; j++)
		{
			for (int i = 0; i < aoStringg[0].max_buffer; i++)
			{
				pStrumBuffer[(j * aoStringg[0].max_buffer + i) + aSeqString[0] * rate_strum] =
					aoStringg[0].ppPlaybackBuffer[j][i];
			}
		}
		// A-string
		for (int j = 0; j <aoStringg[1].dwBlock; j++)
		{
			for (int i = 0; i < aoStringg[1].max_buffer; i++)
			{
				pStrumBuffer[(j * aoStringg[1].max_buffer + i) + aSeqString[1] * rate_strum] +=
					aoStringg[1].ppPlaybackBuffer[j][i];
			}
		}
		// D-string
		for (int j = 0; j <aoStringg[2].dwBlock; j++)
		{
			for (int i = 0; i < aoStringg[2].max_buffer; i++)
			{
				pStrumBuffer[(j * aoStringg[2].max_buffer + i) + aSeqString[2] * rate_strum] +=
					aoStringg[2].ppPlaybackBuffer[j][i];
			}
		}
		// G-string
		for (int j = 0; j < aoStringg[3].dwBlock; j++)
		{
			for (int i = 0; i < aoStringg[3].max_buffer; i++)
			{
				pStrumBuffer[(j *aoStringg[3].max_buffer + i) + aSeqString[3] * rate_strum] +=
					aoStringg[3].ppPlaybackBuffer[j][i];
			}
		}
		// B-string
		for (int j = 0; j < aoStringg[4].dwBlock; j++)
		{
			for (int i = 0; i < aoStringg[4].max_buffer; i++)
			{
				pStrumBuffer[(j * aoStringg[4].max_buffer + i) + aSeqString[4] * rate_strum] +=
					aoStringg[4].ppPlaybackBuffer[j][i];
			}
		}
		// e-string
		for (int j = 0; j < aoStringg[5].dwBlock; j++)
		{
			for (int i = 0; i < aoStringg[5].max_buffer; i++)
			{
				pStrumBuffer[(j * aoStringg[5].max_buffer + i) + aSeqString[5] * rate_strum] +=
					aoStringg[5].ppPlaybackBuffer[j][i];
			}
		}
	}
//private:
	StringG aoStringg[CSTRINGG];
	BYTE* pStrumBuffer = nullptr;
	DWORD dwStrumBufferLength = 0;
};

//****************************************************************************
//*                     globalEx
//****************************************************************************
Guitar g_oGuitar;

//*****************************************************************************
//*                     pluck_prepare_headerEx
//*****************************************************************************
BOOL
pluck_prepare_headerEx(const int& nStringg)
{
	OutputDebugString(L"pluck_prepare_headerEx\n");

	for (int i = 0; i < g_oGuitar.aoStringg[nStringg].dwBlock; i++)
	{
		g_who[i] = new WAVEHDR;
		g_who[i]->lpData = (LPSTR)g_oGuitar.aoStringg[nStringg].ppPlaybackBuffer[i];
		g_who[i]->dwBufferLength = g_oGuitar.aoStringg[nStringg].max_buffer;
		g_who[i]->dwFlags = 0;
		g_who[i]->dwLoops = 0;
		waveOutPrepareHeader(g_hwo, g_who[i], sizeof(WAVEHDR));
	}
	g_nBlock = g_oGuitar.aoStringg[nStringg].dwBlock;

	// start playing
	g_cBufferOut = 0;
	while (g_cBufferOut < g_nBlock)
	{
		waveOutWrite(g_hwo, g_who[g_cBufferOut++], sizeof(WAVEHDR));
	}

	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     pluck_prepare_header
//*****************************************************************************
//BOOL
//pluck_prepare_header(const float& frequency_hz)
//{
//	OutputDebugString(L"pluck_prepare_header\n");
//
//	float nofOfSecondToPlay = 1.5;
//	// g_wfx.nBlockAlign: nof bytes per sample = 4
//	UINT max_buffer = g_wfx.nBlockAlign * round(SAMPLE_RATE / frequency_hz);
//	g_nBlock = round(nofOfSecondToPlay * frequency_hz);
//
//	// initialize first buffer with random values
//	// -1.0 < value < 1.0
//	g_pVectorBuffer[0].resize(max_buffer);
//	g_pPlaybackBuffer = new BYTE*[MAX_DATABLOCK];
//	g_pPlaybackBuffer[0] = new BYTE[max_buffer];
//	for (UINT i = 0; i < max_buffer; i++)
//	{
//		g_pVectorBuffer[0][i] = 2. * g_float_dist(g_engine) - 1.;
//		g_pPlaybackBuffer[0][i] = 0x7F * g_pVectorBuffer[0][i];
//	}
//	g_who[0] = new WAVEHDR;
//	g_who[0]->lpData = (LPSTR)g_pPlaybackBuffer[0];
//	g_who[0]->dwBufferLength = max_buffer;
//	g_who[0]->dwFlags = 0;
//	g_who[0]->dwLoops = 0;
//	waveOutPrepareHeader(g_hwo, g_who[0], sizeof(WAVEHDR));
//
//	// iterate over the following buffer and
//	// synthesize the Karplus-Strong waveform
//	float decay_factor = 0.980;
//	float previous_value = 0.;
//	for (int j = 1; j < g_nBlock; j++)
//	{
//		g_pVectorBuffer[j].resize(max_buffer);
//		g_pPlaybackBuffer[j] = new BYTE[max_buffer];
//		for (UINT i = 0; i < max_buffer; i++)
//		{
//			g_pVectorBuffer[j][i] = decay_factor
//				* .5 * (g_pVectorBuffer[j - 1][i] + previous_value);
//			previous_value = g_pVectorBuffer[j - 1][i];
//			g_pPlaybackBuffer[j][i] = 0x7F * g_pVectorBuffer[j][i];
//		}
//		g_who[j] = new WAVEHDR;
//		g_who[j]->lpData = (LPSTR)g_pPlaybackBuffer[j];
//		g_who[j]->dwBufferLength = max_buffer;
//		g_who[j]->dwFlags = 0;
//		g_who[j]->dwLoops = 0;
//		waveOutPrepareHeader(g_hwo, g_who[j], sizeof(WAVEHDR));
//	}
//
//	// start playing
//	g_cBufferOut = 0;
//	while (g_cBufferOut < g_nBlock)
//	{
//		waveOutWrite(g_hwo, g_who[g_cBufferOut++], sizeof(WAVEHDR));
//	}
//
//	return EXIT_SUCCESS;
//}

	//g_oGuitar.strum_prepare_header(TRUE, 9400);

	//g_who[0] = new WAVEHDR;
	//g_who[0]->lpData = (LPSTR)g_oGuitar.pStrumBuffer;
	//g_who[0]->dwBufferLength = g_oGuitar.dwStrumBufferLength;
	//g_who[0]->dwFlags = 0;
	//g_who[0]->dwLoops = 0;
	//waveOutPrepareHeader(g_hwo, g_who[0], sizeof(WAVEHDR));

//*****************************************************************************
//*                     strum_prepare_headerEx
//*****************************************************************************
BOOL
strum_prepare_headerEx(const BOOL bDownStrum, const UINT& rate_strum)
{
	OutputDebugString(L"strum_prepare_headerEx\n");

	UINT aSequence[CSTRINGG]{};
	if (bDownStrum)
	{
		aSequence[0] = 0;
		aSequence[1] = 1;
		aSequence[2] = 2;
		aSequence[3] = 3;
		aSequence[4] = 4;
		aSequence[5] = 5;
	}
	else
	{
		aSequence[0] = 5;
		aSequence[1] = 4;
		aSequence[2] = 3;
		aSequence[3] = 2;
		aSequence[4] = 1;
		aSequence[5] = 0;

	}
	//UINT aSequence[CSTRINGG] = { 5,4,3,2,1,0 };
	//UINT aSequence[CSTRINGG] = {0,1,2,3,4,5};
	DWORD dwBufferLength = 0;
	for (int i = 0; i < CSTRINGG; i++)
	{
		// find max
		if (g_oGuitar.aoStringg[i].dwBlock
			* g_oGuitar.aoStringg[i].max_buffer 
			+ i * rate_strum > dwBufferLength)
		{
			dwBufferLength = g_oGuitar.aoStringg[i].dwBlock
				* g_oGuitar.aoStringg[i].max_buffer
				+ i * rate_strum;
		}
	}
	BYTE* pPlaybackBuffer = new BYTE[dwBufferLength];
	// E-string
	for (int j = 0; j < g_oGuitar.aoStringg[0].dwBlock; j++)
	{
		for (int i = 0; i < g_oGuitar.aoStringg[0].max_buffer; i++)
		{
			pPlaybackBuffer[(j * g_oGuitar.aoStringg[0].max_buffer + i) + aSequence[0] * rate_strum] =
				g_oGuitar.aoStringg[0].ppPlaybackBuffer[j][i];
		}
	}
	// A-string
	for (int j = 0; j < g_oGuitar.aoStringg[1].dwBlock; j++)
	{
		for (int i = 0; i < g_oGuitar.aoStringg[1].max_buffer; i++)
		{
			pPlaybackBuffer[(j * g_oGuitar.aoStringg[1].max_buffer + i) + aSequence[1] * rate_strum] +=
				g_oGuitar.aoStringg[1].ppPlaybackBuffer[j][i];
		}
	}
	// D-string
	for (int j = 0; j < g_oGuitar.aoStringg[2].dwBlock; j++)
	{
		for (int i = 0; i < g_oGuitar.aoStringg[2].max_buffer; i++)
		{
			pPlaybackBuffer[(j * g_oGuitar.aoStringg[2].max_buffer + i) + aSequence[2] * rate_strum] +=
				g_oGuitar.aoStringg[2].ppPlaybackBuffer[j][i];
		}
	}
	// G-string
	for (int j = 0; j < g_oGuitar.aoStringg[3].dwBlock; j++)
	{
		for (int i = 0; i < g_oGuitar.aoStringg[3].max_buffer; i++)
		{
			pPlaybackBuffer[(j * g_oGuitar.aoStringg[3].max_buffer + i) + aSequence[3] * rate_strum] +=
				g_oGuitar.aoStringg[3].ppPlaybackBuffer[j][i];
		}
	}
	// B-string
	for (int j = 0; j < g_oGuitar.aoStringg[4].dwBlock; j++)
	{
		for (int i = 0; i < g_oGuitar.aoStringg[4].max_buffer; i++)
		{
			pPlaybackBuffer[(j * g_oGuitar.aoStringg[4].max_buffer + i) + aSequence[4] * rate_strum] +=
				g_oGuitar.aoStringg[4].ppPlaybackBuffer[j][i];
		}
	}
	// e-string
	for (int j = 0; j < g_oGuitar.aoStringg[5].dwBlock; j++)
	{
		for (int i = 0; i < g_oGuitar.aoStringg[5].max_buffer; i++)
		{
			pPlaybackBuffer[(j * g_oGuitar.aoStringg[5].max_buffer + i) + aSequence[5] * rate_strum] +=
				g_oGuitar.aoStringg[5].ppPlaybackBuffer[j][i];
		}
	}
	g_who[0] = new WAVEHDR;
	g_who[0]->lpData = (LPSTR)pPlaybackBuffer;
	g_who[0]->dwBufferLength = dwBufferLength;
	g_who[0]->dwFlags = 0;
	g_who[0]->dwLoops = 0;
	waveOutPrepareHeader(g_hwo, g_who[0], sizeof(WAVEHDR));

	// start playing
	g_cBufferOut = 0;
	g_nBlock = 1;
	while (g_cBufferOut < g_nBlock)
	{
		waveOutWrite(g_hwo, g_who[g_cBufferOut++], sizeof(WAVEHDR));
	}

	// clean up
	//delete[] pPlaybackBuffer;
	//pPlaybackBuffer = nullptr;

	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     strum_prepare_header
//*****************************************************************************
BOOL
strum_prepare_header()
{
	OutputDebugString(L"strum_prepare_header\n");

	UINT rate_strum = 9600;
	DWORD dwBufferLength = 4 * (g_oGuitar.aoStringg[5].dwBlock
		* g_oGuitar.aoStringg[5].max_buffer);
	BYTE* pPlaybackBuffer = new BYTE[dwBufferLength];
	for (int j = 0; j < g_oGuitar.aoStringg[0].dwBlock; j++)
	{
		for (int i = 0; i < g_oGuitar.aoStringg[0].max_buffer; i++)
		{
			pPlaybackBuffer[j * g_oGuitar.aoStringg[0].max_buffer + i] =
				g_oGuitar.aoStringg[0].ppPlaybackBuffer[j][i];
		}
	}
	for (int j = 0; j < g_oGuitar.aoStringg[1].dwBlock; j++)
	{
		for (int i = 0; i < g_oGuitar.aoStringg[1].max_buffer; i++)
		{
			pPlaybackBuffer[(j * g_oGuitar.aoStringg[1].max_buffer + i) + rate_strum] +=
				g_oGuitar.aoStringg[1].ppPlaybackBuffer[j][i];
		}
	}
	for (int j = 0; j < g_oGuitar.aoStringg[2].dwBlock; j++)
	{
		for (int i = 0; i < g_oGuitar.aoStringg[2].max_buffer; i++)
		{
			pPlaybackBuffer[(j * g_oGuitar.aoStringg[2].max_buffer + i) + 2 * rate_strum] +=
				g_oGuitar.aoStringg[2].ppPlaybackBuffer[j][i];
		}
	}
	for (int j = 0; j < g_oGuitar.aoStringg[3].dwBlock; j++)
	{
		for (int i = 0; i < g_oGuitar.aoStringg[3].max_buffer; i++)
		{
			pPlaybackBuffer[(j * g_oGuitar.aoStringg[3].max_buffer + i) + 3 * rate_strum] +=
				g_oGuitar.aoStringg[3].ppPlaybackBuffer[j][i];
		}
	}
	for (int j = 0; j < g_oGuitar.aoStringg[4].dwBlock; j++)
	{
		for (int i = 0; i < g_oGuitar.aoStringg[4].max_buffer; i++)
		{
			pPlaybackBuffer[(j * g_oGuitar.aoStringg[4].max_buffer + i) + 4 * rate_strum] +=
				g_oGuitar.aoStringg[4].ppPlaybackBuffer[j][i];
		}
	}
	for (int j = 0; j < g_oGuitar.aoStringg[5].dwBlock; j++)
	{
		for (int i = 0; i < g_oGuitar.aoStringg[5].max_buffer; i++)
		{
			pPlaybackBuffer[(j * g_oGuitar.aoStringg[5].max_buffer + i) + 5 * rate_strum] +=
				g_oGuitar.aoStringg[5].ppPlaybackBuffer[j][i];
		}
	}
	g_who[0] = new WAVEHDR;
	g_who[0]->lpData = (LPSTR)pPlaybackBuffer;
	g_who[0]->dwBufferLength = dwBufferLength;
	g_who[0]->dwFlags = 0;
	g_who[0]->dwLoops = 0;
	waveOutPrepareHeader(g_hwo, g_who[0], sizeof(WAVEHDR));

	// start playing
	g_cBufferOut = 0;
	g_nBlock = 1;
	while (g_cBufferOut < g_nBlock)
	{
		waveOutWrite(g_hwo, g_who[g_cBufferOut++], sizeof(WAVEHDR));
	}

	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     audio_playback
//*****************************************************************************
DWORD WINAPI audio_playback(LPVOID lpVoid)
{
	BOOL bRet;
	MSG msg;
	while ((bRet = GetMessage(&msg, nullptr, 0, 0)) != 0)
	{
		// if bRet == -1: handle error
		switch (msg.message)
		{
		case MM_WOM_OPEN:
		{
			OutputDebugString(L"audio_playback MM_WOM_OPEN\n");

			if (mode_play == IDC_PLUCK)
			{
				// disable button IDC_PAUSE
				EnableWindow(GetDlgItem(g_hDlg, IDC_PLUCK), FALSE);

				pluck_prepare_headerEx(0); // 0 = E-string
				pluck_prepare_headerEx(1); // 1 = A-string
				pluck_prepare_headerEx(2); // 2 = D-string
				pluck_prepare_headerEx(3); // 3 = G-string
				pluck_prepare_headerEx(4); // 4 = B-string
				pluck_prepare_headerEx(5); // 5 = e-string
				//pluck_prepare_header(g_oNote.aFreq[E]);
				//pluck_prepare_header(g_oNote.aFreq[A]);
				//pluck_prepare_header(g_oNote.aFreq[D]);
				//pluck_prepare_header(g_oNote.aFreq[G]);
				//pluck_prepare_header(g_oNote.aFreq[B]);
				//pluck_prepare_header(g_oNote.aFreq[e]);
			}
			if (mode_play == IDC_STRUM)
			{
				// disable button IDC_PAUSE
				EnableWindow(GetDlgItem(g_hDlg, IDC_STRUM), FALSE);

				strum_prepare_headerEx(TRUE, 18800);
				strum_prepare_headerEx(FALSE, 18800);
				//strum_prepare_header();
			}

			break;
		} // eof MM_WOM_OPEN
		case MM_WOM_DONE:
		{
			OutputDebugString(L"audio_playback MM_WOM_DONE\n");

			if (g_cBufferOut == g_nBlock)
			{
				// all blocks are played
				waveOutClose(g_hwo);
			}

			break;
		} // eof MM_WOM_DONE
		case MM_WOM_CLOSE:
		{
			OutputDebugString(L"audio_playback MM_WOM_CLOSE\n");

			// clean up
			//for (int i = 0; i < g_nBlock; i++)
			//{
			//	delete[] g_pPlaybackBuffer[i];
			//}
			//delete[] g_pPlaybackBuffer;

			if (mode_play == IDC_PLUCK)
				// enable button IDC_PLUCK
				EnableWindow(GetDlgItem(g_hDlg, IDC_PLUCK), TRUE);
			if (mode_play == IDC_STRUM)
				// enable button IDC_STRUM
				EnableWindow(GetDlgItem(g_hDlg, IDC_STRUM), TRUE);

			// let this thread die
			return 0;
		} // eof MM_WOM_CLOSE
		} // eof switch
	}

	return 0;
}

//*****************************************************************************
//*                     start_audio_playback
//*****************************************************************************
BOOL start_audio_playback()
{
	OutputDebugString(L"start_audio_playback()\n");

	// start thread audio_playback
	g_hAudioPlayback = CreateThread(NULL
		, 0
		, audio_playback
		, (LPVOID)nullptr
		, 0 // run immediately
		, &g_dwAudioPlaybackId
	);
	// open wave out
	rc = waveOutOpen(&g_hwo
		, SPEAKER_HEADPHONE
		, &g_wfx
		, (DWORD)g_dwAudioPlaybackId
		, (DWORD)0
		, CALLBACK_THREAD
	);

	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     onWmInitDialog_DlgProc
//*****************************************************************************
BOOL onWmInitDialog_DlgProc(const HINSTANCE& hInst
	, const HWND& hDlg
)
{
	OutputDebugString(L"onWmInitDialog_DlgProc\n");

	// initialize waveformat
	g_wfx.nChannels = 2;
	// make compatible with project Mixer_Fmnt_26-01-2023_v1
	g_wfx.nSamplesPerSec = SAMPLE_RATE;
	g_wfx.wFormatTag = WAVE_FORMAT_PCM;
	g_wfx.wBitsPerSample = 16;
	g_wfx.nBlockAlign = g_wfx.nChannels * g_wfx.wBitsPerSample / 8;
	g_wfx.nAvgBytesPerSec = g_wfx.nSamplesPerSec * g_wfx.nBlockAlign;
	g_wfx.cbSize = 0;

	g_oGuitar.init();

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

//****************************************************************************
//*                     globalEx
//****************************************************************************
int aIdxNoteOnStringg[CSTRINGG] = { 4, 11, 7, 2, 9, 4 };
WCHAR wchOctave[12] = { 'C', ' ', 'D', ' ', 'E', 'F', ' ', 'G', ' ', 'A', ' ', 'B' };
HFONT hFont = CreateFont(12	// cHeight
	, 0						// cWidth
	, 0						// cEscapement
	, 0						// cOrientation
	, FW_DONTCARE			// cWeight
	, FALSE					// bItalic
	, FALSE					// bUnderline
	, FALSE					// bStrikeOut
	, DEFAULT_CHARSET		// iCharSet
	, OUT_OUTLINE_PRECIS	// iOutPrecision
	, CLIP_DEFAULT_PRECIS	// iClipPrecision
	, CLEARTYPE_QUALITY		// iQuality
	, VARIABLE_PITCH		// iPitchAndFamily
	, NULL					// pszFaceName
);

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

	// represent guitar string
	graphics.DrawLine(&pen, 35, 20, 500, 20);
	graphics.DrawLine(&pen, 35, 45, 500, 45);
	graphics.DrawLine(&pen, 35, 70, 500, 70);
	graphics.DrawLine(&pen, 35, 95, 500, 95);
	graphics.DrawLine(&pen, 35, 120, 500, 120);
	graphics.DrawLine(&pen, 35, 145, 500, 145);

	int xvl = 35, xl = 45, yl = 0;
	for (int i = 0; i < 12; i++)
	{
		// represent fret on guitar
		graphics.DrawLine(&pen, xvl, 15, xvl, 160);
		yl = 12;
		for (int j = 0; j < CSTRINGG; j++)
		{
			// draw circle around note
			Ellipse(hdc, xl, yl, xl + 16, yl + 16);
			yl += 25;
		}
		xl += 35;
		xvl += 35;
	}

	std::wstring wstr = L" ";
	SelectObject(hdc, hFont);
	RECT rect;
	rect.left = 45;
	rect.top = 12 + 2;
	rect.right = rect.left + 16;
	rect.bottom = rect.top + 16;
	for (int i = 0; i < 12; i++)
	{
		rect.top = 12 + 2;
		rect.bottom = rect.top + 16;
		for (int j = 0; j < CSTRINGG; j++)
		{
			wstr[0] = wchOctave[++aIdxNoteOnStringg[j] % 12];
			DrawText(hdc, (LPCWSTR)wstr.c_str(), 1, &rect, DT_CENTER);
			rect.top += 23 + 2;
			rect.bottom = rect.top + 16;
		}
		rect.left += 35;
		rect.right = rect.left + 16;
	}

	//Ellipse(hdc, 45, 12, 61, 28);
	//Ellipse(hdc, 45, 37, 61, 53);

	//SelectObject(hdc, hFont);
	//RECT rect;
	//rect.left = 45;
	//rect.top = 12 + 2;
	//rect.right = rect.left + 16;
	//rect.bottom = rect.top + 16;
	//std::wstring wstr = L" ";
	//for (int i = 0; i < 13; i++)
	//{
	//	wstr[0] = wch[++n[0] % 12];
	//	DrawText(hdc, (LPCWSTR)wstr.c_str(), 1, &rect, DT_CENTER);
	//	rect.left += 35;
	//	rect.right = rect.left + 16;
	//}
	//for (int j = 0; j < CSTRINGG; j++)
	//{
	//	wstr[0] = wch[++n[j] % 12];
	//	DrawText(hdc, (LPCWSTR)wstr.c_str(), 1, &rect, DT_CENTER);
	//	rect.top += 23 + 2;
	//	rect.bottom = rect.top + 16;
	//}

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

		mode_play = IDC_PLUCK;
		start_audio_playback();

		return (INT_PTR)TRUE;
	} // eof IDC_PLUCK
	case IDC_STRUM:
	{
		OutputDebugString(L"IDC_STRUM\n");

		mode_play = IDC_STRUM;
		start_audio_playback();

		return (INT_PTR)TRUE;
	} // eof IDC_STRUM
	} // eof switch

	return (INT_PTR)FALSE;
}
*/
// waste //////////////////////////////////////////////////////////////////////
/*
//*****************************************************************************
//*                     prepare_header1
//*****************************************************************************
BOOL
prepare_header1()
{
	OutputDebugString(L"prepare_header1()\n");

	float g_frequency_hz = g_oNote.aFreq[69];
	float delta = 2.f * g_frequency_hz * float(M_PI / SAMPLE_RATE);
	float phase = 0.f;
	g_nBlock = SAMPLE_RATE / DATABLOCK_SIZE;
	for (UINT32 i = 0; i < g_nBlock; i++)
	{
		g_pPlaybackBuffer[i] = new BYTE[DATABLOCK_SIZE];

		// NOISE
		//for (int j = 0; j < DATABLOCK_SIZE; j++)
		//{
		//	g_fData[j] = 0x7F * g_float_dist(g_engine);
		//	*((BYTE*)g_pPlaybackBuffer[i] + j) = g_fData[j];
		//}
		// PURE SINE WAVE
		for (int j = 0; j < DATABLOCK_SIZE - 1; j += 2)
		{
			float next_sample = 0x7F * std::sin(phase);
			phase = std::fmod(phase + delta, 2.f * static_cast<float>(M_PI));
			*((BYTE*)g_pPlaybackBuffer[i] + j) = next_sample;
			*((BYTE*)g_pPlaybackBuffer[i] + j + 1) = next_sample;
		}

		g_who[i] = new WAVEHDR;
		g_who[i]->lpData = (LPSTR)g_pPlaybackBuffer[i];
		g_who[i]->dwBufferLength = DATABLOCK_SIZE;
		g_who[i]->dwFlags = 0;
		g_who[i]->dwLoops = 0;

		waveOutPrepareHeader(g_hwo, g_who[i], sizeof(WAVEHDR));
	}
	// start playing
	g_cBufferOut = 0;
	while (g_cBufferOut < g_nBlock)
	{
		waveOutWrite(g_hwo, g_who[g_cBufferOut++], sizeof(WAVEHDR));
	}

	return EXIT_SUCCESS;
}
*/