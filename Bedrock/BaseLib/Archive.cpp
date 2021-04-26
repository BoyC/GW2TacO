#include "BaseLib.h"

#define ARCHIVE_SIGNATURE ("CONSPRCY")
#define ARCHIVE_FILLER ("Conspiracy")

void CArchiveEntry::Init( const CArchiveEntry &f )
{
  StartChunk = f.StartChunk;
  FileSize = f.FileSize;
  Hash = f.Hash;
  FileName = f.FileName;
}

CArchiveEntry::CArchiveEntry()
{
  StartChunk = 0;
  FileSize = 0;
  Hash = 0;
}

CArchiveEntry::CArchiveEntry( const CArchiveEntry &f )
{
  Init( f );
}

CArchiveEntry::~CArchiveEntry()
{

}

const CArchiveEntry &CArchiveEntry::operator = ( const CArchiveEntry &f )
{
  Init( f );
  return *this;
}

CStreamReaderArchive::CStreamReaderArchive() : CStreamReaderMemory()
{
}

CStreamReaderArchive::~CStreamReaderArchive()
{
}

TS32 CStreamReaderArchive::Open( CArchive *BF, TS32 StartChunk, TS32 FileSize )
{
  if ( !BF || !FileSize || !StartChunk ) return 0;

  TU8 *FileData = new TU8[ FileSize ];

  TS32 pos = 0;
  TS32 remaining = FileSize;
  TS32 Chunk = StartChunk;

  while ( remaining )
  {
    TS32 read = BF->ReadChunk( Chunk, FileData + pos, remaining, Chunk );
    remaining -= read;
    pos += read;
    if ( !read || remaining < 0 || !Chunk ) break;
  }

  if ( Chunk || remaining < 0 ) //something got fucked up
  {
    SAFEDELETEA( FileData );
    return 0;
  }

  TBOOL b = CStreamReaderMemory::Open( FileData, FileSize );

  SAFEDELETEA( FileData );

  return b;
}

CArchive::CArchive()
{
  Handle = NULL;
  FileSize = 0;
  ChunkSize = 0;
  EmptySequenceStartChunk = 0;
  FileIndexChunk = 0;
  TempChunk = NULL;
  FileIndexSize = 0;
  ReadOnly = true;
}

CArchive::~CArchive()
{
  if ( Handle )
  {
    fclose( Handle );
    Handle = NULL;
  }
  SAFEDELETEA( TempChunk );
}

TBOOL CArchive::SeekToChunk( TS32 Chunk )
{
  if ( !Handle ) return false;
  return fseek( Handle, Chunk*ChunkSize, SEEK_SET ) == 0;
}

TS32 CArchive::ReadChunk( TS32 ChunkID, TU8 *Data, TS32 BufferSize, TS32 &NextChunk )
{
  NextChunk = 0;

  if ( !Data || !Handle ) return 0;
  if ( !SeekToChunk( ChunkID ) ) return 0;

  if ( fread_s( TempChunk, ChunkSize, ChunkSize, 1, Handle ) != 1 )
    return 0;

  NextChunk = ( (TS32*)TempChunk )[ 0 ];
  TU16 DataSize = ( (TU16*)TempChunk )[ 2 ];
  if ( DataSize > BufferSize )
  {
    DataSize = BufferSize;
    LOG( LOG_ERROR, _T( "[arch] Archive inconsistency in chunk %d, data size does not match expected value!" ), ChunkID );
  }
  memcpy( Data, TempChunk + 6, DataSize );

  return DataSize;
}

TBOOL CArchive::ReadIndex( CStreamReaderArchive *idx )
{
  if ( !idx ) return false;

  while ( !idx->eof() )
  {
    CArchiveEntry f;

    f.StartChunk = idx->ReadDWord();
    f.FileSize = idx->ReadDWord();
    f.Hash = idx->ReadQWord();
    f.FileName = idx->ReadASCIIZ();

    FileIndices.Add( f );
  }

  return true;
}


