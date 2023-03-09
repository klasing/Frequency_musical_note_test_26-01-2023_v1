#pragma once
//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"
#include "Audio_Capture_01-03-2023_v1.h"

//****************************************************************************
//*                     define
//****************************************************************************
#define WAVEFILE_READ	1
#define WAVEFILE_WRITE	2

#define SAFE_DELETE(p)       { if (p) { delete (p); (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[](p); (p)=NULL; } }
#define DXTRACE_ERR(str,hr) (hr)

#define FAILED(hr) (((HRESULT)(hr)) < 0)

//****************************************************************************
//*                     prototype
//****************************************************************************
HRESULT readMMIO();
HRESULT resetWaveFile();
HRESULT writeMMIO(WAVEFORMATEX* pwfxDest);

//****************************************************************************
//*                     global
//****************************************************************************
//MMRESULT rc = MMSYSERR_NOERROR;
WAVEFORMATEX* g_pwfx = NULL;
HMMIO g_hmmio = NULL;
MMCKINFO g_ck;
MMCKINFO g_ckRiff;
DWORD g_dwSize = 0;
MMIOINFO g_mmioinfoOut;
DWORD g_dwFlags;
BOOL g_bIsReadingFromMemory = FALSE;
BYTE* g_pbData;
BYTE* g_pbDataCur;
ULONG g_ulDataSize;
CHAR* g_pResourceBuffer = NULL;

//****************************************************************************
//*                     openWaveFile
//*
//* open a wave file for reading/writing
//****************************************************************************
HRESULT openWaveFile(LPWSTR strFileName
    , WAVEFORMATEX* pwfx
    , DWORD dwFlags
)
{
    HRESULT hr = S_OK;

    g_dwFlags = dwFlags;
    g_bIsReadingFromMemory = FALSE;

    if (g_dwFlags == WAVEFILE_READ)
    {
		if (strFileName == NULL)
			return E_INVALIDARG;
		SAFE_DELETE_ARRAY(g_pwfx);

		g_hmmio = mmioOpen(strFileName, NULL, MMIO_ALLOCBUF | MMIO_READ);

		if (NULL == g_hmmio)
		{
			HRSRC hResInfo;
			HGLOBAL hResData;
			DWORD dwSize;
			VOID* pvRes;

			// loading it as a file failed, so try it as a resource
			if (NULL == (hResInfo = FindResource(NULL, strFileName, L"WAVE")))
			{
				if (NULL == (hResInfo = FindResource(NULL, strFileName, L"WAV")))
					return DXTRACE_ERR(L"FindResource", E_FAIL);
			}

			if (NULL == (hResData = LoadResource(GetModuleHandle(NULL)
				, hResInfo)))
				return DXTRACE_ERR(L"LoadResource", E_FAIL);
			
			if (0 == (dwSize = SizeofResource(GetModuleHandle(NULL)
				, hResInfo)))
				return DXTRACE_ERR(L"SizeofResource", E_FAIL);
			
			if (NULL == (pvRes = LockResource(hResData)))
				return DXTRACE_ERR(L"LockResource", E_FAIL);

			g_pResourceBuffer = new CHAR[dwSize];
			if (g_pResourceBuffer == NULL)
				return DXTRACE_ERR(L"new", E_OUTOFMEMORY);
			memcpy(g_pResourceBuffer, pvRes, dwSize);

			MMIOINFO mmioInfo;
			ZeroMemory(&mmioInfo, sizeof(mmioInfo));
			mmioInfo.fccIOProc = FOURCC_MEM;
			mmioInfo.cchBuffer = dwSize;
			mmioInfo.pchBuffer = (CHAR*)g_pResourceBuffer;

			g_hmmio = mmioOpen(NULL, &mmioInfo, MMIO_ALLOCBUF | MMIO_READ);
		}

		if (FAILED(hr = readMMIO()))
		{
			// ReadMMIO will fail if its not a wave file
			mmioClose(g_hmmio, 0);
			return DXTRACE_ERR(L"readMMIO", hr);
		}

		if (FAILED(hr = resetWaveFile()))
			return DXTRACE_ERR(L"resetWaveFile", hr);

		// after the reset the size of the wav file is m_ck.cksize
		// so store it now
		g_dwSize = g_ck.cksize;
    }
    else
    {
        g_hmmio = mmioOpen(strFileName
            , NULL
            , MMIO_ALLOCBUF | MMIO_READWRITE | MMIO_CREATE
        );

        if (NULL == g_hmmio)
            return DXTRACE_ERR(L"mmioOpen", E_FAIL);

        if (FAILED(hr = writeMMIO(pwfx)))
        {
            mmioClose(g_hmmio, 0);
            return DXTRACE_ERR(L"writeMMIO", hr);
        }

        if (FAILED(hr = resetWaveFile()))
            return DXTRACE_ERR(L"resetWaveFile", hr);
    }

    return hr;
}

