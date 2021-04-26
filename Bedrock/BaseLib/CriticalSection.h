#pragma once

volatile struct LIGHTWEIGHT_CRITICALSECTION
{
  volatile TS32 threadID = 0;
  volatile TS32 spinCount = 0;
  volatile TS32 threadDibsID = 0;
};

#define THREAD_UNUSED 0

void InitializeLightweightCS( LIGHTWEIGHT_CRITICALSECTION * cs );
void EnterLightweightCS( LIGHTWEIGHT_CRITICALSECTION * cs );
void LeaveLightweightCS( LIGHTWEIGHT_CRITICALSECTION * cs );
TBOOL IsLightweightCSInUse( LIGHTWEIGHT_CRITICALSECTION * cs );

class CLightweightCriticalSection
{
  LIGHTWEIGHT_CRITICALSECTION * cs;

public:
  CLightweightCriticalSection( LIGHTWEIGHT_CRITICALSECTION * cs );
  ~CLightweightCriticalSection();
};