TBOOL CArchive::Open( const CString &FileName, TBOOL ro )
{
  ReadOnly = ro;
  if ( _tfopen_s( &Handle, FileName.GetPointer(), ReadOnly ? _T( "rb" ) : _T( "r+b" ) ) ) return false;
  if ( !Handle ) return false;

  ARCHIVEHEADER h;
  if ( fread_s( &h, sizeof( h ), sizeof( h ), 1, Handle ) != 1 )
  {
    fclose( Handle );
    Handle = NULL;
    return false;
  }

  //check signature
  for ( int x = 0; x < sizeof( h.Signature ); x++ )
  {
    if ( h.Signature[ x ] != ARCHIVE_SIGNATURE[ x ] )
    {
      fclose( Handle );
      Handle = NULL;
      return false;
    }
  }

  ChunkSize = h.ChunkSize;
  EmptySequenceStartChunk = h.EmptyChunk;
  FileIndexChunk = h.FileIndexChunk;

  if ( fseek( Handle, 0, SEEK_END ) )
  {
    fclose( Handle );
    Handle = NULL;
    return false;
  }

  if ( FileSize%ChunkSize || ChunkSize < sizeof( ARCHIVEHEADER ) || FileSize < ChunkSize )
  {
    fclose( Handle );
    Handle = NULL;
    return false;
  }

  TempChunk = new TU8[ ChunkSize ];
  for ( TS32 x = 0; x < ChunkSize; x++ )
    TempChunk[ x ] = ARCHIVE_FILLER[ x % ( sizeof( ARCHIVE_FILLER ) - 1 ) ];

  if ( FileIndexChunk > 0 )
  {
    CStreamReaderArchive *Index = new CStreamReaderArchive();
    if ( !Index->Open( this, FileIndexChunk, FileIndexSize ) )
    {
      SAFEDELETE( Index );
      return false;
    }

    if ( !ReadIndex( Index ) )
    {
      SAFEDELETE( Index );
      return false;
    }

    SAFEDELETE( Index );
  }
  else
    LOG( LOG_WARNING, _T( "[arch] File index chunk is 0. Assuming empty archive." ) );

  return true;
}

TBOOL CArchive::Create( const CString &FileName, TU16 chunksize )
{
  ReadOnly = false;
  if ( chunksize < sizeof( ARCHIVEHEADER ) ) return false;

  if ( _tfopen_s( &Handle, FileName.GetPointer(), _T( "w+b" ) ) ) return false;
  if ( !Handle ) return false;

  ARCHIVEHEADER h;
  memcpy( &h.Signature, ARCHIVE_SIGNATURE, sizeof( h.Signature ) );
  ChunkSize = h.ChunkSize = chunksize;
  EmptySequenceStartChunk = h.EmptyChunk = 0;
  FileIndexChunk = h.FileIndexChunk = 0;
  FileIndexSize = h.FileIndexSize = 0;

  if ( fwrite( &h, sizeof( h ), 1, Handle ) != 1 )
  {
    fclose( Handle );
    Handle = NULL;
    return false;
  }

  TempChunk = new TU8[ ChunkSize ];
  for ( TS32 x = 0; x < ChunkSize; x++ )
    TempChunk[ x ] = ARCHIVE_FILLER[ x % ( sizeof( ARCHIVE_FILLER ) - 1 ) ];

  //fill up the rest of chunk #0
  if ( fwrite( TempChunk, ChunkSize - sizeof( h ), 1, Handle ) != 1 )
  {
    fclose( Handle );
    Handle = NULL;
    return false;
  }

  FileSize = ChunkSize;

  return true;
}

TBOOL CArchive::UpdateHeader()
{
  if ( !Handle || ReadOnly ) return false;
  if ( fseek( Handle, 0, SEEK_SET ) ) return false;

  ARCHIVEHEADER h;
  memcpy( &h.Signature, ARCHIVE_SIGNATURE, sizeof( h.Signature ) );
  h.ChunkSize = ChunkSize;
  h.EmptyChunk = EmptySequenceStartChunk;
  h.FileIndexChunk = FileIndexChunk;
  h.FileIndexSize = FileIndexSize;

  if ( fwrite( &h, sizeof( h ), 1, Handle ) != 1 )
  {
    fclose( Handle );
    Handle = NULL;
    return false;
  }

  return true;
}

