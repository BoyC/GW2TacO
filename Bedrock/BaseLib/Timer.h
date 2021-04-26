#pragma once

class CTimer
{
  TS32 LastUpdateTime;
  TS32 StartTime;
  TF32 SpeedModifier;
  TF64 TimeExtension;
  TBOOL Paused;

  unsigned long Time;
public:

  CTimer();
  virtual ~CTimer();

  void Update();
  void SetSpeed( TF32 Speed );
  void Pause( TBOOL Pause );
  unsigned long GetTime();

  TBOOL isPaused();
  void SkipTime( unsigned long Time );
};

extern CTimer globalTimer;
//TF32 GetAccurateTime();
