#pragma once
//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"
#include "Audio_Capture_01-03-2023_v1.h"
//****************************************************************************
//*                     open_input_device
//****************************************************************************
BOOL open_input_device()
{
	MMRESULT rc;
	WAVEINCAPS wic{};
	UINT nDevId;
	UINT nMaxDevices = waveInGetNumDevs();

	for (nDevId = 0; nDevId < nMaxDevices; nDevId++)
	{
		rc = waveInGetDevCaps(nDevId, &wic, sizeof(wic));
		if (rc == MMSYSERR_NOERROR)
		{

		}
		else
		{

		}
	}
	return EXIT_SUCCESS;
}
// waste ///////////////////////////////////////////////////////////////////////
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
