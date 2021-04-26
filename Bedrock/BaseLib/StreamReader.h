#pragma once

class CStreamReader
{
  TU32 readerBitOffset;
  TU8 readerLastChar;
  virtual TS32 ReadStream( void *lpBuf, TU32 nCount ) = NULL; //this reads nCount bytes from the stream

public:

  CStreamReader();
  virtual ~CStreamReader();

  //general purpose reading functions with bitstream support
  //these aren't virtual as they all fall back on ReadStream at one point and should not be overridden
  TS32 Read( void *lpBuf, TU32 nCount );
  TU64 ReadQWord();
  TU32 ReadDWord();
  TU16 ReadWord();
  TU8 ReadByte();
  TU32 ReadBits( TU32 BitCount );
  TBOOL ReadBit();
  TF32 ReadTF32();
  void ReadRemainingBits();

  CString ReadASCIIZ();
  virtual CString ReadLine();

  virtual TS64 GetLength() const = NULL;
  virtual TS64 GetOffset() const = NULL;
  virtual TBOOL eof();

  virtual void SeekFromStart( TU64 lOff ) = NULL;
  virtual void SeekRelative( TS64 lOff ) = NULL;
};

class CStreamReaderMemory : public CStreamReader
{
  TU8 *Data;
  TU64 DataSize;
  TU64 Offset;

  virtual TS32 ReadStream( void *lpBuf, TU32 nCount );

public:

  CStreamReaderMemory();
  virtual ~CStreamReaderMemory();

  virtual TS32 Open( TU8 *data, TU32 size );
  virtual TS32 Open( TCHAR *filename );

  virtual TU8 *GetData() const;
  virtual TS64 GetLength() const;
  virtual TS64 GetOffset() const;

  virtual void SeekFromStart( TU64 lOff );
  virtual void SeekRelative( TS64 lOff );
};

class CStreamReaderFile : public CStreamReader
{
  HANDLE File;

  virtual TS32 ReadStream( void *lpBuf, TU32 nCount );

public:

  CStreamReaderFile();
  virtual ~CStreamReaderFile();

  TS32 Open( TCHAR *filename );

  virtual TS64 GetLength() const;
  virtual TS64 GetOffset() const;

  virtual void SeekFromStart( TU64 lOff );
  virtual void SeekRelative( TS64 lOff );
};