// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;

// an easy way to add the library
#pragma comment(lib, "Winmm.lib")
#pragma comment (lib,"Gdiplus.lib")
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <objbase.h>
#include <mmeapi.h>
#include <mmreg.h>
#include <mmiscapi.h>

#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <memory>
#include <vector>
#include <random>



