// Copyright (c) 2013 FastFieldSolvers S.r.l.
// http://www.fastfieldsolvers.com
// All Rights reserved
//
// Usage is subject to the License that you should have received
// as a text file together with these source files. The License text is also available
// in the relevant source code distribution from http://www.fastfieldsolvers.com
// or contacting FastFieldSolvers S.r.l.


// CPP header file for module containing functions linking
// FastCap with Windows I/O window


#ifndef FASTCAPWINDOW_H
#define FASTCAPWINDOW_H

#include <winuser.h>

#include "FCStructs.h"

#define MAX_TITLE_LENGHT 64
#define MAX_OUTPUT_TEXT_LEN 2048

// exported variables & functions

extern "C" volatile char bFCContinue;
extern volatile BOOL g_bIsFCRunning;
extern HWND FCHwnd;
extern char g_sTitle[MAX_TITLE_LENGHT];
extern char g_sOutputText[MAX_OUTPUT_TEXT_LEN];

extern "C" void FCSetName(char *name);
extern "C" void FCOnClosing(int cause);
extern "C" void FCExit(int cause);
extern "C" int viewprintf(FILE *out, const char *fmt,...);

void OnFastCapExit(int cause);

#endif

