#pragma once
//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"

//****************************************************************************
//*                     MyAudioSource
//****************************************************************************
class MyAudioSource
{
public:
	//************************************************************************
	//*                 SetFormat
	//************************************************************************
	HRESULT SetFormat(WAVEFORMATEX* pwfx)
	{
		if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
		{
			format = *reinterpret_cast<WAVEFORMATEXTENSIBLE*>(pwfx);
		}

		return S_OK;
	}

	//************************************************************************
	//*                 LoadData
	//************************************************************************
	// the size of an audio frame = nChannels * wBitsPerSample
	HRESULT LoadData(UINT32& numFramesAvailable
		, BYTE* pData
		, DWORD* pFlags
	)
	{
		const UINT16 formatTag = EXTRACT_WAVEFORMATEX_ID(&format.SubFormat);
		if (formatTag == WAVE_FORMAT_IEEE_FLOAT)
		{
			OutputDebugString(L"WAVE_FORMAT_IEEE_FLOAT\n");
			float* fData = (float*)pData;
			for (UINT32 i = 0; i < format.Format.nChannels * numFramesAvailable; i++)
			{
				fData[i] = a[i];
			}
			++cPacket;
			if (cPacket == 2)
			{
				// stop playing
				//*pFlags = AUDCLNT_BUFFERFLAGS_SILENT;
				cPacket == 0;
			}
		}
		else
		{
			*pFlags = AUDCLNT_BUFFERFLAGS_SILENT;
		}

		return S_OK;
	}

	//************************************************************************
	//*                 init
	//************************************************************************
	HRESULT init(const UINT8 octave, const UINT8 freq_note)
	{
		float frequency_hz = oNote.aFreq[octave][freq_note];
		int sample_rate = 48'000;
		float delta = 2.f * frequency_hz * float(M_PI / sample_rate);
		float phase = 0.f;

		for (UINT32 i = 0; i < 2 * 48'000; i++)
		{
			float next_sample = std::sin(phase);
			phase = std::fmod(phase + delta, 2.f * static_cast<float>(M_PI));
			a[i] = next_sample;;
		}

		return S_OK;
	}

private:
	WAVEFORMATEXTENSIBLE format;
	Note oNote;
	std::array<float, 2 * 48'000> a = {};
	UINT8 cPacket = 0;
};