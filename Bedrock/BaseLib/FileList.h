#pragma once

class CFileListEntry
{
public:
  CString Path;
  CString FileName;
  TBOOL isDirectory = false;

  CFileListEntry();

  CFileListEntry( const CString &pth, const CString &fn );
};

const TBOOL operator == ( const CFileListEntry &f1, const CFileListEntry &f2 );

class CFileList
{
public:
  CArray<CFileListEntry> Files;

  CFileList();

  ~CFileList();

  CFileList( const CString &Mask, const CString &Path = "", TBOOL Recursive = false );
  void ExpandSearch( const CString &Mask, const CString &Path, TBOOL Recursive, TBOOL getDirectories = false );
};

TBOOL exists( const CString &fname );