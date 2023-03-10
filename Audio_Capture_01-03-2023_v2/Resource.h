#define BUFFER_MAX					256

// depends on sound settings
// either one of the two is active
// 1) use microphone array
#define MICROPHONE_ARRAY			0
// 2) use stereo mix
#define STEREO_MIX					1
// use Speaker/Headphone
#define SPEAKER_HEADPHONE			0

#define WAVEFILE_READ				1
#define WAVEFILE_WRITE				2

#define DATABLOCK_SIZE				8192 // (bytes)
#define MAX_BUFFERS					2
#define PLAY_MAX_BUFFERS			256

#define IDC_START_AUDIO_CAPTURE		30100
#define IDC_PLAYBACK				30101

//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ generated include file.
// Used by Audio_Capture_01-03-2023_v2.rc

#define IDS_APP_TITLE			103

#define IDR_MAINFRAME			128
#define IDD_AUDIOCAPTURE01032023V2_DIALOG	102
#define IDD_ABOUTBOX			103
#define IDM_ABOUT				104
#define IDM_EXIT				105
#define IDI_AUDIOCAPTURE01032023V2			107
#define IDI_SMALL				108
#define IDC_AUDIOCAPTURE01032023V2			109
#define IDC_MYICON				2
#ifndef IDC_STATIC
#define IDC_STATIC				-1
#endif
// Next default values for new objects
//
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS

#define _APS_NO_MFC					130
#define _APS_NEXT_RESOURCE_VALUE	129
#define _APS_NEXT_COMMAND_VALUE		32771
#define _APS_NEXT_CONTROL_VALUE		1000
#define _APS_NEXT_SYMED_VALUE		110
#endif
#endif
