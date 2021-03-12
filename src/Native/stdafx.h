// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

// warning C4820: 'type' : 'n' bytes padding added after data member 'type::var'
#pragma warning(disable:4820)
// warning C4668: 'name' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
#pragma warning(disable:4668)

#include "targetver.h"

// Windows Header Files:
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <ShellAPI.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <strsafe.h>
