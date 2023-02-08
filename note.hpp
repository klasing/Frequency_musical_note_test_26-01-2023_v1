#pragma once
//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"
#include "Resource.h"

//****************************************************************************
//*                     Note
//****************************************************************************
class Note
{
public:
	//************************************************************************
	//*                 <<constructor>>
	//************************************************************************
	Note()
	{
		init();
	}

	FLOAT aFreq[NOF_ALL_OCTAVE][NOF_NOTE_PER_OCTAVE] = { 0 };
	FLOAT aWaveLength[NOF_ALL_OCTAVE][NOF_NOTE_PER_OCTAVE] = { 0 };

private:
	//************************************************************************
	//*                 init
	//************************************************************************
	BOOL init()
	{
		for (BYTE x = 0; x < NOF_ALL_OCTAVE; x++)
		{
			// calculate frequency (tone) for note A
			// in all octave
			aFreq[x][9] = A4 / (16 / (pow(2, x)));
		}

		for (BYTE x = 0; x < NOF_ALL_OCTAVE; x++)
		{
			for (BYTE y = 0; y < NOF_NOTE_PER_OCTAVE; y++)
			{
				// calculate frequency (tone) for all note
				// in all octave
				if (y != 9)
				{
					aFreq[x][y] = aFreq[x][9]
						* pow(2, (y - 9) / 12.);
				}

				// not necessary for this application
				// store as wavelength in microsecond
				aWaveLength[x][y] = (1 / aFreq[x][y])
					* 1e6;
			}
		}

		return TRUE;
	}

	FLOAT A4 = 440.; // Hz
};