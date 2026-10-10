#pragma once
#include <windows.h>

#define  TacO_ALT   0x100000
#define  TacO_CTRL  0x200000
#define  TacO_SHIFT 0x400000
#define  TacO_VKEY  0x010000 // numpad and function keys are stored as virtual key codes, not characters

#define  WM_TACO_HOTKEY ( WM_APP + 1 ) // posted by the keyboard hook, handled as WM_CHAR on the main thread

void InitInputHooks();
void ShutDownInputHooks();

int GetTacOKeyCode( int key );
CString GetTacOKeyString( int key );

