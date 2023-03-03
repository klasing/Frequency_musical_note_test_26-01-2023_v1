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

#define SAFE_DELETE_ARRAY(p)	{ if (p) { delete[](p); (p)=NULL; } }
#define DXTRACE_ERR(str,hr)		(hr)

//****************************************************************************
//*                     global
//****************************************************************************
//MMRESULT rc = MMSYSERR_NOERROR;
HMMIO g_hmmio = NULL;
MMCKINFO g_ck;
MMCKINFO g_ckRiff;
MMIOINFO g_mmioinfoOut;
DWORD g_dwFlags;
BOOL g_bIsReadingFromMemory;
BYTE* g_pbData;
BYTE* g_pbDataCur;
CHAR* g_pResourceBuffer;

//****************************************************************************
//*                     WriteMMIO
//****************************************************************************
HRESULT WriteMMIO(WAVEFORMATEX* pwfxDest)
{
	// cntains the actual fact chunk. Garbage until WaveCloseWriteFile
	DWORD dwFactChunk; 
	MMCKINFO ckOut1;

	memset( &ckOut1, 0, sizeof(ckOut1) );

	dwFactChunk = ( DWORD )-1;

	// create the output file RIFF chunk of form type 'WAVE'
	g_ckRiff.fccType = mmioFOURCC( 'W', 'A', 'V', 'E' );
	g_ckRiff.cksize = 0;

	if( 0 != mmioCreateChunk(g_hmmio, &g_ckRiff, MMIO_CREATERIFF))
		return DXTRACE_ERR( L"mmioCreateChunk", E_FAIL );

	// we are now descended into the 'RIFF' chunk we just created
	// now create the 'fmt ' chunk
	// since we know the size of this chunk,
	// specify it in the MMCKINFO structure so MMIO doesn't have to seek
	// back and set the chunk size after ascending from the chunk
	g_ck.ckid = mmioFOURCC( 'f', 'm', 't', ' ' );
	g_ck.cksize = sizeof( PCMWAVEFORMAT );

	if( 0 != mmioCreateChunk(g_hmmio, &g_ck, 0))
		return DXTRACE_ERR( L"mmioCreateChunk", E_FAIL );

	// write the PCMWAVEFORMAT structure to the 'fmt ' chunk if its that type
	if( pwfxDest->wFormatTag == WAVE_FORMAT_PCM )
	{
		if( mmioWrite(g_hmmio
			, (HPSTR)pwfxDest
			, sizeof(PCMWAVEFORMAT)) != sizeof(PCMWAVEFORMAT))
			return DXTRACE_ERR( L"mmioWrite", E_FAIL );
	}
	else
	{
		// write the variable length size
		if ((UINT)mmioWrite(g_hmmio
			, (HPSTR)pwfxDest
			, sizeof(*pwfxDest) + pwfxDest->cbSize) !=
			(sizeof(*pwfxDest) + pwfxDest->cbSize))
			return DXTRACE_ERR( L"mmioWrite", E_FAIL );
	}

	// ascend out of the 'fmt ' chunk, back into the 'RIFF' chunk
	if (0 != mmioAscend(g_hmmio, &g_ck, 0))
		return DXTRACE_ERR( L"mmioAscend", E_FAIL );

	// now create the fact chunk, not required for PCM but nice to have
	// this is filled in when the close routine is called
	ckOut1.ckid = mmioFOURCC( 'f', 'a', 'c', 't' );
	ckOut1.cksize = 0;

	if (0 != mmioCreateChunk(g_hmmio, &ckOut1, 0))
		return DXTRACE_ERR( L"mmioCreateChunk", E_FAIL );

	if (mmioWrite(g_hmmio
		, (HPSTR)&dwFactChunk
		, sizeof(dwFactChunk)) != sizeof(dwFactChunk))
		return DXTRACE_ERR( L"mmioWrite", E_FAIL );

	// now ascend out of the fact chunk
	if (0 != mmioAscend(g_hmmio, &ckOut1, 0))
		return DXTRACE_ERR( L"mmioAscend", E_FAIL );

	return S_OK;
}

//****************************************************************************
//*                     ResetFile
//****************************************************************************
HRESULT ResetFile()
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
//*                     openWaveFile
//****************************************************************************
HRESULT openWaveFile(LPWSTR strFileName
    , WAVEFORMATEX* pwfx
    , DWORD dwFlags
)
{
    HRESULT hr;

    g_dwFlags = dwFlags;
    g_bIsReadingFromMemory = FALSE;

    if (g_dwFlags == WAVEFILE_READ)
    {
    }
    else
    {
        g_hmmio = mmioOpen(strFileName
            , NULL
            , MMIO_ALLOCBUF | MMIO_READWRITE | MMIO_CREATE
        );

        if (NULL == g_hmmio)
            return DXTRACE_ERR(L"mmioOpen", E_FAIL);

        if (FAILED(hr = WriteMMIO(pwfx)))
        {
            mmioClose(g_hmmio, 0);
            return DXTRACE_ERR(L"WriteMMIO", hr);
        }

        if (FAILED(hr = ResetFile()))
            return DXTRACE_ERR(L"ResetFile", hr);
    }

    return hr;
}

//****************************************************************************
//*                     writeWaveFile
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


//****************************************************************************
//*                     readWaveFile
//****************************************************************************

//****************************************************************************
//*                     closeWaveFile
//****************************************************************************
HRESULT closeWaveFile()
{
	if (g_dwFlags == WAVEFILE_READ)
	{
		if (g_hmmio != NULL) mmioClose(g_hmmio, 0);
		g_hmmio = NULL;
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
