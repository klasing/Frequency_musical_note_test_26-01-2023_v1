#pragma once
//*****************************************************************************
//*                     include
//*****************************************************************************
#include "framework.h"
#include "audio.h"

//****************************************************************************
//*                     global
//****************************************************************************
// stuff  for synthesiser
constexpr std::array<int, 22> notes = { 88, 86, 78, 78, 80, 80
	, 85, 83, 74, 74, 76, 76
	, 63, 81, 73, 73, 76, 76
	, 81, 81, 81, 81
};
constexpr float bpm = 260.0;
std::atomic<bool> stop = false;

//*****************************************************************************
//*                     prototype
//*****************************************************************************

//*****************************************************************************
//*                     special purpose define
//*****************************************************************************
// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC 10'000'000
#define REFTIMES_PER_MILLISEC 10'000

#define EXIT_ON_ERROR(hres) \
	if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk) \
	if ((punk) != NULL) \
	  { (punk)->Release(); (punk) = NULL; }
//*****************************************************************************
//*                     playAudioStream
//*****************************************************************************
DWORD WINAPI playAudioStream(LPVOID lpVoid)
//BOOL playAudioStream()
{
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioClient = __uuidof(IAudioClient);
	const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

	HRESULT hr;
	REFERENCE_TIME hnsRequestDuration = 10'000'000;
	REFERENCE_TIME hnsActualDuration;
	UINT32 numFramesPadding = 0;
	UINT32 numFramesAvailable = 0;
	UINT32 bufferFrameCount = 0;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* pDevice = NULL;
	IAudioClient* pAudioClient = NULL;
	WAVEFORMATEX* pwfx = NULL;
	IAudioRenderClient* pRenderClient = NULL;
	BYTE* pData;
	DWORD flags = 0;

	UINT8 octave = 3;
	UINT8 freq_note = 9;

	MyAudioSource* pMySource = new MyAudioSource;
	hr = pMySource->init(octave, freq_note);
	EXIT_ON_ERROR(hr);

	hr = CoInitialize(nullptr);
	hr = CoCreateInstance(CLSID_MMDeviceEnumerator
		, NULL
		, CLSCTX_ALL
		, IID_IMMDeviceEnumerator
		, (void**)&pEnumerator
	);
	EXIT_ON_ERROR(hr);

	hr = pEnumerator->GetDefaultAudioEndpoint(eRender
		, eConsole
		, &pDevice
	);
	EXIT_ON_ERROR(hr);

	hr = pDevice->Activate(IID_IAudioClient
		, CLSCTX_ALL
		, NULL
		, (void**)&pAudioClient
	);
	EXIT_ON_ERROR(hr);

	hr = pAudioClient->GetMixFormat(&pwfx);
	EXIT_ON_ERROR(hr);

	hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED
		, 0
		, hnsRequestDuration
		, 0
		, pwfx
		, NULL
	);
	EXIT_ON_ERROR(hr);

	hr = pMySource->SetFormat(pwfx);
	EXIT_ON_ERROR(hr);

	// tell the audio source which format to use
	hr = pAudioClient->GetBufferSize(&bufferFrameCount);
	EXIT_ON_ERROR(hr);

	// get the actual size of the allocated buffer
	hr = pAudioClient->GetService(IID_IAudioRenderClient
		, (void**)&pRenderClient
	);
	EXIT_ON_ERROR(hr);

	// grab the entire buffer for the initial fill operation
	hr = pRenderClient->GetBuffer(bufferFrameCount
		, &pData
	);
	EXIT_ON_ERROR(hr);

	// load the initial data into the shared buffer
	hr = pMySource->LoadData(bufferFrameCount
		, pData
		, &flags
	);
	EXIT_ON_ERROR(hr);

	hr = pRenderClient->ReleaseBuffer(bufferFrameCount
		, flags
	);
	EXIT_ON_ERROR(hr);

	// calculate the actual duration of the allocated buffer
	hnsActualDuration = (double)REFTIMES_PER_SEC
		* bufferFrameCount / pwfx->nSamplesPerSec;

	// start playing
	hr = pAudioClient->Start();
	EXIT_ON_ERROR(hr);

	// each loop fills about half of the shared buffer
	while (flags != AUDCLNT_BUFFERFLAGS_SILENT)
	{
		// sleep for half the buffer duration
		Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));

		// see how much buffer space is available
		hr = pAudioClient->GetCurrentPadding(&numFramesPadding);
		EXIT_ON_ERROR(hr);

		numFramesAvailable = bufferFrameCount - numFramesPadding;

		// grab all the available space in the shared buffer
		hr = pRenderClient->GetBuffer(numFramesAvailable
			, &pData
		);
		EXIT_ON_ERROR(hr);

		// get next 1/2-second of data from the audio source
		hr = pMySource->LoadData(numFramesAvailable
			, pData
			, &flags
		);
		EXIT_ON_ERROR(hr);

		hr = pRenderClient->ReleaseBuffer(numFramesAvailable
			, flags
		);
		EXIT_ON_ERROR(hr);

		// play following note
		hr = pMySource->init(octave, ++freq_note);
	}

	// wait for the last data in buffer to play before stopping
	// time unit in millisecond
	// don't wait too long
	Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 64));

	hr = pAudioClient->Stop();
	EXIT_ON_ERROR(hr);

