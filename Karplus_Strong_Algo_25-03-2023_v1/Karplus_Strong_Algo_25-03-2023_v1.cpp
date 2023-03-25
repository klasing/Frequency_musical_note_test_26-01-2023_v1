// Karplus_Strong_Algo_25-03-2023_v1.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Karplus_Strong_Algo_25-03-2023_v1.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_KARPLUSSTRONGALGO25032023V1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_KARPLUSSTRONGALGO25032023V1));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KARPLUSSTRONGALGO25032023V1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_KARPLUSSTRONGALGO25032023V1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

/*
Google: Princeton COS 126: Plucking a Guitar String
// waste /////////////////////////////////////////////////////////////////////
https://blog.demofox.org/2016/06/16/synthesizing-a-pluked-string-sound-with-the-karplus-strong-algorithm/
#include <stdio.h>
#include <memory.h>
#include <inttypes.h>
#include <vector>

// constants
const float c_pi = 3.14159265359f;
const float c_twoPi = 2.0f * c_pi;

// typedefs
typedef uint16_t    uint16;
typedef uint32_t    uint32;
typedef int16_t     int16;
typedef int32_t     int32;

//this struct is the minimal required header data for a wav file
struct SMinimalWaveFileHeader
{
    //the main chunk
    unsigned char m_chunkID[4];
    uint32        m_chunkSize;
    unsigned char m_format[4];

    //sub chunk 1 "fmt "
    unsigned char m_subChunk1ID[4];
    uint32        m_subChunk1Size;
    uint16        m_audioFormat;
    uint16        m_numChannels;
    uint32        m_sampleRate;
    uint32        m_byteRate;
    uint16        m_blockAlign;
    uint16        m_bitsPerSample;

    //sub chunk 2 "data"
    unsigned char m_subChunk2ID[4];
    uint32        m_subChunk2Size;

    //then comes the data!
};

//this writes
template <typename T>
bool WriteWaveFile(const char* fileName, std::vector<T> data, int16 numChannels, int32 sampleRate)
{
    int32 dataSize = data.size() * sizeof(T);
    int32 bitsPerSample = sizeof(T) * 8;

    //open the file if we can
    FILE* File = nullptr;
    fopen_s(&File, fileName, "w+b");
    if (!File)
        return false;

    SMinimalWaveFileHeader waveHeader;

    //fill out the main chunk
    memcpy(waveHeader.m_chunkID, "RIFF", 4);
    waveHeader.m_chunkSize = dataSize + 36;
    memcpy(waveHeader.m_format, "WAVE", 4);

    //fill out sub chunk 1 "fmt "
    memcpy(waveHeader.m_subChunk1ID, "fmt ", 4);
    waveHeader.m_subChunk1Size = 16;
    waveHeader.m_audioFormat = 1;
    waveHeader.m_numChannels = numChannels;
    waveHeader.m_sampleRate = sampleRate;
    waveHeader.m_byteRate = sampleRate * numChannels * bitsPerSample / 8;
    waveHeader.m_blockAlign = numChannels * bitsPerSample / 8;
    waveHeader.m_bitsPerSample = bitsPerSample;

    //fill out sub chunk 2 "data"
    memcpy(waveHeader.m_subChunk2ID, "data", 4);
    waveHeader.m_subChunk2Size = dataSize;

    //write the header
    fwrite(&waveHeader, sizeof(SMinimalWaveFileHeader), 1, File);

    //write the wave data itself
    fwrite(&data[0], dataSize, 1, File);

    //close the file and return success
    fclose(File);
    return true;
}

template <typename T>
void ConvertFloatSamples(const std::vector<float>& in, std::vector<T>& out)
{
    // make our out samples the right size
    out.resize(in.size());

    // convert in format to out format !
    for (size_t i = 0, c = in.size(); i < c; ++i)
    {
        float v = in[i];
        if (v < 0.0f)
            v *= -float(std::numeric_limits<T>::lowest());
        else
            v *= float(std::numeric_limits<T>::max());
        out[i] = T(v);
    }
}
//calculate the frequency of the specified note.
//fractional notes allowed!
float CalcFrequency(float octave, float note)

    //Calculate the frequency of any note!
    //frequency = 440×(2^(n/12))

    //N=0 is A4
    //N=1 is A#4
    //etc...

    //notes go like so...
    //0  = A
    //1  = A#
    //2  = B
    //3  = C
    //4  = C#
    //5  = D
    //6  = D#
    //7  = E
    //8  = F
    //9  = F#
    //10 = G
    //11 = G#

{
    return (float)(440 * pow(2.0, ((double)((octave - 4) * 12 + note)) / 12.0));
}

class CKarplusStrongStringPluck
{
public:
    CKarplusStrongStringPluck(float frequency, float sampleRate, float feedback)
    {
        m_buffer.resize(uint32(float(sampleRate) / frequency));
        for (size_t i = 0, c = m_buffer.size(); i < c; ++i) {
            m_buffer[i] = ((float)rand()) / ((float)RAND_MAX) * 2.0f - 1.0f;  // noise
            //m_buffer[i] = float(i) / float(c); // saw wave
        }
        m_index = 0;
        m_feedback = feedback;
    }

    float GenerateSample()
    {
        // get our sample to return
        float ret = m_buffer[m_index];

        // low pass filter (average) some samples
        float value = (m_buffer[m_index] + m_buffer[(m_index + 1) % m_buffer.size()]) * 0.5f * m_feedback;
        m_buffer[m_index] = value;

        // move to the next sample
        m_index = (m_index + 1) % m_buffer.size();

        // return the sample from the buffer
        return ret;
    }

private:
    std::vector<float>  m_buffer;
    size_t              m_index;
    float               m_feedback;
};

void GenerateSamples(std::vector<float>& samples, int sampleRate)
{
    std::vector<CKarplusStrongStringPluck> notes;

    enum ESongMode {
        e_twinkleTwinkle,
        e_strum
    };

    int timeBegin = 0;
    ESongMode mode = e_twinkleTwinkle;
    for (int index = 0, numSamples = samples.size(); index < numSamples; ++index)
    {
        switch (mode) {
        case e_twinkleTwinkle: {
            const int c_noteTime = sampleRate / 2;
            int time = index - timeBegin;
            // if we should start a new note
            if (time % c_noteTime == 0) {
                int note = time / c_noteTime;
                switch (note) {
                case 0:
                case 1: {
                    notes.push_back(CKarplusStrongStringPluck(CalcFrequency(3, 0), float(sampleRate), 0.996f));
                    break;
                }
                case 2:
                case 3: {
                    notes.push_back(CKarplusStrongStringPluck(CalcFrequency(3, 7), float(sampleRate), 0.996f));
                    break;
                }
                case 4:
                case 5: {
                    notes.push_back(CKarplusStrongStringPluck(CalcFrequency(3, 9), float(sampleRate), 0.996f));
                    break;
                }
                case 6: {
                    notes.push_back(CKarplusStrongStringPluck(CalcFrequency(3, 7), float(sampleRate), 0.996f));
                    break;
                }
                case 7: {
                    mode = e_strum;
                    timeBegin = index + 1;
                    break;
                }
                }
            }
            break;
        }
        case e_strum: {
            const int c_noteTime = sampleRate / 32;
            int time = index - timeBegin - sampleRate;
            // if we should start a new note
            if (time % c_noteTime == 0) {
                int note = time / c_noteTime;
                switch (note) {
                case 0: notes.push_back(CKarplusStrongStringPluck(55.0f, float(sampleRate), 0.996f)); break;
                case 1: notes.push_back(CKarplusStrongStringPluck(55.0f + 110.0f, float(sampleRate), 0.996f)); break;
                case 2: notes.push_back(CKarplusStrongStringPluck(55.0f + 220.0f, float(sampleRate), 0.996f)); break;
                case 3: notes.push_back(CKarplusStrongStringPluck(55.0f + 330.0f, float(sampleRate), 0.996f)); break;
                case 4: mode = e_strum; timeBegin = index + 1; break;
                }
            }
            break;
        }
        }

        // generate and mix our samples from our notes
        samples[index] = 0;
        for (CKarplusStrongStringPluck& note : notes)
            samples[index] += note.GenerateSample();

        // to keep from clipping
        samples[index] *= 0.5f;
    }
}

//the entry point of our application
int main(int argc, char** argv)
{
    // sound format parameters
    const int c_sampleRate = 44100;
    const int c_numSeconds = 9;
    const int c_numChannels = 1;
    const int c_numSamples = c_sampleRate * c_numChannels * c_numSeconds;

    // make space for our samples
    std::vector<float> samples;
    samples.resize(c_numSamples);

    // generate samples
    GenerateSamples(samples, c_sampleRate);

    // convert from float to the final format
    std::vector<int32> samplesInt;
    ConvertFloatSamples(samples, samplesInt);

    // write our samples to a wave file
    WriteWaveFile("out.wav", samplesInt, c_numChannels, c_sampleRate);
}
*/