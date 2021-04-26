#include "Memory.h"
#include <stdlib.h>
#include "BaseConfig.h"

#ifdef MEMORY_TRACKING

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "String.h"

#include <new>
#undef new

//this should force the memTracker variable to be constructed before everything else:
#pragma warning(disable:4074)
#pragma init_seg(compiler)
CMemTracker memTracker;

TU32 TotalLeaked = 0;
TU32 TotalAllocCount = 0;
TU32 CurrentAllocCount = 0;

CMemTracker::CMemTracker()
{
  InitializeLightweightCS( &critsec );
  Paused = true;
  MemTrackerPool = new CDictionary<void*, CAllocationInfo>();
  Paused = false;
  IgnoreMissing = false;
}

void DumpMemleakEntry( CAllocationInfo &e )
{
  OutputDebugString( ( CString( "Leak:\t" ) + e.Size + " bytes\n" ).GetPointer() );

#ifndef ENABLE_MALLOC_STACK_TRACE
  OutputDebugString( ( CString( "\t\t" ) + e.File + " (" + e.Line + ")\n" ).GetPointer() );
#else
  e.Stack.DumpToDebugOutput();
#endif
  TotalLeaked += e.Size;
}

CMemTracker::~CMemTracker()
{
  Paused = true;

  if ( MemTrackerPool->NumItems() )
  {
    //report leaks
    OutputDebugString( CString( "\n---Memleaks start here---\n\n" ).GetPointer() );
    MemTrackerPool->ForEach( DumpMemleakEntry );
    OutputDebugString( ( CString( "\tTotal bytes leaked: " ) + TotalLeaked + "\n\n" ).GetPointer() );
  }
  else
  {
    OutputDebugString( _T( "**********************************************************\n\t\t\t\t\tNo memleaks found.\n**********************************************************\n\n" ) );
  }

  CDictionary<void*, CAllocationInfo> *temp = MemTrackerPool;
  MemTrackerPool = NULL;
  SAFEDELETE( temp );
}

void CMemTracker::AddPointer( void *p, const TS8* file, TS32 line, TS32 size )
{
  CLightweightCriticalSection cs( &critsec );
  if ( MemTrackerPool && !Paused && p )
  {
    Paused = true;
    CAllocationInfo &allocinfo = CAllocationInfo( (TS8*)file, line, size );
    MemTrackerPool->Add( p, allocinfo );
    Paused = false;
    CurrentAllocCount++;
    TotalAllocCount++;
  }
}

void CMemTracker::RemovePointer( void *p )
{
  CLightweightCriticalSection cs( &critsec );
  if ( !Paused && p )
  {
    CurrentAllocCount--;

    Paused = true;
    if ( MemTrackerPool )
    {
      if ( MemTrackerPool->HasKey( p ) )
        MemTrackerPool->Delete( p );
      else
        if ( !IgnoreMissing )
        {
//					OutputDebugString(_T("[mem] Trying to delete non logged, possibly already freed memory block!\n"));
//#ifdef _DEBUG
//					CStackTracker s(-4);
//#else
//					CStackTracker s;
//#endif
//					s.DumpToDebugOutput();
        }
    }

    Paused = false;
  }
}

TS32 CMemTracker::GetAllocatedMemorySize()
{
  Pause();

  TS32 mem = 0;
  for ( TS32 x = 0; x < MemTrackerPool->NumItems(); x++ )
    mem += MemTrackerPool->GetByIndex( x ).Size;

  Resume();
  return mem;
}

//#pragma push_macro("new")
//#undef new

void* __cdecl operator new( size_t size, const TS8* file, TS32 line )
{
  void *p = malloc( size );
  memTracker.AddPointer( p, file, line, size );
  return p;
}

void* __cdecl operator new[]( size_t size, const TS8* file, TS32 line )
{
  void *p = malloc( size );
  memTracker.AddPointer( p, file, line, size );
  return p;
}

//#pragma pop_macro("new")

void __cdecl operator delete( void* pointer )
{
  memTracker.RemovePointer( pointer );
  free( pointer );
}

void __cdecl operator delete[]( void* pointer )
{
  memTracker.RemovePointer( pointer );
  free( pointer );
}

void __cdecl operator delete( void* pointer, const TS8* file, TS32 line )
{
  memTracker.RemovePointer( pointer );
  free( pointer );
}

void __cdecl operator delete[]( void* pointer, const TS8* file, TS32 line )
{
  memTracker.RemovePointer( pointer );
  free( pointer );
}

#endif