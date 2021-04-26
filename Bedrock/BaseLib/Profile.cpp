#include "BaseLib.h"
#include <MMSystem.h>
#pragma comment(lib,"winmm.lib")

CProfile::CProfile( TU64 key, const CString &label )
{
  Key = key;
  Time = timeGetTime();
  Label = label;
}

CProfile::~CProfile( void )
{
  TU32 nt = timeGetTime();
  TU32 t = nt - Time;

  LOG( LOG_DEBUG, _T( "[profile] %s: %d" ), Label.GetPointer(), t );

  //GENXY_PROFILEDATA * pd = cmProfileData.GetByID(nKey);
  //if (pd)
  //	pd->fTime += t;
  //else 
  //{
  //	pd = new GENXY_PROFILEDATA;
  //	pd->fTime = t;
  //	pd->sLabel = sLabel;
  //	cmProfileData.Add(nKey,pd);
  //}
}