TBOOL CArchive::UpdateIndex( TBOOL IncludeFilenames )
{
  if ( !Handle || ReadOnly ) return false;

  CStreamWriterMemory mf;
  for ( TS32 x = 0; x < FileIndices.NumItems(); x++ )
  {
    mf.WriteDWord( FileIndices[ x ].StartChunk );
    mf.WriteDWord( FileIndices[ x ].FileSize );
    mf.WriteQWord( FileIndices[ x ].Hash );
    if ( IncludeFilenames && FileIndices[ x ].FileName.Length() )
    {
      TS8 *sz = new TS8[ FileIndices[ x ].FileName.Length() + 1 ];
      FileIndices[ x ].FileName.WriteAsMultiByte( sz, FileIndices[ x ].FileName.Length() + 1 );
      mf.WriteFormatZT( _T( "%s" ), sz );
      delete[] sz;
    }
    else
      mf.WriteByte( 0 );
  }

  TS32 OldIdxStart = FileIndexChunk;

  if ( !WriteFile( mf.GetData(), mf.GetLength(), FileIndexChunk ) )
  {
    LOG( LOG_ERROR, _T( "[arch] Critical error writing new Archive index" ) );
    return false;
  }
  FileIndexSize = mf.GetLength();

  if ( OldIdxStart > 0 )
  {
    if ( !ClearChunkSequence( OldIdxStart ) ) //this updates the header as well
    {
      LOG( LOG_ERROR, _T( "[arch] Critical error updating Archive header (index update)" ) );
      return false;
    }
    return true;
  }
  else return UpdateHeader();
}

TBOOL CArchive::ClearChunkSequence( TS32 StartChunk )
{
  if ( ReadOnly ) return false;
  if ( StartChunk <= 0 || StartChunk*ChunkSize >= FileSize ) return false; //invalid chunk id

  TS32 chunk = StartChunk;
  TS32 LastChunk = chunk;
  while ( chunk )
  {
    LastChunk = chunk;
    if ( !SeekToChunk( chunk ) ) return false;
    if ( fread_s( &chunk, 4, 4, 1, Handle ) != 1 ) return false;
  }

  if ( !SeekToChunk( LastChunk ) ) return false;
  if ( fwrite( &EmptySequenceStartChunk, 4, 1, Handle ) != 1 ) return false;

  //this code zeros the chunk - debug use only
  //for (TS32 x=0; x<ChunkSize-4; x++)
  //{
  //	char y='.';
  //	fwrite(&y,1,1,Handle);
  //}
  //CurrentPos+=ChunkSize-4;

  EmptySequenceStartChunk = StartChunk;

  return UpdateHeader();
}

TBOOL CArchive::AddFile( TU8 *Data, TS32 Size, const CString &FileName )
{
  if ( ReadOnly || !Handle ) return false;

  TS32 sc = 0;
  CArchiveEntry f;
  f.FileName = FileName;
  f.FileSize = Size;
  f.Hash = CalculateHash( FileName );

  if ( !WriteFile( Data, Size, f.StartChunk ) )
  {
    LOG( LOG_ERROR, _T( "[arch] An error occured while adding a file to the archive" ) );
    return false;
  }

  FileIndices.Add( f );
  return UpdateIndex();
}

TBOOL CArchive::WriteFile( TU8 *Data, TS32 Size, TS32 &startchunk )
{
  if ( !Handle || ReadOnly ) return false;

  TS32 TargetChunk = startchunk = EmptySequenceStartChunk;
  if ( !startchunk ) startchunk = (TS32)( FileSize / ChunkSize );

  while ( Size )
  {
    if ( !WriteChunk( TargetChunk, Data, Size ) )
    {
      LOG( LOG_ERROR, _T( "[arch] Critical error writing data to archive" ) );
      return false;
    }
    if ( Size < 0 )
    {
      LOG( LOG_ERROR, _T( "[arch] Critical error writing data to archive" ) );
      return false;
    }
  }

  EmptySequenceStartChunk = TargetChunk;
  //UpdateHeader(); //for now whatever calls this also updates the header

  return true;
}