//****************************************************************************
//*                     openWaveFromMemory
//****************************************************************************
HRESULT openWaveFromMemory(BYTE* pbData
	, ULONG ulDataSize
	, WAVEFORMATEX* pwfx
	, DWORD dwFlags
)
{
	g_pwfx = pwfx;
	g_ulDataSize = ulDataSize;
	g_pbData = pbData;
	g_pbDataCur = g_pbData;
	g_bIsReadingFromMemory = TRUE;

	if (dwFlags != WAVEFILE_READ)
		return E_NOTIMPL;

	return S_OK;
}

//****************************************************************************
//*                     readMMIO
//*
//* support function for reading from a multimedia I/O stream
//* g_hmmio must be valid before calling, this function uses g_hmmio to
//* update g_ckRiff, and g_ck
//****************************************************************************
HRESULT readMMIO()
{
	// chunk info, for general use
	MMCKINFO ckIn;
	// temp PCM structure to load in
	PCMWAVEFORMAT pcmWaveFormat;

	memset(&ckIn, 0, sizeof(ckIn));

	g_pwfx = NULL;

	if ((0 != mmioDescend(g_hmmio, &g_ckRiff, NULL, 0)))
		return DXTRACE_ERR(L"mmioDescend", E_FAIL);

	// here things go wrong
	// check to make sure this is a valid wave file
	if ((g_ckRiff.ckid != FOURCC_RIFF) ||
		(g_ckRiff.fccType != mmioFOURCC('W','A','V','E')))
		return DXTRACE_ERR(L"mmioFOURCC", E_FAIL);

	// search the input file for the 'fmt' chunk
	ckIn.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if ((0 != mmioDescend(g_hmmio, &ckIn, &g_ckRiff, MMIO_FINDCHUNK)))
		return DXTRACE_ERR(L"mmioDescend", E_FAIL);

	// expect the 'fmt' chunk to be at least as large as <PCMWAVEFORMAT>
	// if there are extra parameters at the end, we'll ignore them
	if (ckIn.cksize < (LONG)sizeof(PCMWAVEFORMAT))
		return DXTRACE_ERR(L"sizeof(PCMWAVEFORMAT)", E_FAIL);

	// read the 'fmt ' chunk into pcmWaveFormat
	if (mmioRead(g_hmmio, (HPSTR)&pcmWaveFormat,
		sizeof(pcmWaveFormat)) != sizeof(pcmWaveFormat))
		return DXTRACE_ERR(L"mmioRead", E_FAIL);

	// allocat the waveformatex, but if its not pcm format, read the next
	// word, and thats how many extra bytes to allocate
	if (pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM)
	{
		g_pwfx = (WAVEFORMATEX*)new CHAR[sizeof(WAVEFORMATEX)];
		if (NULL == g_pwfx)
			return DXTRACE_ERR(L"g_pwfx", E_FAIL);

		// copy the bytes from the pcm structure to the waveformatex structure
		memcpy(g_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat));
		g_pwfx->cbSize = 0;
	}
	else
	{
		// read in the length of extra bytes
		WORD cbExtraBytes = 0L;
		if (mmioRead(g_hmmio, (CHAR*)&cbExtraBytes, sizeof(WORD)) != sizeof(WORD))
			return DXTRACE_ERR(L"mmioRead", E_FAIL);

		g_pwfx = (WAVEFORMATEX*)new CHAR[sizeof(WAVEFORMATEX) + cbExtraBytes];
		if (NULL == g_pwfx)
			return DXTRACE_ERR(L"new", E_FAIL);

		// copy the bytes from the pcm structure to the waveformatex structure
		memcpy(g_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat));
		g_pwfx->cbSize = cbExtraBytes;

		// now, read those extra bytes into the structure, if cbExtraAlloc != 0
		if (mmioRead(g_hmmio, (CHAR*)(((BYTE*)&(g_pwfx->cbSize)) + sizeof(WORD))
			, cbExtraBytes) != cbExtraBytes)
		{
			SAFE_DELETE(g_pwfx);
			return DXTRACE_ERR(L"mmioRead", E_FAIL);
		}
	}

	// ascend the input file out of the 'fmt ' chunk
	if (0 != mmioAscend(g_hmmio, &ckIn, 0))
	{
		SAFE_DELETE(g_pwfx);
		return DXTRACE_ERR(L"mmioRead", E_FAIL);
	}

	return S_OK;
}

