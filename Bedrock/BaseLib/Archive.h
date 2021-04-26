#pragma once

class CArchive;

#pragma pack(push,1)

struct ARCHIVEHEADER
{
  TS8 Signature[ 8 ];
  TU16 ChunkSize;
  TS32 EmptyChunk;
  TS32 FileIndexChunk;
  TS32 FileIndexSize;
};

class CArchiveEntry
{
  friend CArchive;

  TS32 StartChunk;
  TS32 FileSize;
  TU64 Hash;
  CString FileName;

  void Init( const CArchiveEntry &f );
public:

  CArchiveEntry();
  CArchiveEntry( const CArchiveEntry &f );
  virtual ~CArchiveEntry();
  virtual const CArchiveEntry &operator = ( const CArchiveEntry &f );

};

#pragma pack(pop)

class CStreamReaderArchive : public CStreamReaderMemory
{
public:
  CStreamReaderArchive();
  virtual ~CStreamReaderArchive();

  virtual TS32 Open( CArchive *BF, TS32 StartChunk, TS32 FileSize );
};

class CArchive
{
  friend CStreamReaderArchive;

  TBOOL ReadOnly;

  TU8 *TempChunk;
  FILE *Handle;

  TU64 FileSize;
  //TU64 CurrentPos;

  TU16 ChunkSize;
  TS32 EmptySequenceStartChunk;
  TS32 FileIndexChunk;
  TS32 FileIndexSize;

  CArray<CArchiveEntry> FileIndices;

  TS32 ReadChunk( TS32 ChunkID, TU8 *Data, TS32 BufferSize, TS32 &NextChunk );
  TBOOL ReadIndex( CStreamReaderArchive *idx );

  TBOOL SeekToChunk( TS32 Chunk );

  TBOOL UpdateHeader();
  TBOOL UpdateIndex( TBOOL IncludeFilenames = true );
  TBOOL ClearChunkSequence( TS32 StartChunk );
  TBOOL WriteFile( TU8 *Data, TS32 Size, TS32 &startchunk );
  TBOOL WriteChunk( TS32 &Chunk, TU8 *&Data, TS32 &DataSize );

  TU64 CalculateHash( const CString &Filename );

public:

  CArchive();
  virtual ~CArchive();

  TBOOL Open( const CString &FileName, TBOOL ReadOnly = true );
  TBOOL Create( const CString &FileName, TU16 ChunkSize = 1024 );

  TBOOL OpenFile( const CString &FileName, CStreamReaderArchive *&Reader );
  TBOOL AddFile( TU8 *Data, TS32 Size, const CString &FileName );

};