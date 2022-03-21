#include "BaseLib.h"

CLogger Logger;

void CLoggerOutput::Process( LOGVERBOSITY v, TCHAR *String )
{

}

CLoggerOutput::~CLoggerOutput()
{

}

CLoggerOutput::CLoggerOutput()
{

}

void CLoggerOutput_DebugOutput::Process( LOGVERBOSITY v, TCHAR *String )
{
  OutputDebugString( String );
  OutputDebugString( _T( "\n" ) );
}

CLoggerOutput_DebugOutput::~CLoggerOutput_DebugOutput()
{

}

CLoggerOutput_DebugOutput::CLoggerOutput_DebugOutput()
{

}

void CLoggerOutput_StdOut::Process( LOGVERBOSITY v, TCHAR *String )
{
  _tprintf( _T( "%s\n" ), String );
}

CLoggerOutput_StdOut::~CLoggerOutput_StdOut()
{

}

CLoggerOutput_StdOut::CLoggerOutput_StdOut()
{

}

void CLoggerOutput_File::Process( LOGVERBOSITY v, TCHAR *String )
{
  if ( !f )
  {
    if ( !OpenLogFile( fname.GetPointer(), Append ) ) return;
    if ( !f ) return;
  }
#ifndef UNICODE
  fprintf( f, "%s\n", String );
#else
  fwprintf( f, _T( "%s\n" ), String );
#endif
}

TBOOL CLoggerOutput_File::OpenLogFile( TCHAR *Filename, TBOOL Append /*= true*/ )
{
#ifndef UNICODE
  if ( !Append )
    return ( !fopen_s( &f, Filename, "wt" ) );
  else
    return ( !fopen_s( &f, Filename, "at" ) );
#else
  CString s = Filename;
  char Fname[ 2048 ];
  s.WriteAsMultiByte( Fname, 2048 );
  if ( !Append )
    return ( !fopen_s( &f, Fname, "wt, ccs= UTF-8 " ) );
  else
    return ( !fopen_s( &f, Fname, "at, ccs= UTF-8 " ) );
#endif
}

CLoggerOutput_File::~CLoggerOutput_File()
{
  if ( f )
    fclose( f );
  f = 0;
}

CLoggerOutput_File::CLoggerOutput_File( TCHAR *Filename, TBOOL append /*= true*/ )
{
  Append = append;
  fname = Filename;
  OpenLogFile( Filename, Append );
}

CLoggerOutput_File::CLoggerOutput_File()
{
  f = 0;
  Append = true;
  fname = _T( "log.log" );
}

TS32 CLogger::GetNewEntryCount()
{
  return NewEntryCount;
}

void CLogger::ResetEntryCounter()
{
  NewEntryCount = 0;
}

void CLogger::RemoveOutput( CLoggerOutput *Output )
{
  Outputs.Delete( Output );
}

void CLogger::AddOutput( CLoggerOutput *Output )
{
  Outputs.Add( Output );
}

void CLogger::SetVerbosity( LOGVERBOSITY v )
{
  Verbosity = v;
}

void CLogger::Log( LOGVERBOSITY v, TBOOL Prefix, TBOOL AddTimeStamp, TCHAR *String, ... )
{
  if ( Verbosity < v ) return;

  CString LogString;

  va_list argList;
  va_start( argList, String );
  CString::FormatVA( String, argList, LogString );
  va_end( argList );

  time_t rawtime;
  tm timeinfo;

  time( &rawtime );
  localtime_s( &timeinfo, &rawtime );

  CString TimeStamp = CString::Format( _T( "[%.4d-%.2d-%.2d %.2d:%.2d:%.2d] " ), timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec );

  if ( Prefix )
  {
    if ( v < LOG_WARNING ) LogString = _T( "(Err)  " ) + LogString; else
      if ( v < LOG_INFO ) LogString = _T( "(Warn) " ) + LogString; else
        if ( v < LOG_DEBUG ) LogString = _T( "(Info) " ) + LogString; else
          LogString = _T( "(Dbg)  " ) + LogString;
  }

  if ( AddTimeStamp )
    LogString = TimeStamp + LogString;

  for ( TS32 x = 0; x < Outputs.NumItems(); x++ )
    Outputs[ x ]->Process( v, LogString.GetPointer() );

  NewEntryCount++;
}

void CLogger::Close()
{
  Outputs.FreeArray();
}

CLogger::~CLogger()
{
  Outputs.FreeArray();
}

CLogger::CLogger()
{
  Verbosity = LOGGER_BASE_OUTPUT_VERBOSITY;
#ifdef LOG_TO_DEBUGOUTPUT
  Outputs.Add( new CLoggerOutput_DebugOutput() );
#endif
}

CLoggerOutput_RingBuffer::CLoggerOutput_RingBuffer()
{
}

CLoggerOutput_RingBuffer::~CLoggerOutput_RingBuffer()
{

}

void CLoggerOutput_RingBuffer::Process( LOGVERBOSITY v, TCHAR *String )
{
  Buffer.Add( String );
}

void CLoggerOutput_RingBuffer::Dump( CString fname )
{
  FILE* f = nullptr;
  fopen_s(&f, fname.GetPointer(), "w+t");
  for ( TS32 x = 0; x < Buffer.NumItems(); x++ )
    fprintf( f, "%s\n", Buffer[ x ].GetPointer() );
  fclose( f );
}

CString CLoggerOutput_RingBuffer::Dump()
{
  CString result;

  for ( TS32 x = 0; x < Buffer.NumItems(); x++ )
    result += CString::Format("%s\n", Buffer[x].GetPointer());

  return result;
}