Exit:
	// cleanup
	CoTaskMemFree(pwfx);
	SAFE_RELEASE(pEnumerator);
	SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pAudioClient);
	SAFE_RELEASE(pRenderClient);

	return 0;
	//return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     note_to_frequency
//*****************************************************************************
float note_to_frequency_hz(int note)
{
	constexpr float pitch_standard_hz = 440.0f;
	return pitch_standard_hz * std::pow(2.0f, float(note - 69) / 12.0f);
}

//*****************************************************************************
//*                     struct
//*****************************************************************************
struct synthesiser
{
	float get_next_sample()
	{
		// assert (_sample_rate > 0);

		_ms_counter += 1000.0f / _sample_rate;
		if (_ms_counter >= _note_duration_ms)
		{
			_ms_counter = 0;
			if (++_current_note_index < notes.size())
			{
				update();
			}
			else
			{
				stop.store(true);
				return 0;
			}
		}

		auto next_sample = std::copysign(0.1f, std::sin(_phase));
		_phase = std::fmod(_phase + _delta, 2.0f * float(M_PI));
		return next_sample;
	}

	void set_sample_rate(float sample_rate)
	{
		_sample_rate = sample_rate;
		update();
	}

private:
	void update() noexcept
	{
		float frequency_hz = note_to_frequency_hz(notes.at(_current_note_index));
		_delta = 2.0f * frequency_hz * static_cast<float>(M_PI / _sample_rate);
	}

	float _sample_rate = 0;
	float _delta = 0;
	float _phase = 0;
	float _ms_counter = 0;
	float _note_duration_ms = 60'000.0f / bpm;
	int _current_note_index = 0;
};

