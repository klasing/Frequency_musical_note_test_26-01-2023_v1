// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

#include <CommCtrl.h>

// an easy way to add the library
#pragma comment(lib, "Winmm.lib")
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string>

#include <objbase.h>
#include <mmeapi.h>
#include <mmreg.h>
#include <mmiscapi.h>
#include <winerror.h>
#include <WinUser.h>