TBOOL CArchive::WriteChunk( TS32 &Chunk, TU8 *&Data, TS32 &DataSize )
{
  if ( !Handle || ReadOnly ) return false;

  TBOOL eof = Chunk == 0;
  TBOOL FileFits = DataSize <= ChunkSize - 6;

  //seek to chunk and write next chunk information if needed
  if ( eof ) //we're appending new chunks at the end of the Archive
  {
    if ( fseek( Handle, 0, SEEK_END ) ) return false;

    TS32 NextChunk = 0;
    if ( !FileFits ) NextChunk = (TS32)( ftell( Handle ) / ChunkSize ) + 1; //file isn't over yet, reference the next chunk

    if ( fwrite( &NextChunk, 4, 1, Handle ) != 1 ) return false; //write next chunk id
  }
  else //we're replacing chunks inside the Archive
  {
    TS32 CurrentChunk = Chunk;

    //first read the next chunk id from the empty sequence
    if ( !SeekToChunk( CurrentChunk ) ) return false;
    if ( fread_s( &Chunk, 4, 4, 1, Handle ) != 1 ) return false; //also advances empty sequence by overwriting the currently checked chunk id

    //modify if needed
    TS32 NextChunk = Chunk;
    if ( FileFits ) NextChunk = 0; //file is over, sequence needs to be terminated
    if ( !Chunk && !FileFits ) NextChunk = (TS32)( FileSize / ChunkSize ); //file isn't over yet but the empty sequence is, refer to eof chunk (to be added later)

    if ( Chunk != NextChunk ) //next chunk id changed
    {
      if ( !SeekToChunk( CurrentChunk ) ) return false;
      if ( fwrite( &NextChunk, 4, 1, Handle ) != 1 ) return false; //overwrite next chunk id
    }
  }

  fseek( Handle, 0, SEEK_CUR ); //switch to writing mode

  TU16 ds = min( DataSize, ChunkSize - 6 );
  if ( fwrite( &ds, 2, 1, Handle ) != 1 ) return false; //write current chunk data content size
  if ( fwrite( Data, ds, 1, Handle ) != 1 ) return false; //write data

  Data += ds;
  DataSize -= ds;

  if ( ChunkSize - 6 - ds > 0 )
    if ( fwrite( TempChunk, ChunkSize - 6 - ds, 1, Handle ) != 1 ) return false; //fill rest of the chunk

  if ( eof ) FileSize += ChunkSize; //adjust filesize
  return true;
}

#define HASHSEED 0x811C9DC5
#define HASHMULT 0x1000193

TU64 CArchive::CalculateHash( const CString &Filename )
{
  TU64 x = 0;

  TS8 *String = new TS8[ Filename.Length() + 2 ];
  String[ Filename.Length() + 1 ] = 0;
  Filename.WriteAsMultiByte( String, Filename.Length() + 1 );

  TU64 Hash = HASHSEED;
  do
  {
    Hash = HASHMULT * ( tolower( String[ x++ ] ) ^ Hash );
  } while ( x < strlen( String ) );

  delete[] String;

  return Hash;
}

TBOOL CArchive::OpenFile( const CString &FileName, CStreamReaderArchive *&Reader )
{
  if ( !Reader || !Handle ) return NULL;

  TS32 idx = -1;

  for ( TS32 x = 0; x < FileIndices.NumItems(); x++ )
  {
    if ( CString::CompareNoCase( FileIndices[ x ].FileName, FileName ) == 0 )
    {
      idx = x;
      break;
    }
  }

  if ( idx == -1 )
  {
    TU64 hash = CalculateHash( FileName );

    for ( TS32 x = 0; x < FileIndices.NumItems(); x++ )
      if ( FileIndices[ x ].Hash == hash )
      {
        idx = x;
        break;
      }
  }

  if ( idx == -1 ) return false;

  return Reader->Open( this, FileIndices[ idx ].StartChunk, FileIndices[ idx ].FileSize );
}