//****************************************************************************
//*                     getSizeWaveFile
//****************************************************************************
DWORD getSizeWaveFile()
{
	return g_dwSize;
}

//****************************************************************************
//*                     resetWaveFile
//****************************************************************************
HRESULT resetWaveFile()
{
	if (g_bIsReadingFromMemory)
	{
		g_pbDataCur = g_pbData;
	}
	else
	{
		if (g_hmmio == NULL)
			return CO_E_NOTINITIALIZED;

		if (g_dwFlags == WAVEFILE_READ)
		{
			// seek to the data
			if (-1 == mmioSeek(g_hmmio, g_ckRiff.dwDataOffset + sizeof(FOURCC),
				SEEK_SET))
				return DXTRACE_ERR(L"mmioSeek", E_FAIL);

			// search the input file for the 'data' chunk.
			g_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
			if (0 != mmioDescend(g_hmmio, &g_ck, &g_ckRiff, MMIO_FINDCHUNK))
				return DXTRACE_ERR(L"mmioDescend", E_FAIL);
		}
		else
		{
			// Create the 'data' chunk that holds the waveform samples.
			g_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
			g_ck.cksize = 0;

			if (0 != mmioCreateChunk(g_hmmio, &g_ck, 0))
				return DXTRACE_ERR(L"mmioCreateChunk", E_FAIL);

			if (0 != mmioGetInfo(g_hmmio, &g_mmioinfoOut, 0))
				return DXTRACE_ERR(L"mmioGetInfo", E_FAIL);
		}
	}

	return S_OK;
}

//****************************************************************************
//*                     readWaveFile
//*
//* read section of data from a wave file into pBuffer and return how much
//* is read in pdwSizeRead, reading not more than dwSizeToRead
//* this uses g_ck to determine where to start reading from, 
//* so subsequent calls will continue to read where the last left off unless
//* resetWaveFile() is called
//****************************************************************************
HRESULT readWaveFile(BYTE* pBuffer
	, DWORD dwSizeToRead
	, DWORD* pdwSizeRead
)
{
	if (g_bIsReadingFromMemory)
	{
		if (g_pbDataCur == NULL)
			return CO_E_NOTINITIALIZED;
		if (pdwSizeRead != NULL)
			*pdwSizeRead = 0;

		if ((BYTE*)(g_pbDataCur + dwSizeToRead) >
			(BYTE*)(g_pbData + g_ulDataSize))
		{
			dwSizeToRead = g_ulDataSize - (DWORD)(g_pbDataCur - g_pbData);
		}
// disable warning about warning number '22104' being out of range		
#pragma warning(disable: 4616)
// disable PREfast warning during static code analysis
#pragma warning(disable: 22104)
		CopyMemory(pBuffer, g_pbDataCur, dwSizeToRead);
#pragma warning(default: 22104)
#pragma warning(default: 4616)
		if (pdwSizeRead != NULL)
			*pdwSizeRead = dwSizeToRead;

		return S_OK;
	}
	else
	{
		// current status of g_hmmio
		MMIOINFO mmioinfoIn;

		if (g_hmmio == NULL)
			return CO_E_NOTINITIALIZED;
		if (pBuffer == NULL || pdwSizeRead == NULL)
			return E_INVALIDARG;

		*pdwSizeRead = 0;

		if (0 != mmioGetInfo(g_hmmio, &mmioinfoIn, 0))
			return DXTRACE_ERR(L"mmioGetInfo", E_FAIL);

		UINT cbDataIn = dwSizeToRead;
		if (cbDataIn > g_ck.cksize)
			cbDataIn = g_ck.cksize;

		g_ck.cksize -= cbDataIn;

		for (DWORD cT = 0; cT < cbDataIn; cT++)
		{
			// copy the bytes from io to the buffer
			if (mmioinfoIn.pchNext == mmioinfoIn.pchEndRead)
			{
				if (0 != mmioAdvance(g_hmmio, &mmioinfoIn, MMIO_READ))
					return DXTRACE_ERR(L"mmioAdvance", E_FAIL);

				if (mmioinfoIn.pchNext == mmioinfoIn.pchEndRead)
					return DXTRACE_ERR(L"mmioinfoIn.pchNext", E_FAIL);
			}

			// actual copy
			*((BYTE*)pBuffer + cT) = *((BYTE*)mmioinfoIn.pchNext);
			mmioinfoIn.pchNext++;
		}

		if (0 != mmioSetInfo(g_hmmio, &mmioinfoIn, 0))
			return DXTRACE_ERR(L"mmioSetInfo", E_FAIL);

		*pdwSizeRead = cbDataIn;

		return S_OK;
	}
}

