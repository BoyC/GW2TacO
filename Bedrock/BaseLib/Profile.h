#pragma once

//profiling class - measures the length the instance has been alive

class CProfile
{
  TU32 Time;
  TU64 Key;
  CString Label;

public:
  CProfile( TU64, const CString &label );
  ~CProfile( void );

};