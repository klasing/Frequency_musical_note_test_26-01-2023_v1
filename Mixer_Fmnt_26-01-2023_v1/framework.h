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

#include <cstdio>
#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <random>

// windows multimedia device
#include <mmreg.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

// wasapi
#include <initguid.h>
#include <audiopolicy.h>
#include <Audioclient.h>