//****************************************************************************
//*                     closeWaveFile
//****************************************************************************
HRESULT closeWaveFile()
{
	if (g_dwFlags == WAVEFILE_READ)
	{
		if (g_hmmio != NULL)
		{
			mmioClose(g_hmmio, 0);
			g_hmmio = NULL;
		}
		SAFE_DELETE_ARRAY(g_pResourceBuffer);
	}
	else
	{
		g_mmioinfoOut.dwFlags |= MMIO_DIRTY;

		if (g_hmmio == NULL)
			return CO_E_NOTINITIALIZED;

		if (0 != mmioSetInfo(g_hmmio, &g_mmioinfoOut, 0))
			return DXTRACE_ERR(L"mmioSetInfo", E_FAIL);

		// ascend the output file out of the 'data' chunk -- this will cause
		// the chunk size of the 'data' chunk to be written
		if (0 != mmioAscend(g_hmmio, &g_ck, 0))
			return DXTRACE_ERR(L"mmioAscend", E_FAIL);

		// do this here instead
		if (0 != mmioAscend(g_hmmio, &g_ckRiff, 0))
			return DXTRACE_ERR(L"mmioAscend", E_FAIL);

		mmioSeek(g_hmmio, 0, SEEK_SET);

		if (0 != (INT)mmioDescend(g_hmmio, &g_ckRiff, NULL, 0))
			return DXTRACE_ERR(L"mmioDescend", E_FAIL);

		g_ck.ckid = mmioFOURCC('f', 'a', 'c', 't');

		if (0 == mmioDescend(g_hmmio, &g_ck, &g_ckRiff, MMIO_FINDCHUNK))
		{
			DWORD dwSamples = 0;
			mmioWrite(g_hmmio, (HPSTR)&dwSamples, sizeof(DWORD));
			mmioAscend(g_hmmio, &g_ck, 0);
		}

		// ascend the output file out of the 'RIFF' chunk -- this will cause
		// the chunk size of the 'RIFF' chunk to be written
		if (0 != mmioAscend(g_hmmio, &g_ckRiff, 0))
			return DXTRACE_ERR(L"mmioAscend", E_FAIL);

		mmioClose(g_hmmio, 0);
		g_hmmio = NULL;
	}

	return S_OK;
}

