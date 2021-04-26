#include "BaseLib.h"

const TBOOL operator == ( const CFileListEntry &f1, const CFileListEntry &f2 )
{
  return f1.Path == f2.Path && f1.FileName == f2.FileName;
}

TBOOL exists( const CString &fname )
{
  char Fname[ 2048 ];
  fname.WriteAsMultiByte( Fname, 2048 );

  FILE *f = NULL;
  if ( fopen_s( &f, Fname, "rb" ) )
    return false;
  if ( !f ) return false;
  fclose( f );
  return true;
}

CFileList::CFileList( const CString &Mask, const CString &Path/* ="" */, TBOOL Recursive/* =false */ )
{
  ExpandSearch( Mask, Path, Recursive );
}

CFileList::CFileList()
{

}

void CFileList::ExpandSearch( const CString &Mask, const CString &Path, TBOOL Recursive, TBOOL getDirectories )
{
  HANDLE hSearch;
  WIN32_FIND_DATA FileData;

  CString ValidPath = Path;
  if ( ValidPath[ ValidPath.Length() - 1 ] != '/' && ValidPath[ ValidPath.Length() - 1 ] != '\\' )
    ValidPath += "/";

  hSearch = FindFirstFile( ( ValidPath + Mask ).GetPointer(), &FileData );

  if ( hSearch != INVALID_HANDLE_VALUE )
  {
    BOOL fFinished = FALSE;
    while ( !fFinished )
    {
      CString FileName = FileData.cFileName;

      if ( !( FileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY ) && FileName != "." && FileName != ".." )
      {
        Files.Add( CFileListEntry( ValidPath, FileName ) );
      }

      if ( getDirectories && ( ( FileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY ) && FileName != "." && FileName != ".." ) )
      {
        CFileListEntry e = CFileListEntry( ValidPath, FileName );
        e.isDirectory = true;
        Files.Add( e );
      }

      fFinished = !FindNextFile( hSearch, &FileData );
    }
  }
  FindClose( hSearch );

  //find all directories
  if ( Recursive )
  {
    hSearch = FindFirstFile( ( ValidPath + "*.*" ).GetPointer(), &FileData );
    if ( hSearch != INVALID_HANDLE_VALUE )
    {
      BOOL fFinished = FALSE;
      while ( !fFinished )
      {
        CString FileName = FileData.cFileName;

        if ( FileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY && FileName != "." && FileName != ".." )
        {
          ExpandSearch( Mask, ValidPath + FileData.cFileName + "/", Recursive );
        }
        fFinished = !FindNextFile( hSearch, &FileData );
      }
    }
    FindClose( hSearch );
  }
}

CFileList::~CFileList()
{

}

CFileListEntry::CFileListEntry( const CString &pth, const CString &fn )
{
  Path = pth;
  FileName = fn;
}

CFileListEntry::CFileListEntry()
{

}
