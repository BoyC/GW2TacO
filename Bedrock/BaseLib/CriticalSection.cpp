#include "BaseLib.h"
#include <Windows.h>

void InitializeLightweightCS( LIGHTWEIGHT_CRITICALSECTION * cs )
{
  cs->threadID = THREAD_UNUSED;
  cs->spinCount = 0;
  cs->threadDibsID = THREAD_UNUSED;
}

void EnterLightweightCS( LIGHTWEIGHT_CRITICALSECTION * cs )
{
  DWORD id = GetCurrentThreadId();

  if ( cs->threadID == id )
  {
    cs->spinCount++;
    return;
  }

  // ----------------------------------------
  // mechanism:
  //  if cs->threadID == THREAD_UNUSED,
  //    sets cs->threadID to GetCurrentThreadId() and returns;
  //  if cs->threadID != THREAD_UNUSED,
  //    waits
  // ----------------------------------------

  while ( true )
  {
    if ( cs->threadDibsID == THREAD_UNUSED || cs->threadDibsID == id )
    {
      if ( InterlockedCompareExchange( (LONG*)&cs->threadID, id, THREAD_UNUSED ) == THREAD_UNUSED )
      {
        // got the lock
        cs->spinCount = 1;
        cs->threadDibsID = THREAD_UNUSED;
        return;
      }
    }

    // failed lock, wait
    InterlockedCompareExchange( (LONG*)&cs->threadDibsID, id, THREAD_UNUSED );

    Sleep( 1 );
  }
}

void LeaveLightweightCS( LIGHTWEIGHT_CRITICALSECTION * cs )
{
  cs->spinCount--;
  if ( cs->spinCount <= 0 )
  {
    cs->spinCount = 0;
    cs->threadID = THREAD_UNUSED;
  }
}

TBOOL IsLightweightCSInUse( LIGHTWEIGHT_CRITICALSECTION * cs )
{
  return !( cs->spinCount == 0 && cs->threadID == THREAD_UNUSED );
}


CLightweightCriticalSection::~CLightweightCriticalSection()
{
  LeaveLightweightCS( cs );
}

CLightweightCriticalSection::CLightweightCriticalSection( LIGHTWEIGHT_CRITICALSECTION * cs ) : cs( cs )
{
  EnterLightweightCS( cs );
}