//****************************************************************************
//*                     writeMMIO
//*
//* support function for reading from a multimedia I/O stream
//* pwfxDest is the WAVEFORMATEX for this new file
//* g_hmmio must be valid before calling, this function uses g_hmmio to
//* update g_ckRiff, and g_ck
//****************************************************************************
HRESULT writeMMIO(WAVEFORMATEX* pwfxDest)
{
	// contains the actual fact chunk
	// garbage until WaveCloseWriteFile
	DWORD dwFactChunk;
	MMCKINFO ckOut1;

	memset(&ckOut1, 0, sizeof(ckOut1));

	dwFactChunk = (DWORD)-1;

	// create the output file RIFF chunk of form type 'WAVE'
	g_ckRiff.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	g_ckRiff.cksize = 0;

	if (0 != mmioCreateChunk(g_hmmio, &g_ckRiff, MMIO_CREATERIFF))
		return DXTRACE_ERR(L"mmioCreateChunk", E_FAIL);

	// we are now descended into the 'RIFF' chunk we just created
	// now create the 'fmt ' chunk
	// since we know the size of this chunk,
	// specify it in the MMCKINFO structure so MMIO doesn't have to seek
	// back and set the chunk size after ascending from the chunk
	g_ck.ckid = mmioFOURCC('f', 'm', 't', ' ');
	g_ck.cksize = sizeof(PCMWAVEFORMAT);

	if (0 != mmioCreateChunk(g_hmmio, &g_ck, 0))
		return DXTRACE_ERR(L"mmioCreateChunk", E_FAIL);

	// write the PCMWAVEFORMAT structure to the 'fmt ' chunk if its that type
	if (pwfxDest->wFormatTag == WAVE_FORMAT_PCM)
	{
		if (mmioWrite(g_hmmio
			, (HPSTR)pwfxDest
			, sizeof(PCMWAVEFORMAT)) != sizeof(PCMWAVEFORMAT))
			return DXTRACE_ERR(L"mmioWrite", E_FAIL);
	}
	else
	{
		// write the variable length size
		if ((UINT)mmioWrite(g_hmmio
			, (HPSTR)pwfxDest
			, sizeof(*pwfxDest) + pwfxDest->cbSize) !=
			(sizeof(*pwfxDest) + pwfxDest->cbSize))
			return DXTRACE_ERR(L"mmioWrite", E_FAIL);
	}

	// ascend out of the 'fmt ' chunk, back into the 'RIFF' chunk
	if (0 != mmioAscend(g_hmmio, &g_ck, 0))
		return DXTRACE_ERR(L"mmioAscend", E_FAIL);

	// now create the fact chunk, not required for PCM but nice to have
	// this is filled in when the close routine is called
	ckOut1.ckid = mmioFOURCC('f', 'a', 'c', 't');
	ckOut1.cksize = 0;

	if (0 != mmioCreateChunk(g_hmmio, &ckOut1, 0))
		return DXTRACE_ERR(L"mmioCreateChunk", E_FAIL);

	if (mmioWrite(g_hmmio
		, (HPSTR)&dwFactChunk
		, sizeof(dwFactChunk)) != sizeof(dwFactChunk))
		return DXTRACE_ERR(L"mmioWrite", E_FAIL);

	// now ascend out of the fact chunk
	if (0 != mmioAscend(g_hmmio, &ckOut1, 0))
		return DXTRACE_ERR(L"mmioAscend", E_FAIL);

	return S_OK;
}

//****************************************************************************
//*                     writeWaveFile
//*
//* writes data to a open wave file
//****************************************************************************
HRESULT writeWaveFile(UINT nSizeToWrite
	, BYTE* pbSrcData
	, UINT* pnSizeWrote
)
{

	if (g_bIsReadingFromMemory)
		return E_NOTIMPL;
	if (g_hmmio == NULL)
		return CO_E_NOTINITIALIZED;
	if (pnSizeWrote == NULL || pbSrcData == NULL)
		return E_INVALIDARG;

	*pnSizeWrote = 0;

	for (UINT cT = 0; cT < nSizeToWrite; cT++)
	{
		if (g_mmioinfoOut.pchNext == g_mmioinfoOut.pchEndWrite)
		{
			g_mmioinfoOut.dwFlags |= MMIO_DIRTY;
			if (0 != mmioAdvance(g_hmmio, &g_mmioinfoOut, MMIO_WRITE))
				return DXTRACE_ERR(L"mmioAdvance", E_FAIL);
		}

		*((BYTE*)g_mmioinfoOut.pchNext) = *((BYTE*)pbSrcData + cT);
		(BYTE*)g_mmioinfoOut.pchNext++;

		(*pnSizeWrote)++;
	}

	return S_OK;
}
