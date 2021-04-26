#include "BaseLib.h"

CStreamWriter::CStreamWriter()
{
  writerBitOffset = 0;
  writerCurrentChar = 0;
}

CStreamWriter::~CStreamWriter()
{

}

TBOOL CStreamWriter::Write( void* lpBuf, TU32 nCount )
{
  if ( writerBitOffset == 0 ) //non bitstream mode
    return WriteStream( lpBuf, nCount ) == nCount;

  //bitstream mode
  for ( TU32 x = 0; x < nCount; x++ )
    BASEASSERT( WriteBits( ( (TU8 *)lpBuf )[ x ], 8 ) == 1 );

  return true;
}

TBOOL CStreamWriter::WriteByte( TU8 data )
{
  return Write( &data, 1 );
}

TBOOL CStreamWriter::WriteWord( TU16 data )
{
  return Write( &data, 2 );
}

TBOOL CStreamWriter::WriteDWord( TU32 data )
{
  return Write( &data, 4 );
}

TBOOL CStreamWriter::WriteQWord( TU64 data )
{
  return Write( &data, 8 );
}

TBOOL CStreamWriter::WriteTF32( TF32 data )
{
  return Write( &data, 4 );
}

TBOOL CStreamWriter::WriteBits( TU32 data, TU32 BitCount )
{
  BASEASSERT( BitCount <= 64 );

  while ( BitCount > 0 )
  {
    TU32 count = min( 8 - writerBitOffset, BitCount );
    TU32 mask = ( 1 << count ) - 1;

    writerCurrentChar = (TU8)( ( writerCurrentChar & ( ~( mask << writerBitOffset ) ) ) | ( ( ( data >> ( BitCount - count ) ) & mask ) << writerBitOffset ) );

    BitCount -= count;
    writerBitOffset += count;

    if ( writerBitOffset >= 8 )
    {
      BASEASSERT( WriteStream( &writerCurrentChar, 1 ) );
      writerCurrentChar = 0;
    }

    writerBitOffset &= 7;
  }

  return true;
}

TBOOL CStreamWriter::WriteBool( TBOOL data )
{
  return WriteBits( data, 1 );
}

TBOOL CStreamWriter::WriteRemainingBits()
{
  if ( !writerBitOffset ) return true;

  TBOOL b = WriteStream( &writerCurrentChar, 1 ) != 0;
  writerBitOffset = 0;
  writerCurrentChar = 0;
  return b;
}

//TBOOL CStreamWriter::WriteFormat(const wchar_t *format, ...) {
//	CString s;
//
//	va_list argList;
//	va_start(argList, format);
//	CString::FormatVA(format, argList, s);
//	va_end(argList);
//
//	WCHAR * sz = new WCHAR[s.Length() + 1];
//	s.WriteAsWideChar(sz, s.Length() + 1);
//	TBOOL result = Write(sz, s.Length() * sizeof(wchar_t));
//	delete[] sz;
//	return result;
//}

TBOOL CStreamWriter::WriteFormat( const TCHAR *format, ... ) {
  CString s;

  va_list argList;
  va_start( argList, format );
  CString::FormatVA( format, argList, s );
  va_end( argList );

  char * sz = new char[ s.Length() + 1 ];
  s.WriteAsMultiByte( sz, s.Length() + 1 );
  TBOOL result = Write( sz, s.Length() * sizeof( char ) );
  delete[] sz;
  return result;
}

//TBOOL CStreamWriter::WriteFormatZT(const wchar_t *format, ...) {
//	CString s;
//
//	va_list argList;
//	va_start(argList, format);
//	CString::FormatVA(format, argList, s);
//	va_end(argList);
//
//	WCHAR * sz = new WCHAR[s.Length() + 1];
//	s.WriteAsWideChar(sz, s.Length() + 1);
//	TBOOL result = Write(sz, (s.Length() + 1) * sizeof(wchar_t));
//	delete[] sz;
//	return result;
//}

TBOOL CStreamWriter::WriteFormatZT( const TCHAR *format, ... ) {
  CString s;

  va_list argList;
  va_start( argList, format );
  CString::FormatVA( format, argList, s );
  va_end( argList );

  char * sz = new char[ s.Length() + 1 ];
  s.WriteAsMultiByte( sz, s.Length() + 1 );
  TBOOL result = Write( sz, ( s.Length() + 1 ) * sizeof( char ) );
  delete[] sz;
  return result;
}

TBOOL CStreamWriter::WriteASCIIZ( CString &s )
{
  TBOOL b = Write( s.GetPointer(), s.Length() * sizeof( TCHAR ) );
  b |= WriteByte( 0 );
  return b;
}

//////////////////////////////////////////////////////////////////////////
// streamwritermemory

CStreamWriterMemory::CStreamWriterMemory() : CStreamWriter()
{
  Data = new TU8[ 1024 ];
  BufferSize = 1024;
  DataLength = 0;
}

CStreamWriterMemory::~CStreamWriterMemory()
{
  SAFEDELETEA( Data );
  BufferSize = 0;
}

TS32 CStreamWriterMemory::WriteStream( void* lpBuf, TU32 nCount )
{
  if ( DataLength + nCount > BufferSize )
  {
    BufferSize = (TU32)( ( BufferSize + nCount )*1.2f );
    TU8 *temp = Data;
    Data = new TU8[ BufferSize ];
    memcpy( Data, temp, DataLength );
    SAFEDELETEA( temp );
  }

  memcpy( Data + DataLength, lpBuf, nCount );
  DataLength += nCount;

  return nCount;
}

TU8 *CStreamWriterMemory::GetData()
{
  return Data;
}

TU32 CStreamWriterMemory::GetLength()
{
  return DataLength;
}

void CStreamWriterMemory::Flush()
{
  SAFEDELETEA( Data );
  BufferSize = 0;
  DataLength = 0;

  Data = new TU8[ 1024 ];
  BufferSize = 1024;
  DataLength = 0;
}

//////////////////////////////////////////////////////////////////////////
// streamwriterfile

CStreamWriterFile::CStreamWriterFile() : CStreamWriter()
{
  File = NULL;
}

CStreamWriterFile::~CStreamWriterFile()
{
  if ( File )
    CloseHandle( File );
}

TS32 CStreamWriterFile::WriteStream( void *lpBuf, TU32 nCount )
{
  DWORD nWritten = 0;
  BOOL b = WriteFile( File, lpBuf, nCount, &nWritten, NULL );
  if ( !b ) return 0;
  return nWritten;
}

TS32 CStreamWriterFile::Open( TCHAR *Filename )
{
  if ( File )
    CloseHandle( File ); //close previous handle

  File = CreateFile( Filename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, NULL, NULL );
  CloseHandle( File );

  File = CreateFile( Filename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS | TRUNCATE_EXISTING, NULL, NULL );
  if ( File == INVALID_HANDLE_VALUE )
  {
    LPTSTR pMsgBuf;
    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                   (LPTSTR)&pMsgBuf, 0, NULL );

    LOG_ERR( "[writer] Error opening file '%s': %s", Filename, pMsgBuf );
    LocalFree( pMsgBuf );
    return 0;
  }
  return 1;
}

TBOOL CStreamWriterFile::Flush()
{
  SetFilePointer( File, 0, NULL, FILE_BEGIN );
  return SetEndOfFile( File ) != 0;
}
