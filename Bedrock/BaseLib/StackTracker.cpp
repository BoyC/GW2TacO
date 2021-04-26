#include "BaseLib.h"

#ifdef ENABLE_STACKTRACKER_CLASS

TBOOL CStackTracker::DbgInitialized = false;

#include <DbgHelp.h>
#pragma comment(lib,"dbghelp.lib")

CStackTracker::CStackTracker( TS8 Offset )
{
  memset( Stack, 0, sizeof( void* )*STACK_TRACE_DEPTH );
#ifdef _DEBUG
  RtlCaptureStackBackTrace( 4 + Offset, STACK_TRACE_DEPTH, Stack, NULL );
#else
  RtlCaptureStackBackTrace( Offset, STACK_TRACE_DEPTH, Stack, NULL );
#endif
}

CStackTracker::CStackTracker( void *Context, TS8 Offset )
{
  InitializeSym();
  memset( Stack, 0, sizeof( void* )*STACK_TRACE_DEPTH );

  CONTEXT *context = (CONTEXT*)Context;

  DWORD machine_type;
  STACKFRAME frame;
  ZeroMemory( &frame, sizeof( frame ) );
  frame.AddrPC.Mode = AddrModeFlat;
  frame.AddrFrame.Mode = AddrModeFlat;
  frame.AddrStack.Mode = AddrModeFlat;
#ifdef _M_X64  
  frame.AddrPC.Offset = context->Rip;
  frame.AddrFrame.Offset = context->Rbp;
  frame.AddrStack.Offset = context->Rsp;
  machine_type = IMAGE_FILE_MACHINE_AMD64;
#else  
  frame.AddrPC.Offset = context->Eip;
  frame.AddrFrame.Offset = context->Ebp;
  frame.AddrStack.Offset = context->Esp;
  machine_type = IMAGE_FILE_MACHINE_I386;
#endif  

  for ( TS32 i = 0; i < STACK_TRACE_DEPTH + Offset; i++ )
  {
    if ( StackWalk( machine_type,
                    GetCurrentProcess(),
                    GetCurrentThread(),
                    &frame,
                    context,
                    NULL,
                    SymFunctionTableAccess,
                    SymGetModuleBase,
                    NULL ) )
    {
      if ( i >= Offset )
      {
        Stack[ i - Offset ] = (void*)( frame.AddrPC.Offset );
      }
    }
    else
    {
      break;
    }
  }
}

void CStackTracker::DumpToLog( LOGVERBOSITY v )
{
  InitializeSym();

  for ( TS32 x = 0; x < STACK_TRACE_DEPTH; x++ )
  {
    if ( Stack[ x ] )
    {
      DWORD  dwDisplacement;
      IMAGEHLP_LINE line;

      line.SizeOfStruct = sizeof( IMAGEHLP_LINE );

      if ( SymGetLineFromAddr( GetCurrentProcess(), (DWORD)Stack[ x ], &dwDisplacement, &line ) )
      {
        LOGSIMPLE( v, _T( "\t\t%s (%d)" ), CString( line.FileName ).GetPointer(), line.LineNumber );
      }
      else
      {
        LOGSIMPLE( v, _T( "\t\tUnresolved address: %8X" ), Stack[ x ] );
      }
    }
  }
  LOGSIMPLE( v, _T( "" ) );
}

void CStackTracker::DumpToDebugOutput()
{
  InitializeSym();

  for ( TS32 x = 0; x < STACK_TRACE_DEPTH; x++ )
  {
    if ( Stack[ x ] )
    {
      DWORD  dwDisplacement;
      IMAGEHLP_LINE line;

      line.SizeOfStruct = sizeof( IMAGEHLP_LINE );

      if ( SymGetLineFromAddr( GetCurrentProcess(), (DWORD)Stack[ x ], &dwDisplacement, &line ) )
      {
        OutputDebugString( ( CString( "\t\t" ) + line.FileName + " (" + line.LineNumber + ")\n" ).GetPointer() );
      }
      else
      {
        OutputDebugString( ( CString( "\t\tUnresolved address: " ) + CString::Format( _T( "%8X\n" ), Stack[ x ] ) ).GetPointer() );
      }
    }
  }
  OutputDebugString( CString( "\n" ).GetPointer() );
}

TS8 *CStackTracker::DumpToString()
{
  CString s;

  InitializeSym();

  for ( TS32 x = 0; x < STACK_TRACE_DEPTH; x++ )
  {
    if ( Stack[ x ] )
    {
      DWORD  dwDisplacement;
      IMAGEHLP_LINE line;

      line.SizeOfStruct = sizeof( IMAGEHLP_LINE );

      if ( SymGetLineFromAddr( GetCurrentProcess(), (DWORD)Stack[ x ], &dwDisplacement, &line ) )
      {
        s += ( CString( "\t\t" ) + line.FileName + " (" + line.LineNumber + ")\n" );
      }
      else
      {
        s += ( CString( "\t\tUnresolved address: " ) + CString::Format( _T( "%8X\n" ), Stack[ x ] ) );
      }
    }
  }
  s += _T( "\n" );

  TS8 *copy = new TS8[ s.Length() + 1 ];
  memset( copy, 0, s.Length() + 1 );
  s.WriteAsMultiByte( copy, s.Length() + 1 );
  return copy;
}

void CStackTracker::InitializeSym()
{
  if ( !DbgInitialized )
  {
    SymInitialize( GetCurrentProcess(), NULL, true );
    SymSetOptions( SYMOPT_LOAD_LINES );
    DbgInitialized = true;
  }
}


#endif