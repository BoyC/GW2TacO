#pragma once
#include "GW2Tactical.h"
#include "TrailLogger.h"

#define MINIZ_HEADER_FILE_ONLY
#include "Bedrock/UtilLib/miniz.c"

class MarkerPack
{
public:
  CString name;
  CString id;
  CString versionURL;
  CString fileName;
  CString versionSearchString;
  CString versionTerminator;
  CString downloadURL;
  CString backupURL;
  CString backupVersion;

  CString versionString;

  bool versionCheckDone = false;
  bool defaultEnabled = false;
  bool outdated = false;
  bool versionCheckOk = false;
  bool beingDownloaded = false;
  bool downloadFinished = false;
  bool failed = false;

  bool BackupVersionCheck();
  bool CheckVersion();
  bool NeedsUpdate();
  bool UpdateFromWeb();
};

TBOOL FindSavedCategory( GW2TacticalCategory* t );
void ExportSavedCategories( CXMLNode* n, GW2TacticalCategory* t );
void ExportPOI( CXMLNode* n, POI& p );
void ExportTrail( CXMLNode* n, GW2Trail& p );
void ExportPOIS( const CString& fileName = "poidata.xml", bool onlyCurrentMap = false );
GUID LoadGUID( CXMLNode& n );
void RecursiveImportPOIType( CXMLNode& root, GW2TacticalCategory* Root, CString currentCategory, MarkerTypeData& defaults, TBOOL KeepSaveState, const CString& zipFile );
void ImportPOITypes();
void ImportPOI( CWBApplication* App, CXMLNode& t, POI& p, const CString& zipFile );
TBOOL ImportTrail( CWBApplication* App, CXMLNode& t, GW2Trail& p, const CString& zipFile );
void ImportPOIDocument( CWBApplication* App, CXMLDocument& d, TBOOL External, const CString& zipFile );
void ImportPOIFile( CWBApplication* App, CString s, TBOOL External );
void ImportPOIString( CWBApplication* App, const CString& data, const CString& zipFile );
void ImportMarkerPack( CWBApplication* App, const CString& zipFile );
void ImportPOIS( CWBApplication* App );
void ImportPOIActivationData();
void ExportPOIActivationData();
void FlushZipDict();
mz_zip_archive* OpenZipFile( const CString& zipFile );
void UpdateMarkerPackList();
void WaitForMarkerPackUpdate();

extern CArrayThreadSafe< CString >markerPackQueue;
extern LIGHTWEIGHT_CRITICALSECTION dlTextCritSec;

CString GetCurrentDownload();
extern CArrayThreadSafe<MarkerPack> markerPacks;

void ExportMyMapMarkers();