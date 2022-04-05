#pragma once
#include <windows.h>

#define  TacO_ALT   0x100000
#define  TacO_CTRL  0x200000
#define  TacO_SHIFT 0x400000

void InitInputHooks();
void ShutDownInputHooks();

int GetTacOKeyCode( int key );
CString GetTacOKeyString( int key );

