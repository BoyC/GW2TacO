#pragma once
#include "../BaseLib/BaseLib.h"

class CSocketSSL : public CSocket
{
public:
  CSocketSSL( void );
  ~CSocketSSL( void );
  virtual TS32 ReadStream( void *lpBuf, TU32 nCount );
  virtual TS32 WriteStream( void* lpBuf, TU32 nCount );
  virtual TS32 Connect( const CString &Server, const TU32 Port );
  virtual TS32 Close();
  void LogSSLError();
private:
  void * ssl;
  void * ctx;
};

