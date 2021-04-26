#pragma once

class CStreamWriter
{
  TU32 writerBitOffset;
  TU8 writerCurrentChar;
  virtual TS32 WriteStream( void* lpBuf, TU32 nCount ) = 0;

public:
  CStreamWriter();
  virtual ~CStreamWriter();

  TBOOL Write( void* lpBuf, TU32 nCount );
  TBOOL WriteQWord( TU64 data );
  TBOOL WriteDWord( TU32 data );
  TBOOL WriteWord( TU16 data );
  TBOOL WriteByte( TU8 data );
  TBOOL WriteTF32( TF32 data );
  TBOOL WriteBits( TU32 data, TU32 bitcount );
  TBOOL WriteBool( TBOOL data );
  TBOOL WriteRemainingBits();
  TBOOL WriteASCIIZ( CString &s );

  //TBOOL WriteFormat(const wchar_t *format, ...);
  TBOOL WriteFormat( const TCHAR *format, ... );
  //TBOOL WriteFormatZT(const wchar_t *format, ...);
  TBOOL WriteFormatZT( const TCHAR *format, ... );
};

class CStreamWriterMemory : public CStreamWriter
{
  TU8 *Data;
  TU32 BufferSize;
  TU32 DataLength;

  virtual TS32 WriteStream( void* lpBuf, TU32 nCount );

public:
  CStreamWriterMemory();
  virtual ~CStreamWriterMemory();

  TU8 *GetData();
  TU32 GetLength();

  void Flush();
};

class CStreamWriterFile : public CStreamWriter
{
  HANDLE File;

  virtual TS32 WriteStream( void* lpBuf, TU32 nCount );

public:
  CStreamWriterFile();
  virtual ~CStreamWriterFile();

  TS32 Open( TCHAR *filename );
  TBOOL Flush();
};
