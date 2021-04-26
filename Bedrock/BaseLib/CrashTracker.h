#pragma once

LONG WINAPI baseCrashTracker( struct _EXCEPTION_POINTERS * excpInfo );
LONG WINAPI FullDumpCrashTracker( struct _EXCEPTION_POINTERS * excpInfo );
void InitializeCrashTracker( CString &Build, LPTOP_LEVEL_EXCEPTION_FILTER Tracker = baseCrashTracker );
