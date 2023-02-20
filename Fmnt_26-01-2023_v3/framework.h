// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

#include <CommCtrl.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <string>
#include <array>
#include <memory>
#include <random>

// windows multimedia device
#include <mmreg.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

// wasapi
#include <initguid.h>
#include <audiopolicy.h>
#include <audioclient.h>

/*
* header files for the Core Audio APIs
* audioclient.h
* audioendpoints.h
* audioenginebaseapo.h
* audioengineendpoint.h
* audiopolicy.h
* audiosessiontypes.h
* audiotopology.h
* endpointvolume.h
* mmdeviceapi.h
* spatialaudioclient.h
* spatialaudiohrtf.h
* spatialaudiometadata.h
*/
