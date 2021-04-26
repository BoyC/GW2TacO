#pragma once

class CFPUDouble
{
  TU32 OriginalFPUState;

public:

  CFPUDouble();
  ~CFPUDouble();
};

class CFPUAnsiCRounding
{
  TU32 OriginalFPUState;

public:

  CFPUAnsiCRounding();
  ~CFPUAnsiCRounding();
};