//*****************************************************************************
//*                     playMelody
//*
//* This example app plays a short melody
//* using a simple square wave synthesiser.
//*****************************************************************************
BOOL playMelody()
{
	auto device = get_default_audio_output_device();
	if (!device)
		return 1;

	auto synth = synthesiser();
	synth.set_sample_rate(float(device->get_sample_rate()));

	device->connect([=](audio_device&, audio_device_io<float>& io) mutable noexcept {
		if (!io.output_buffer.has_value())
			return;

		auto& out = *io.output_buffer;

		for (int frame = 0; frame < out.size_frames(); ++frame)
		{
			auto next_sample = synth.get_next_sample();

			for (int channel = 0; channel < out.size_channels(); ++channel)
				out(frame, channel) = next_sample;
		}
	});

	device->start();
	while (!stop.load())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     playSineWave
//*****************************************************************************
BOOL playSineWave()
{
	auto device = get_default_audio_output_device();
	if (!device)
		return 1;

	float frequency_hz = 1000.0f;
	float delta = 2.0f * frequency_hz * float(M_PI / device->get_sample_rate());
	float phase = 0.;

	device->connect([=](audio_device&, audio_device_io<float>& io) mutable noexcept
		{
			if (!io.output_buffer.has_value())
			return;

	auto& out = *io.output_buffer;

	for (int frame = 0; frame < out.size_frames(); ++frame)
	{
		float next_sample = std::sin(phase);
		phase = std::fmod(phase + delta, 2.0f * static_cast<float>(M_PI));

		for (int channel = 0; channel < out.size_channels(); ++channel)
			out(frame, channel) = 0.2f * next_sample;
	}
		});

	device->start();
	std::this_thread::sleep_for(std::chrono::seconds(1));

	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     onWmInitDialog_DlgProc
//*****************************************************************************
BOOL onWmInitDialog_DlgProc(const HINSTANCE& hInst
	, const HWND& hDlg
)
{
	// create thread to continuously transmit and receive
	CreateThread(NULL
		, 0
		, playAudioStream
		, (LPVOID)hDlg
		, 0 // run immediately
		, NULL
	);
	// Microsoft example
	//playAudioStream();
	// Timor Doumler example
	// does not work
	//playMelody();
	//playSineWave();

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
	return (INT_PTR)FALSE;
}
/*
// waste /////////////////////////////////////////////////////////////////////
#pragma once
//*****************************************************************************
//*                     include
//*****************************************************************************
#include "framework.h"

#include <initguid.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>

#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include <cmath>
#include "audio.h"

#include "note.hpp"

#include <random>

//****************************************************************************
//*                     global
//****************************************************************************
Note oNote;

// stuff  for synthesiser
constexpr std::array<int, 22> notes = { 88, 86, 78, 78, 80, 80
	, 85, 83, 74, 74, 76, 76
	, 63, 81, 73, 73, 76, 76
	, 81, 81, 81, 81
};
constexpr float bpm = 260.0;
std::atomic<bool> stop = false;

//*****************************************************************************
//*                     prototype
//*****************************************************************************

//*****************************************************************************
//*                     MyAudioSource
//*****************************************************************************
class MyAudioSource
{
public:
	MyAudioSource() : format(), engine(__rdtsc()), float_dist(-1.f, 1.f) {};

	HRESULT SetFormat(WAVEFORMATEX* pwfx)
	{
		if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
		{
			format = *reinterpret_cast<WAVEFORMATEXTENSIBLE*>(pwfx);
		}
		return S_OK;
	}

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
				//fData[i] = float_dist(engine);
			}

			++cPacket;
			if (cPacket == 2)
			{
				//*pFlags = AUDCLNT_BUFFERFLAGS_SILENT;
				init(2);
			}
		}
		else
		{
			*pFlags = AUDCLNT_BUFFERFLAGS_SILENT;
		}

		return S_OK;
	}
	HRESULT init(const UINT8 select_freq)
	{
		int sample_rate = 48'000;
		float frequency_hz2 = oNote.aFreq[6][0];// 1046.5f;
		//float frequency_hz = 1000.f;
		float frequency_hz1 = oNote.aFreq[5][9];// 880.f;
		float frequency_hz0 = oNote.aFreq[4][9];// 440.f;
		float frequency_hz = 0.f;
		switch (select_freq)
		{
		case 0:
			frequency_hz = oNote.aFreq[4][9];
			break;
		case 1:
			frequency_hz = oNote.aFreq[5][9];
			break;
		case 2:
			frequency_hz = oNote.aFreq[6][0];
			break;
		}
		float delta = 2.f * frequency_hz * float(M_PI / sample_rate);
		float phase = 0.f;
		for (UINT32 i = 0; i < 2 * 48'000; i++)
		{
			float next_sample = std::sin(phase);
			phase = std::fmod(phase + delta, 2.f * static_cast<float>(M_PI));
			a[i] = next_sample;;
		}
		++cPacket = 0;
		OutputDebugString(L"passed: init()\n");
		return S_OK;
	}
private:
	WAVEFORMATEXTENSIBLE format;

	std::mt19937_64 engine;
	std::uniform_real_distribution<float> float_dist;

	std::array<float, 2 * 48'000> a = {};

	UINT8 cPacket = 0;
};

//*****************************************************************************
//*                     special purpose define
//*****************************************************************************
// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC 10'000'000
#define REFTIMES_PER_MILLISEC 10'000

#define EXIT_ON_ERROR(hres) \
	if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk) \
	if ((punk) != NULL) \
	  { (punk)->Release(); (punk) = NULL; }
//*****************************************************************************
//*                     playAudioStream
//*****************************************************************************
BOOL playAudioStream()
{
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioClient = __uuidof(IAudioClient);
	const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

	HRESULT hr;
	REFERENCE_TIME hnsRequestDuration = 10'000'000;
	REFERENCE_TIME hnsActualDuration;
	UINT32 numFramesPadding = 0;
	UINT32 numFramesAvailable = 0;
	UINT32 bufferFrameCount = 0;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* pDevice = NULL;
	IAudioClient* pAudioClient = NULL;
	WAVEFORMATEX* pwfx = NULL;
	IAudioRenderClient* pRenderClient = NULL;
	BYTE* pData;
	DWORD flags = 0;

	MyAudioSource* pMySource = new MyAudioSource;
	hr = pMySource->init(0);
	EXIT_ON_ERROR(hr);

	hr = CoInitialize(nullptr);
	hr = CoCreateInstance(CLSID_MMDeviceEnumerator
		, NULL
		, CLSCTX_ALL
		, IID_IMMDeviceEnumerator
		, (void**)&pEnumerator
	);
	EXIT_ON_ERROR(hr);

	hr = pEnumerator->GetDefaultAudioEndpoint(eRender
		, eConsole
		, &pDevice
	);
	EXIT_ON_ERROR(hr);

	hr = pDevice->Activate(IID_IAudioClient
		, CLSCTX_ALL
		, NULL
		, (void**)&pAudioClient
	);
	EXIT_ON_ERROR(hr);

	hr = pAudioClient->GetMixFormat(&pwfx);
	EXIT_ON_ERROR(hr);

	hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED
		, 0
		, hnsRequestDuration
		, 0
		, pwfx
		, NULL
	);
	EXIT_ON_ERROR(hr);

	hr = pMySource->SetFormat(pwfx);
	EXIT_ON_ERROR(hr);

	// tell the audio source which format to use
	hr = pAudioClient->GetBufferSize(&bufferFrameCount);
	EXIT_ON_ERROR(hr);

	// get the actual size of the allocated buffer
	hr = pAudioClient->GetService(IID_IAudioRenderClient
		, (void**)&pRenderClient
	);
	EXIT_ON_ERROR(hr);

	// grab the entire buffer for the initial fill operation
	hr = pRenderClient->GetBuffer(bufferFrameCount
		, &pData
	);
	EXIT_ON_ERROR(hr);

	// load the initial data into the shared buffer
	hr = pMySource->LoadData(bufferFrameCount
		, pData
		, &flags
	);
	EXIT_ON_ERROR(hr);

	hr = pRenderClient->ReleaseBuffer(bufferFrameCount
		, flags
	);
	EXIT_ON_ERROR(hr);

	// calculate the actual duration of the allocated buffer
	hnsActualDuration = (double)REFTIMES_PER_SEC
		* bufferFrameCount / pwfx->nSamplesPerSec;

	// start playing
	hr = pAudioClient->Start();
	EXIT_ON_ERROR(hr);

	// each loop fills about half of the shared buffer
	while (flags != AUDCLNT_BUFFERFLAGS_SILENT)
	{
		// sleep for half the buffer duration
		Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));

		// see how much buffer space is available
		hr = pAudioClient->GetCurrentPadding(&numFramesPadding);
		EXIT_ON_ERROR(hr);

		numFramesAvailable = bufferFrameCount - numFramesPadding;

		// grab all the available space in the shared buffer
		hr = pRenderClient->GetBuffer(numFramesAvailable
			, &pData
		);
		EXIT_ON_ERROR(hr);

		// get next 1/2-second of data from the audio source
		hr = pMySource->LoadData(numFramesAvailable
			, pData
			, &flags
		);
		EXIT_ON_ERROR(hr);

		hr = pRenderClient->ReleaseBuffer(numFramesAvailable
			, flags
		);
		EXIT_ON_ERROR(hr);
	}

	// wait for the last data in buffer to play before stopping
	// time unit in millisecond
	Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));

	hr = pAudioClient->Stop();
	EXIT_ON_ERROR(hr);

Exit:
	// cleanup
	CoTaskMemFree(pwfx);
	SAFE_RELEASE(pEnumerator);
	SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pAudioClient);
	SAFE_RELEASE(pRenderClient);

	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     note_to_frequency
//*****************************************************************************
float note_to_frequency_hz(int note)
{
	constexpr float pitch_standard_hz = 440.0f;
	return pitch_standard_hz * std::pow(2.0f, float(note - 69) / 12.0f);
}

//*****************************************************************************
//*                     struct
//*****************************************************************************
struct synthesiser
{
	float get_next_sample()
	{
		// assert (_sample_rate > 0);

		_ms_counter += 1000.0f / _sample_rate;
		if (_ms_counter >= _note_duration_ms)
		{
			_ms_counter = 0;
			if (++_current_note_index < notes.size())
			{
				update();
			}
			else
			{
				stop.store(true);
				return 0;
			}
		}

		auto next_sample = std::copysign(0.1f, std::sin(_phase));
		_phase = std::fmod(_phase + _delta, 2.0f * float(M_PI));
		return next_sample;
	}

	void set_sample_rate(float sample_rate)
	{
		_sample_rate = sample_rate;
		update();
	}

private:
	void update() noexcept
	{
		float frequency_hz = note_to_frequency_hz(notes.at(_current_note_index));
		_delta = 2.0f * frequency_hz * static_cast<float>(M_PI / _sample_rate);
	}

	float _sample_rate = 0;
	float _delta = 0;
	float _phase = 0;
	float _ms_counter = 0;
	float _note_duration_ms = 60'000.0f / bpm;
	int _current_note_index = 0;
};

//*****************************************************************************
//*                     playMelody
//*
//* This example app plays a short melody
//* using a simple square wave synthesiser.
//*****************************************************************************
BOOL playMelody()
{
	auto device = get_default_audio_output_device();
	if (!device)
		return 1;

	auto synth = synthesiser();
	synth.set_sample_rate(float(device->get_sample_rate()));

	device->connect([=](audio_device&, audio_device_io<float>& io) mutable noexcept
		{
			if (!io.output_buffer.has_value())
				return;
			
			auto& out = *io.output_buffer;

			for (int frame = 0; frame < out.size_frames(); ++frame)
			{
				auto next_sample = synth.get_next_sample();

				for (int channel = 0; channel < out.size_channels(); ++channel)
					out(frame, channel) = next_sample;
			}
		});

	device->start();
	while (!stop.load())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     playSineWave
//*****************************************************************************
BOOL playSineWave()
{
	auto device = get_default_audio_output_device();
	if (!device)
		return 1;

	//float frequency_hz = 440.0f;
	float frequency_hz = 1000.0f;
	float delta = 2.0f * frequency_hz * float(M_PI / device->get_sample_rate());
	float phase = 0.;

	device->connect([=](audio_device&, audio_device_io<float>& io) mutable noexcept
		{
			if (!io.output_buffer.has_value())
			return;

	auto& out = *io.output_buffer;

	for (int frame = 0; frame < out.size_frames(); ++frame)
	{
		float next_sample = std::sin(phase);
		phase = std::fmod(phase + delta, 2.0f * static_cast<float>(M_PI));

		for (int channel = 0; channel < out.size_channels(); ++channel)
			out(frame, channel) = 0.2f * next_sample;
	}
		});

	device->start();
	std::this_thread::sleep_for(std::chrono::seconds(1));

	return EXIT_SUCCESS;
}

//*****************************************************************************
//*                     onWmInitDialog_DlgProc
//*****************************************************************************
BOOL onWmInitDialog_DlgProc(const HINSTANCE& hInst
	, const HWND& hDlg
)
{
	oNote.init();
	// Microsoft example
	playAudioStream();
	// Timor Doumler example
	// does not work
	//playMelody();
	//playSineWave();


	return EXIT_SUCCESS;
}
*/