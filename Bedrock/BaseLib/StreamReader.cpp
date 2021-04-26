#include "BaseLib.h"

//////////////////////////////////////////////////////////////////////////
// base class implementation

CStreamReader::CStreamReader()
{
  readerBitOffset = 0;
  readerLastChar = 0;
}

CStreamReader::~CStreamReader()
{

}

TS32 CStreamReader::Read( void *lpBuf, TU32 nCount )
{
  if ( readerBitOffset == 0 ) //non bitstream mode
    return ReadStream( lpBuf, nCount );

  //bitstream mode
  for ( TU32 x = 0; x < nCount; x++ )
    ( (TU8*)lpBuf )[ x ] = ReadBits( 8 );

  return nCount;
}

TU32 CStreamReader::ReadDWord()
{
  TU32 i = 0;
  BASEASSERT( Read( &i, sizeof( TU32 ) ) == sizeof( TU32 ) );
  return i;
}

TU16 CStreamReader::ReadWord()
{
  TU16 i = 0;
  BASEASSERT( Read( &i, sizeof( TU16 ) ) == sizeof( TU16 ) );
  return i;
}

TU8 CStreamReader::ReadByte()
{
  TU8 i = 0;
  BASEASSERT( Read( &i, sizeof( TU8 ) ) == sizeof( TU8 ) );
  return i;
}

TF32 CStreamReader::ReadTF32()
{
  TF32 i = 0;
  BASEASSERT( Read( &i, sizeof( TF32 ) ) == sizeof( TF32 ) );
  return i;
}

TU64 CStreamReader::ReadQWord()
{
  TU64 i = 0;
  BASEASSERT( Read( &i, sizeof( TU64 ) ) == sizeof( TU64 ) );
  return i;
}

TU32 CStreamReader::ReadBits( TU32 BitCount )
{
  BASEASSERT( BitCount <= 64 );

  TU32 result = 0;

  while ( BitCount > 0 )
  {
    //read next byte if needed
    if ( readerBitOffset == 0 )
      BASEASSERT( ReadStream( &readerLastChar, 1 ) == 1 );

    TU32 count = min( 8 - readerBitOffset, BitCount );
    TU32 mask = ( 1 << count ) - 1;

    TU8 bits = readerLastChar >> readerBitOffset;
    result |= ( bits&mask ) << ( BitCount - count );
    BitCount -= count;

    readerBitOffset = ( readerBitOffset + count ) & 7;
  }

  return result;
}

TBOOL CStreamReader::ReadBit()
{
  return ReadBits( 1 ) != 0;
}

void CStreamReader::ReadRemainingBits()
{
  readerBitOffset = 0;
}

CString CStreamReader::ReadASCIIZ()
{
  CString result;

  TS8 s[ 2 ];
  s[ 0 ] = s[ 1 ] = 0;

  do
  {
    s[ 0 ] = ReadByte();

    result += s;

  } while ( s[ 0 ] );

  return result;
}

CString CStreamReader::ReadLine()
{
  CString result;

  TS8 s[ 2 ];
  s[ 0 ] = s[ 1 ] = 0;

  do
  {
    if ( GetOffset() == GetLength() )
      break;

    s[ 0 ] = ReadByte();

    if ( s[ 0 ] != '\n' && s[ 0 ] != '\r' )
      result += s;

  } while ( s[ 0 ] && s[ 0 ] != '\n' );

  return result;
}

TBOOL CStreamReader::eof()
{
  return GetOffset() >= GetLength();
}

//////////////////////////////////////////////////////////////////////////
// streamreader memory

CStreamReaderMemory::CStreamReaderMemory() : CStreamReader()
{
  Data = NULL;
  DataSize = 0;
  Offset = 0;
}

CStreamReaderMemory::~CStreamReaderMemory()
{
  SAFEDELETEA( Data );
}

TS32 CStreamReaderMemory::ReadStream( void *lpBuf, TU32 nCount )
{
  TS64 bytestoread = max( 0, min( nCount, DataSize - Offset ) );
  memcpy( lpBuf, Data + Offset, (size_t)bytestoread );
  Offset += bytestoread;
  return (TS32)bytestoread;
}

TS32 CStreamReaderMemory::Open( TU8 *data, TU32 size )
{
  if ( !data || !size ) return 0;

  SAFEDELETEA( Data );
  DataSize = size;
  Offset = 0;

  Data = new TU8[ size ];
  memcpy( Data, data, size );

  return 1;
}

TS32 CStreamReaderMemory::Open( TCHAR *Filename )
{
  HANDLE hFile = CreateFile( Filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL );
  if ( hFile == INVALID_HANDLE_VALUE ) return 0;

  TS32 tDataSize = GetFileSize( hFile, NULL );

  TU8 *tData = new TU8[ tDataSize ];
  DWORD nRead = 0;
  BOOL b = ReadFile( hFile, tData, tDataSize, &nRead, NULL );

  if ( b && nRead == tDataSize ) //all ok
  {
    SAFEDELETEA( Data );
    Data = tData;
    DataSize = tDataSize;
    Offset = 0;
  }
  else
  {
    SAFEDELETEA( tData ); //couldn't read complete file - fail.
  }

  CloseHandle( hFile );
  return nRead == tDataSize;
}

TU8 *CStreamReaderMemory::GetData() const
{
  return Data;
}

TS64 CStreamReaderMemory::GetLength() const
{
  return DataSize;
}

TS64 CStreamReaderMemory::GetOffset() const
{
  return Offset;
}

void CStreamReaderMemory::SeekFromStart( TU64 lOff )
{
  Offset = lOff;
}

void CStreamReaderMemory::SeekRelative( TS64 lOff )
{
  Offset = max( 0, min( DataSize, Offset + lOff ) );
}

//////////////////////////////////////////////////////////////////////////
// streamreader file

CStreamReaderFile::CStreamReaderFile() : CStreamReader()
{
  File = 0;
}

CStreamReaderFile::~CStreamReaderFile()
{
  CloseHandle( File );
}

TS32 CStreamReaderFile::ReadStream( void *lpBuf, TU32 nCount )
{
  DWORD nRead = 0;
  BOOL b = ReadFile( File, lpBuf, nCount, &nRead, NULL );
  if ( !b ) return 0;
  return nRead;
}

TS32 CStreamReaderFile::Open( TCHAR *Filename )
{
  if ( File )
    CloseHandle( File ); //close previous handle

  File = CreateFile( Filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL );
  if ( File == INVALID_HANDLE_VALUE ) return 0;
  return 1;
}

TS64 CStreamReaderFile::GetLength() const
{
  return GetFileSize( File, NULL );
}

TS64 CStreamReaderFile::GetOffset() const
{
  return SetFilePointer( File, NULL, NULL, FILE_CURRENT );
}

void CStreamReaderFile::SeekFromStart( TU64 lOff )
{
  LARGE_INTEGER li;
  li.QuadPart = lOff;
  DWORD res = SetFilePointer( File, li.LowPart, &li.HighPart, FILE_BEGIN );
}

void CStreamReaderFile::SeekRelative( TS64 lOff )
{
  LARGE_INTEGER li;
  li.QuadPart = lOff;
  DWORD res = SetFilePointer( File, li.LowPart, &li.HighPart, FILE_CURRENT );
}