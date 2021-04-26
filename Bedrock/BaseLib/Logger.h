#pragma once
#include <time.h>

#define ENABLE_LOGGING

//base verbosity for the log:
#define LOGGER_BASE_OUTPUT_VERBOSITY 0x400

//logged items with a verbosity above this value are killed compile time:
#define LOGGER_DROPPED_OUTPUT_VERBOSITY 0x200

#define LOG_TO_DEBUGOUTPUT
//#define LOG_TO_STDOUT

enum LOGVERBOSITY
{
  LOG_ERROR = 0x100,
  LOG_WARNING = 0x200,
  LOG_INFO = 0x300,
  LOG_DEBUG = 0x400,
};

class CLoggerOutput
{
public:

  CLoggerOutput();
  virtual ~CLoggerOutput();
  virtual void Process( LOGVERBOSITY v, TCHAR *String );
};

class CLoggerOutput_DebugOutput : public CLoggerOutput
{
public:

  CLoggerOutput_DebugOutput();
  virtual ~CLoggerOutput_DebugOutput();
  virtual void Process( LOGVERBOSITY v, TCHAR *String );
};

class CLoggerOutput_StdOut : public CLoggerOutput
{
public:

  CLoggerOutput_StdOut();
  virtual ~CLoggerOutput_StdOut();
  virtual void Process( LOGVERBOSITY v, TCHAR *String );
};

class CLoggerOutput_File : public CLoggerOutput
{
  FILE *f;
  CString fname;
  TBOOL Append;
public:

  CLoggerOutput_File();
  CLoggerOutput_File( TCHAR *Filename, TBOOL append = true );
  virtual ~CLoggerOutput_File();
  TBOOL OpenLogFile( TCHAR *Filename, TBOOL Append = true );
  virtual void Process( LOGVERBOSITY v, TCHAR *String );
};

class CLoggerOutput_RingBuffer : public CLoggerOutput
{
public:

  CRingBuffer<CString> Buffer;

  CLoggerOutput_RingBuffer();
  virtual ~CLoggerOutput_RingBuffer();
  virtual void Process( LOGVERBOSITY v, TCHAR *String );
  void Dump( CString fname );
};

class CLogger
{
  CArray<CLoggerOutput*> Outputs;
  TS32 Verbosity;
  TS32 NewEntryCount;

public:

  CLogger();
  virtual ~CLogger();
  void Close();
  void Log( LOGVERBOSITY v, TBOOL Prefix, TBOOL TimeStamp, TCHAR *String, ... );
  void SetVerbosity( LOGVERBOSITY v );
  void AddOutput( CLoggerOutput *Output );
  void RemoveOutput( CLoggerOutput *Output );
  void ResetEntryCounter();
  TS32 GetNewEntryCount();
};

extern CLogger Logger;

#ifdef ENABLE_LOGGING
#define LOG(v,s,...) Logger.Log(v,true,true,s,__VA_ARGS__)
#else
#define LOG(v,s,...) do{}while (0)
#endif 

#ifdef ENABLE_LOGGING
#define LOGSIMPLE(v,s,...) Logger.Log(v,false,false,s,__VA_ARGS__)
#else
#define LOGSIMPLE(v,s,...) do{}while (0)
#endif 

#define LOG_WARN(s,...) LOG(LOG_WARNING,_T(s),__VA_ARGS__)
#define LOG_ERR(s,...) LOG(LOG_ERROR,_T(s),__VA_ARGS__)
#define LOG_DBG(s,...) LOG(LOG_DEBUG,_T(s),__VA_ARGS__)
#define LOG_NFO(s,...) LOG(LOG_INFO,_T(s),__VA_ARGS__)

#define LOG_SETVERBOSITY(v) Logger.SetVerbosity(v)
#define LOG_ADDOUTPUT(v) Logger.AddOutput(v)
#define LOG_REMOVEOUTPUT(v) Logger.RemoveOutput(v)

//#define FORCEDOPENDEBUGFILE()  do { HANDLE h=CreateFile("debug.log",GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); CloseHandle(h); } while (0)
//#define FORCEDDEBUGLOG(v,...) do { HANDLE h=CreateFile("debug.log", FILE_APPEND_DATA , FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0); char str[1024]; for (int yyy=0; yyy<1024; yyy++) str[yyy]=0; sprintf_s(str,1023, v, __VA_ARGS__); int xxx=0; for (int zzz=0; zzz<1023; zzz++) if (str[zzz]!=0) xxx++; else break; str[xxx]='\n'; WriteFile(h, str, xxx+1, NULL, NULL); CloseHandle(h); } while (0)

#define FORCEDOPENDEBUGFILE()  do { } while (0)
#define FORCEDDEBUGLOG(v,...) do { } while (0)
