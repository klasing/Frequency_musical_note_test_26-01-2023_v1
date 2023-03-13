#pragma once
//*****************************************************************************
//*                     include
//*****************************************************************************
#include "framework.h"
#include "Audio_Capture_01-03-2023_v1.h"
//*****************************************************************************
//*                     createMapping
//*****************************************************************************
BOOL createMapping(const PWCHAR wszFileName)
{
	// 1) create file, for reading
	HANDLE hFile = CreateFile(wszFileName
		, GENERIC_READ
		, 0
		, NULL
		, OPEN_EXISTING
		, FILE_ATTRIBUTE_NORMAL
		, NULL
	);
	if (hFile == INVALID_HANDLE_VALUE) 
		return EXIT_FAILURE;
	// 2) create mapping
	HANDLE hFileMapping = CreateFileMapping(hFile
		, NULL
		, PAGE_READONLY
		, 0
		, pow(2, 32) // 4 Gb
		, NULL
	);
	if (hFileMapping == INVALID_HANDLE_VALUE)
		return EXIT_FAILURE;
	// 3) create view
	return EXIT_SUCCESS;
}