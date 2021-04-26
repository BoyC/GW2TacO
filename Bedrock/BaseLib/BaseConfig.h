#pragma once

//base library config

//////////////////////////////////////////////////////////////////////////
// global tracking enabler macro

#define ENABLE_TRACKING

#ifdef ENABLE_TRACKING
//////////////////////////////////////////////////////////////////////////
// call stack tracker class
// stack tracing requires the "omit frame pointer" optimization to be DISABLED <- !!!

#define ENABLE_STACKTRACKER_CLASS

//////////////////////////////////////////////////////////////////////////
// memory profiler config

#ifdef _DEBUG
#define MEMORY_TRACKING
#endif

#ifdef ENABLE_STACKTRACKER_CLASS
#ifdef MEMORY_TRACKING
#define ENABLE_MALLOC_STACK_TRACE
#endif
#endif

#endif

//////////////////////////////////////////////////////////////////////////
// global config

#define INLINE 
#define ASSERTMODULENAME "Perpetuum"

#define HASHED_STRINGS
//#define FAST_INVSQRT

#define STACK_TRACE_DEPTH 10
