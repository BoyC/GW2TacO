#pragma once

//Call stack tracker class for tracking resource allocations

enum LOGVERBOSITY;

#ifdef ENABLE_STACKTRACKER_CLASS

class CStackTracker
{
  static TBOOL DbgInitialized;

public:
  void *Stack[ STACK_TRACE_DEPTH ];

  CStackTracker( TS8 Offset = 0 );
  CStackTracker( void *Context, TS8 Offset = 0 );
  void DumpToLog( LOGVERBOSITY v );
  void DumpToDebugOutput();
  TS8 *DumpToString();

  static void InitializeSym();
};

#else

class CStackTracker
{
public:
  INLINE CStackTracker( TS8 Offset = 0 ) {};
  INLINE CStackTracker( void *Context, TS8 Offset = 0 ) {}
  INLINE virtual ~CStackTracker() {}
  INLINE void DumpToLog( LOGVERBOSITY v ) {}
  INLINE void DumpToDebugOutput() {}
  INLINE TS8 *DumpToString() { return 0; }
};

#endif