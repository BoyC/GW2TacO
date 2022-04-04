#include "MarkerPack.h"
#include "Language.h"
#include "OverlayConfig.h"
#include "GW2API.h"
#include "Bedrock/UtilLib/jsonxx.h"
#include <commdlg.h>
using namespace jsonxx;

CDictionary<CString, mz_zip_archive*> zipDict;
CArrayThreadSafe< CString > markerPackQueue;
CString currentDownload;
CArrayThreadSafe<MarkerPack> markerPacks;

extern bool disableHooks;
extern CWBApplication* App;


void ExportMyMapMarkers()
{
  disableHooks = true;

  TCHAR dir[ 1024 ];
  if ( !GetCurrentDirectory( 1024, dir ) )
    memset( dir, 0, sizeof( TCHAR ) * 1024 );
  char Filestring[ 256 ];

  OPENFILENAME opf;
  opf.hwndOwner = (HWND)App->GetHandle();
  opf.lpstrFilter = "GW2 Taco Markers\0*.xml\0\0";
  opf.lpstrCustomFilter = 0;
  opf.nMaxCustFilter = 0L;
  opf.nFilterIndex = 1L;
  opf.lpstrFile = Filestring;
  opf.lpstrFile[ 0 ] = '\0';
  opf.nMaxFile = 256;
  opf.lpstrFileTitle = 0;
  opf.nMaxFileTitle = 50;
  opf.lpstrInitialDir = "Data";
  opf.lpstrTitle = "Save My Map Markers";
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = "xml";
  opf.lpfnHook = NULL;
  opf.lCustData = 0;
  opf.Flags = ( OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NONETWORKBUTTON ) & ~OFN_ALLOWMULTISELECT;
  opf.lStructSize = sizeof( OPENFILENAME );

  opf.hInstance = GetModuleHandle( 0 );
  opf.pvReserved = NULL;
  opf.dwReserved = 0;
  opf.FlagsEx = 0;

  opf.lpstrInitialDir = dir;

  if ( GetSaveFileName( &opf ) )
  {
    ExportPOIS( CString( opf.lpstrFile ), true );
  }
  else
  {
    DWORD error = CommDlgExtendedError();
  }

  SetCurrentDirectory( dir );

  disableHooks = false;
}

bool markerPacksBeingFetched = false;
std::thread markerPackFetcherThread;

LIGHTWEIGHT_CRITICALSECTION dlTextCritSec;
LIGHTWEIGHT_CRITICALSECTION zipCritSec;

CString GetCurrentDownload()
{
  CLightweightCriticalSection cs( &dlTextCritSec );
  return currentDownload;
}

void FlushZipDict()
{
  for ( int x = 0; x < zipDict.NumItems(); x++ )
  {
    auto i = zipDict.GetByIndex( x );
    if ( i )
      mz_zip_reader_end( i );
  }
  zipDict.FreeAll();
}

mz_zip_archive* OpenZipFile( const CString& zipFile )
{
  if ( !zipDict.HasKey( zipFile ) )
  {
    mz_zip_archive* zip = new mz_zip_archive;
    memset( zip, 0, sizeof( mz_zip_archive ) );

    if ( !mz_zip_reader_init_file( zip, zipFile.GetPointer(), 0 ) )
    {
      LOG_ERR( "[GW2TacO] Failed to open zip archive %s", zipFile.GetPointer() );
      zipDict[ zipFile ] = nullptr;
      delete zip;
    }
    else
    {
      zipDict[ zipFile ] = zip;
    }
  }

  return zipDict[ zipFile ];
}

TBOOL FindSavedCategory( GW2TacticalCategory* t )
{
  if ( !t )
    return false;

  if ( t->keepSaveState || t->needsExport )
    return true;
  for ( TS32 x = 0; x < t->children.NumItems(); x++ )
    if ( FindSavedCategory( t->children[ x ] ) ) 
      return true;
  return false;
}

void ExportSavedCategories( CXMLNode* n, GW2TacticalCategory* t )
{
  if ( !FindSavedCategory( t ) || t->name.Length() == 0 )
    return;
  auto& nn = n->AddChild( "MarkerCategory" );
  nn.SetAttribute( "Name", t->name.GetPointer() );
  if ( t->name != t->displayName )
    nn.SetAttribute( "DisplayName", t->displayName.GetPointer() );

  if ( t->forceExport )
    nn.SetAttributeFromInteger( "forceExport", 1 );

  t->data.Write( &nn );
  for ( TS32 x = 0; x < t->children.NumItems(); x++ )
    ExportSavedCategories( &nn, t->children[ x ] );
}

void ExportPOI( CXMLNode* n, POI& p )
{
  CXMLNode* t = &n->AddChild( _T( "POI" ) );
  t->SetAttributeFromInteger( "MapID", p.mapID );
  t->SetAttributeFromFloat( "xpos", p.position.x );
  t->SetAttributeFromFloat( "ypos", p.position.y );
  t->SetAttributeFromFloat( "zpos", p.position.z );
  //if ( p.Name.Length() )
  //  t->SetAttribute( "text", p.Name.GetPointer() );
  if ( p.Type != -1 )
    t->SetAttribute( "type", GetStringFromMap( p.Type ).GetPointer() );
  t->SetAttribute( "GUID", CString::EncodeToBase64( (TU8*)&( p.guid ), sizeof( GUID ) ).GetPointer() );
  p.typeData.Write( t );
}

void ExportTrail( CXMLNode* n, GW2Trail& p )
{
  CXMLNode* t = &n->AddChild( _T( "Trail" ) );
  if ( p.Type.Length() )
    t->SetAttribute( "type", p.Type.GetPointer() );
  t->SetAttribute( "GUID", CString::EncodeToBase64( (TU8*)&( p.guid ), sizeof( GUID ) ).GetPointer() );
  p.typeData.Write( t );
}

void ExportPOIS( const CString& fileName, bool onlyCurrentMap )
{
  CXMLDocument d;
  CXMLNode& root = d.GetDocumentNode();
  root = root.AddChild( "OverlayData" );

  CategoryRoot.ClearExportNeeded();

  // build cat list
  for ( auto& POIs : POISet )
  {
    if ( onlyCurrentMap && POIs.first != mumbleLink.mapID )
      continue;

    for ( TS32 x = 0; x < POIs.second.NumItems(); x++ )
    {
      auto& p = POIs.second.GetByIndex( x );
      if ( !p.external && !p.routeMember )
        if ( p.category )
          p.category->SetExportNeeded();
    }
  }

  for ( auto& trails : trailSet )
  {
    if ( onlyCurrentMap && trails.first != mumbleLink.mapID )
      continue;

    for ( TS32 x = 0; x < trails.second.NumItems(); x++ )
    {
      auto& p = trails.second.GetByIndex( x );
      if ( !p->External )
        if ( p->category )
          p->category->SetExportNeeded();
    }
  }

  for ( TS32 x = 0; x < Routes.NumItems(); x++ )
  {
    if ( Routes[ x ].external )
      continue;

    for ( TS32 y = 0; y < Routes[ x ].route.NumItems(); y++ )
    {
      for ( auto& POIs : POISet )
      {
        if ( POIs.second.HasKey( Routes[ x ].route[ y ] ) && POIs.second[ Routes[ x ].route[ y ] ].category )
          POIs.second[ Routes[ x ].route[ y ] ].category->SetExportNeeded();
      }
    }
  }


  for ( TS32 x = 0; x < CategoryRoot.children.NumItems(); x++ )
    ExportSavedCategories( &root, CategoryRoot.children[ x ] );

  CXMLNode* n = &root.AddChild( "POIs" );
  // export pois

  for ( auto& POIs : POISet )
  {
    if ( onlyCurrentMap && POIs.first != mumbleLink.mapID )
      continue;

    for ( TS32 x = 0; x < POIs.second.NumItems(); x++ )
    {
      auto& p = POIs.second.GetByIndex( x );
      if ( !p.external && !p.routeMember )
        ExportPOI( n, p );
    }
  }

  for ( auto& trails : trailSet )
  {
    if ( onlyCurrentMap && trails.first != mumbleLink.mapID )
      continue;

    for ( TS32 x = 0; x < trails.second.NumItems(); x++ )
    {
      auto& p = trails.second.GetByIndex( x );
      if ( !p->External )
        ExportTrail( n, *p );
    }
  }

  for ( TS32 x = 0; x < Routes.NumItems(); x++ )
  {
    if ( Routes[ x ].external )
      continue;

    CXMLNode* t = &n->AddChild( _T( "Route" ) );
    t->SetAttribute( "Name", Routes[ x ].name.GetPointer() );
    t->SetAttributeFromInteger( "BackwardDirection", (TS32)Routes[ x ].backwards );

    for ( TS32 y = 0; y < Routes[ x ].route.NumItems(); y++ )
    {
      for ( auto& POIs : POISet )
      {
        if ( POIs.second.HasKey( Routes[ x ].route[ y ] ) )
          ExportPOI( t, POIs.second[ Routes[ x ].route[ y ] ] );
      }
    }
  }

  d.SaveToFile( fileName.GetPointer() );
}

GUID LoadGUID( CXMLNode& n )
{
  CString guidb64 = n.GetAttributeAsString( _T( "GUID" ) );

  TU8* Data = NULL;
  TS32 Size = 0;
  guidb64.DecodeBase64( Data, Size );

  GUID guid;

  if ( Size == sizeof( GUID ) )
    memcpy( &guid, Data, sizeof( GUID ) );
  else
    CoCreateGuid( &guid );

  SAFEDELETEA( Data );

  return guid;
}

void RecursiveImportPOIType( CXMLNode& root, GW2TacticalCategory* Root, CString currentCategory, MarkerTypeData& defaults, TBOOL KeepSaveState, const CString& zipFile )
{
  for ( TS32 x = 0; x < root.GetChildCount( "MarkerCategory" ); x++ )
  {
    auto& n = root.GetChild( "MarkerCategory", x );
    if ( !n.HasAttribute( "name" ) )
      continue;

    CString name = n.GetAttribute( "name" );

    for ( TS32 x = 0; x < (TS32)name.Length(); x++ )
      if ( !isalnum( name.GetPointer()[ x ] ) && name.GetPointer()[ x ] != '.' )
        name.GetPointer()[ x ] = '_';

    int forceExport = 0;
    if ( n.HasAttribute( "forceExport" ) )
      n.GetAttributeAsInteger( "forceExport", &forceExport );

    bool needsReExport = forceExport > 0;

    CString displayName;
    CString newCatName = currentCategory;

    CStringArray nameExploded = name.Explode( "." );

    GW2TacticalCategory* c = nullptr;

    for ( TS32 y = 0; y < nameExploded.NumItems(); y++ )
    {
      GW2TacticalCategory* Root2 = Root;

      if ( newCatName.Length() )
        newCatName += ".";
      newCatName += nameExploded[ y ];
      newCatName.ToLower();

      c = GetCategory( newCatName );

      if ( !c )
      {
        c = new GW2TacticalCategory();
        c->name = nameExploded[ y ];
        c->data = defaults;

        // don't propagate defaulttoggle
        c->data.bits.defaultToggleLoaded = false;
        c->data.bits.defaultToggle = false;

        // don't propagate save bits
        c->data.ClearSavedBits();

        CategoryMap[ newCatName ] = c;
        Root2->children += c;
        c->parent = Root2;
        Root2 = c;
        displayName = c->name;

        c->name.ToLower();
      }
    }

    if ( !c )
      continue;

    if ( !c->displayName.Length() )
      c->displayName = displayName;

    if ( n.HasAttribute( "DisplayName" ) )
    {
      displayName = n.GetAttribute( "DisplayName" );
      c->displayName = displayName;
      Localization::ProcessStringForUsedGlyphs( displayName );
    }

    if ( n.HasAttribute( "IsSeparator" ) )
    {
      int separator = 0;
      n.GetAttributeAsInteger( "IsSeparator", &separator );
      c->isOnlySeparator = separator;
    }

    c->data.Read( n, true );
    c->zipFile = AddStringToMap( zipFile );
    c->keepSaveState = needsReExport;
    c->forceExport = forceExport > 0;

    RecursiveImportPOIType( n, c, newCatName, c->data, KeepSaveState, zipFile );
  }
}

void ImportPOITypes()
{
  CXMLDocument d;
  if ( !d.LoadFromFile( "categorydata.xml" ) ) 
    return;

  if ( !d.GetDocumentNode().GetChildCount( "OverlayData" ) ) 
    return;
  CXMLNode root = d.GetDocumentNode().GetChild( "OverlayData" );

  CategoryMap.Flush();
  CategoryRoot.children.FreeArray();
  RecursiveImportPOIType( root, &CategoryRoot, CString(), MarkerTypeData(), false, CString() );
}

void ImportPOI( CWBApplication* App, CXMLNode& t, POI& p, const CString& zipFile )
{
  if ( t.HasAttribute( "MapID" ) ) t.GetAttributeAsInteger( "MapID", &p.mapID );
  if ( t.HasAttribute( "xpos" ) ) t.GetAttributeAsFloat( "xpos", &p.position.x );
  if ( t.HasAttribute( "ypos" ) ) t.GetAttributeAsFloat( "ypos", &p.position.y );
  if ( t.HasAttribute( "zpos" ) ) t.GetAttributeAsFloat( "zpos", &p.position.z );
  if ( t.HasAttribute( "icon" ) ) t.GetAttributeAsInteger( "icon", &p.icon );
  //if ( t.HasAttribute( "text" ) ) p.Name = t.GetAttributeAsString( "text" );
  if ( t.HasAttribute( "type" ) ) p.Type = AddStringToMap( t.GetAttributeAsString( "type" ) );

  if ( !t.HasAttribute( "GUID" ) )
    CoCreateGuid( &p.guid );
  else
    p.guid = LoadGUID( t );

  p.zipFile = AddStringToMap( zipFile );

  auto* td = GetCategory( GetStringFromMap( p.Type ) );
  if ( td )
    p.SetCategory( App, td );

  if ( td )
    td->markerCount++;

  while ( td )
  {
    td->containedMapIds.insert( p.mapID );
    td = td->parent;
  }

  p.typeData.Read( t, true );

  p.iconFile = p.typeData.iconFile;
  //p.icon = GetMapIcon( App, p.typeData.iconFile, zipFile );
  //p.iconSize = App->GetAtlas()->GetSize( p.icon );
}

TBOOL ImportTrail( CWBApplication* App, CXMLNode& t, GW2Trail& p, const CString& zipFile )
{
  p.zipFile = zipFile;

  if ( t.HasAttribute( "type" ) ) p.Type = t.GetAttributeAsString( "type" );

  if ( !t.HasAttribute( "GUID" ) )
    CoCreateGuid( &p.guid );
  else
    p.guid = LoadGUID( t );

  auto* td = GetCategory( p.Type );
  if ( td )
    p.SetCategory( App, td );

  if ( td )
    td->markerCount++;

  p.typeData.Read( t, true );

  auto result = p.Import( GetStringFromMap( p.typeData.trailData ), zipFile );

  if ( result )
  {
    while ( td )
    {
      td->containedMapIds.insert( p.map );
      td = td->parent;
    }
  }

  return result;
}

void ImportPOIDocument( CWBApplication* App, CXMLDocument& d, TBOOL External, const CString& zipFile )
{
  if ( !d.GetDocumentNode().GetChildCount( "OverlayData" ) ) 
    return;
  CXMLNode root = d.GetDocumentNode().GetChild( "OverlayData" );

  RecursiveImportPOIType( root, &CategoryRoot, CString(), MarkerTypeData(), !External, zipFile );

  if ( root.GetChildCount( "POIs" ) )
  {
    CXMLNode n = root.GetChild( "POIs" );

    //LOG_DBG("Reading %d pois", n.GetChildCount("POI"));

    if ( n.GetChildCount( "POI" ) > 0 )
    {
      CXMLNode t = n.GetChild( "POI", 0 );
      do
      {
        POI p;
        ImportPOI( App, t, p, zipFile );
        p.external = External;
        POISet[ p.mapID ][ p.guid ] = p;
      } while ( t.Next( t, "POI" ) );
    }

    for ( TS32 x = 0; x < n.GetChildCount( "Route" ); x++ )
    {
      CXMLNode rn = n.GetChild( "Route", x );
      POIRoute r;
      if ( rn.HasAttribute( "Name" ) ) r.name = rn.GetAttributeAsString( "Name" );
      TS32 b = false;
      if ( rn.HasAttribute( "BackwardDirection" ) ) rn.GetAttributeAsInteger( "BackwardDirection", &b );
      if ( rn.HasAttribute( "MapID" ) ) rn.GetAttributeAsInteger( "MapID", &r.MapID );
      r.backwards = b;
      r.external = External;
      if ( rn.HasAttribute( "resetposx" ) &&
           rn.HasAttribute( "resetposy" ) &&
           rn.HasAttribute( "resetposz" ) &&
           rn.HasAttribute( "resetrange" ) )
      {
        rn.GetAttributeAsFloat( "resetposx", &r.resetPos.x );
        rn.GetAttributeAsFloat( "resetposy", &r.resetPos.y );
        rn.GetAttributeAsFloat( "resetposz", &r.resetPos.z );
        rn.GetAttributeAsFloat( "resetrange", &r.resetRad );
        r.hasResetPos = true;
      }

      if ( rn.GetChildCount( "POI" ) > 0 )
      {
        CXMLNode t = rn.GetChild( "POI", 0 );
        do
        {
          POI p;
          ImportPOI( App, t, p, zipFile );
          p.external = External;
          p.routeMember = true;
          POISet[ p.mapID ][ p.guid ] = p;
          r.route += p.guid;
        } while ( t.Next( t, "POI" ) );
      }

      Routes.Add( r );
    }

    if ( n.GetChildCount( "Trail" ) > 0 )
    {
      CXMLNode t = n.GetChild( "Trail", 0 );
      do
      {
        GW2Trail* p = new GW2Trail();
        if ( ImportTrail( App, t, *p, zipFile ) )
        {
          p->External = External;
          trailSet[ p->map ][ p->guid ] = p;
        }
      } while ( t.Next( t, "Trail" ) );
    }
  }

  CategoryRoot.SetDefaultToggleValues();
  CategoryRoot.CalculateVisibilityCache();
}

void ImportPOIFile( CWBApplication* App, CString s, TBOOL External )
{
  CXMLDocument d;
  {
    CLightweightCriticalSection fileWrite( &zipCritSec );
    if ( !d.LoadFromFile( s.GetPointer() ) )
      return;
  }
  ImportPOIDocument( App, d, External, CString( "" ) );
}

void ImportPOIString( CWBApplication* App, const CString& data, const CString& zipFile )
{
  CXMLDocument d;
  if ( !d.LoadFromString( data ) ) 
    return;
  ImportPOIDocument( App, d, true, zipFile );
}

void ImportMarkerPack( CWBApplication* App, const CString& zipFile )
{
  mz_zip_archive* zip = OpenZipFile( zipFile );
  if ( !zip )
    return;

  for ( TU32 x = 0; x < mz_zip_reader_get_num_files( zip ); x++ )
  {
    mz_zip_archive_file_stat stat;
    if ( !mz_zip_reader_file_stat( zip, x, &stat ) )
      continue;

    if ( mz_zip_reader_is_file_a_directory( zip, x ) )
      continue;

    if ( stat.m_uncomp_size <= 0 )
      continue;

    CString fileName( stat.m_filename );
    fileName.ToLower();
    if ( fileName.Find( ".xml" ) != fileName.Length() - 4 )
      continue;

    TU8* data = new TU8[ (TS32)stat.m_uncomp_size ];

    if ( !mz_zip_reader_extract_to_mem( zip, x, data, (TS32)stat.m_uncomp_size, 0 ) )
    {
      delete[] data;
      continue;
    }

    CString doc( (TS8*)data, (TU32)stat.m_uncomp_size );
    ImportPOIString( App, doc, zipFile );

    delete[] data;
  }
}

void ImportPOIS( CWBApplication* App )
{
  FlushZipDict();
  ImportPOITypes();

  POISet.clear();
  Routes.Flush();
  trailSet.clear();

  {
    CFileList list;
    list.ExpandSearch( "*.xml", "POIs", true );
    for ( TS32 x = 0; x < list.Files.NumItems(); x++ )
      ImportPOIFile( App, list.Files[ x ].Path + list.Files[ x ].FileName, true );
  }

  {
    CFileList list;
    list.ExpandSearch( "*.zip", "POIs", true );
    for ( TS32 x = 0; x < list.Files.NumItems(); x++ )
      ImportMarkerPack( App, list.Files[ x ].Path + list.Files[ x ].FileName );
  }

  {
    CFileList list;
    list.ExpandSearch( "*.taco", "POIs", true );
    for ( TS32 x = 0; x < list.Files.NumItems(); x++ )
      ImportMarkerPack( App, list.Files[ x ].Path + list.Files[ x ].FileName );
  }

  ImportPOIFile( App, "poidata.xml", false );

  Config::LoadMarkerCategoryVisibilityInfo();

  //FlushZipDict();
}

void ImportPOIActivationData()
{
  CXMLDocument d;
  if ( !d.LoadFromFile( "activationdata.xml" ) ) 
    return;

  if ( !d.GetDocumentNode().GetChildCount( "OverlayData" ) ) return;
  CXMLNode root = d.GetDocumentNode().GetChild( "OverlayData" );

  if ( root.GetChildCount( "Activations" ) )
  {
    CXMLNode n = root.GetChild( "Activations" );

    for ( TS32 x = 0; x < n.GetChildCount( "POIActivation" ); x++ )
    {
      CXMLNode t = n.GetChild( "POIActivation", x );

      if ( !t.HasAttribute( "GUID" ) )
        continue;

      POIActivationData p;

      if ( t.HasAttribute( "lut1" ) ) t.GetAttributeAsInteger( "lut1", &( (TS32*)&p.lastUpdateTime )[ 0 ] );
      if ( t.HasAttribute( "lut2" ) ) t.GetAttributeAsInteger( "lut2", &( (TS32*)&p.lastUpdateTime )[ 1 ] );

      p.poiguid = LoadGUID( t );

      p.uniqueData = 0;
      if ( t.HasAttribute( "instance" ) ) t.GetAttributeAsInteger( "instance", &p.uniqueData );

      activationData[ POIActivationDataKey( p.poiguid, p.uniqueData ) ] = p;

      for ( auto& POIs : POISet )
      {
        if ( POIs.second.HasKey( p.poiguid ) )
          POIs.second[ p.poiguid ].lastUpdateTime = p.lastUpdateTime;
      }
    }
  }
}

void ExportPOIActivationData()
{
  CXMLDocument d;
  CXMLNode& root = d.GetDocumentNode();
  root = root.AddChild( "OverlayData" );

  CXMLNode* n = &root.AddChild( "Activations" );

  for ( TS32 x = 0; x < activationData.NumItems(); x++ )
  {
    auto& dat = activationData.GetByIndex( x );
    for ( auto& POIs : POISet )
    {
      if ( POIs.second.HasKey( dat.poiguid ) )
      {
        auto& poi = POIs.second[ dat.poiguid ];
        if ( poi.typeData.behavior == POIBehavior::AlwaysVisible )
          continue;

        //if (poi.behavior == (TS32)POIBehavior::ReappearOnDailyReset)
        //{
        //	continue;
        //}
      }
    }

    CXMLNode* t = &n->AddChild( _T( "POIActivation" ) );
    t->SetAttributeFromInteger( "lut1", ( (TS32*)&dat.lastUpdateTime )[ 0 ] );
    t->SetAttributeFromInteger( "lut2", ( (TS32*)&dat.lastUpdateTime )[ 1 ] );
    if ( dat.uniqueData )
      t->SetAttributeFromInteger( "instance", dat.uniqueData );
    t->SetAttribute( "GUID", CString::EncodeToBase64( (TU8*)&( dat.poiguid ), sizeof( GUID ) ).GetPointer() );
  }

  d.SaveToFile( "activationdata.xml" );
}

void FetchMarkerPacks()
{
  if ( !markerPacks.NumItems() )
  {
    CString result = FetchHTTPS( L"raw.githubusercontent.com", L"BoyC/GW2TacO/main/MarkerPacks.json" );
    if ( !result.Length() )
      return;

/*
    CStreamReaderMemory packDesc;
    packDesc.Open( "MarkerPacks.json" );
    CString result( (TCHAR*)packDesc.GetData(), (TU32)packDesc.GetLength() );
*/

    Object json;
    json.parse( result.GetPointer() );

    if ( json.has<Array>( "markerpacks" ) )
    {
      auto& values = json.get<Array>( "markerpacks" ).values();
      for ( auto v : values )
      {
        auto& data = v->get<Object>();
        bool incomplete = false;
        MarkerPack pack;

        if ( data.has<String>( "name" ) )
          pack.name = data.get<String>( "name" ).data();
        else
          incomplete = true;

        if ( data.has<String>( "id" ) )
          pack.id = data.get<String>( "id" ).data();
        else
          incomplete = true;

        if ( data.has<String>( "filename" ) )
          pack.fileName = data.get<String>( "filename" ).data();
        else
          incomplete = true;

        if ( data.has<String>( "forcedversion" ) )
        {
          pack.versionString = data.get<String>( "forcedversion" ).data();
          pack.versionCheckDone = true;
          pack.versionCheckOk = true;
          if ( !exists( ( "POIs/Online/" + pack.fileName ).GetPointer() ) )
            pack.outdated = true;
        }
        else
        {
          if ( data.has<String>( "versionurl" ) )
            pack.versionURL = data.get<String>( "versionurl" ).data();
          else
            incomplete = true;

          if ( data.has<String>( "versionsearchstring" ) )
            pack.versionSearchString = data.get<String>( "versionsearchstring" ).data();
          else
            incomplete = true;

          if ( data.has<String>( "versionterminator" ) )
            pack.versionTerminator = data.get<String>( "versionterminator" ).data();
          else
            incomplete = true;
        }

        if ( data.has<String>( "downloadurl" ) )
          pack.downloadURL = data.get<String>( "downloadurl" ).data();
        else
          incomplete = true;

        if ( data.has<String>( "backupurl" ) )
          pack.backupURL = data.get<String>( "backupurl" ).data();

        if ( data.has<String>( "backupversion" ) )
          pack.backupVersion = data.get<String>( "backupversion" ).data();

        if ( data.has<Boolean>( "enabledbydefault" ) )
          pack.defaultEnabled = data.get<Boolean>( "enabledbydefault" );
        else
          incomplete = true;

        if ( !incomplete )
        {
          CLightweightCriticalSection cs( &dlTextCritSec );
          markerPacks.Add( pack );
          CString versionUpdateValue = "MarkerPack_" + pack.id + "_autoupdate";
          Config::SetDefaultValue( versionUpdateValue.GetPointer(), pack.defaultEnabled );

        }
      }
    }

    for ( int x = 0; x < markerPacks.NumItems(); x++ )
      markerPacks[ x ].CheckVersion();
  }

  for ( int x = 0; x < markerPacks.NumItems(); x++ )
  {
    if ( !markerPacks[ x ].NeedsUpdate() )
      continue;

    markerPacks[ x ].UpdateFromWeb();
  }
}

bool MarkerPack::BackupVersionCheck()
{
  if ( !backupURL.Length() || !backupVersion.Length() )
    return false;

  versionCheckOk = true;
  CString versionConfigValue = "MarkerPack_" + id + "_version";
  versionString = backupVersion;

  bool upToDate = false;

  if ( Config::HasString( versionConfigValue.GetPointer() ) )
    if ( Config::GetString( versionConfigValue.GetPointer() ) == versionString )
      upToDate = true;

  if ( exists( ( "POIs/Online/" + fileName ).GetPointer() ) )
    upToDate = true; // assume previously downloaded file to be up to date

  outdated = !upToDate;
  downloadURL = backupURL;
  return true;
}

bool MarkerPack::CheckVersion()
{
  if ( versionCheckDone )
    return true;

  CString versionUpdateValue = "MarkerPack_" + id + "_autoupdate";
  if ( Config::GetValue( versionUpdateValue.GetPointer() ) == 0 )
    return false;

  versionCheckDone = true;
  versionCheckOk = false;

  CString data = FetchWeb( versionURL );
  if ( !data.Length() )
  {
    LOG_ERR( "[GW2TacO] Package %s version page download fail", id.GetPointer() );
    return BackupVersionCheck();
  }

  int versionStart = data.Find( versionSearchString );
  if ( versionStart < 0 )
  {
    LOG_ERR( "[GW2TacO] Package %s version string not found", id.GetPointer() );
    return BackupVersionCheck();
  }
  versionStart += versionSearchString.Length();

  int versionEnd = data.Find( versionTerminator, versionStart );
  if ( versionEnd < 0 )
  {
    LOG_ERR( "[GW2TacO] Package %s version string end not found", id.GetPointer() );
    return BackupVersionCheck();
  }

  CString currentVersion = data.Substring( versionStart, versionEnd - versionStart );
  CString versionConfigValue = "MarkerPack_" + id + "_version";

  bool upToDate = false;

  if ( Config::HasString( versionConfigValue.GetPointer() ) )
    if ( Config::GetString( versionConfigValue.GetPointer() ) == currentVersion )
      upToDate = true;

  if ( !exists( ( "POIs/Online/" + fileName ).GetPointer() ) )
    upToDate = false;

  outdated = !upToDate;
  downloadURL = CString::Format( downloadURL.GetPointer(), currentVersion.GetPointer() );
  versionString = currentVersion;

  versionCheckOk = true;
  return true;
}

bool MarkerPack::NeedsUpdate()
{
  if ( !versionCheckDone )
    CheckVersion();

  if ( !versionCheckOk || !outdated )
    return false;
  CString versionUpdateValue = "MarkerPack_" + id + "_autoupdate";
  if ( Config::GetValue( versionUpdateValue.GetPointer() ) == 0 )
    return false;

  return true;
}

bool MarkerPack::UpdateFromWeb()
{
  if ( failed )
    return false;

  {
    CLightweightCriticalSection cs( &dlTextCritSec );
    currentDownload = id;
  }

  beingDownloaded = true;
  CString markerPack = FetchWeb( downloadURL );
  if ( !markerPack.Length() )
  {
    if ( downloadURL != backupURL )
    {
      if ( backupVersion == Config::GetString( ( "MarkerPack_" + id + "_version" ).GetPointer() ) && exists( ( "POIs/Online/" + fileName ).GetPointer() ) )
      {
        beingDownloaded = false;

      }


      markerPack = FetchWeb( backupURL );
    }

    if ( !markerPack.Length() )
    {
      beingDownloaded = false;
      return false;
    }
  }

  mz_zip_archive zip;
  memset( &zip, 0, sizeof( zip ) );
  if ( !mz_zip_reader_init_mem( &zip, markerPack.GetPointer(), markerPack.Length(), 0 ) )
  {
    LOG_ERR( "[GW2TacO] Package %s doesn't seem to be a well formed zip file", downloadURL.GetPointer() );
    beingDownloaded = false;
    failed = true;
    {
      CLightweightCriticalSection cs( &dlTextCritSec );
      currentDownload = "";
    }
    return false;
  }
  mz_zip_reader_end( &zip );

  CreateDirectory( "POIs", 0 );
  CreateDirectory( "POIs/Online", 0 );

  FlushZipDict();

  CString fn = "POIs/Online/" + fileName;
  FILE* f = nullptr;

  {
    CLightweightCriticalSection fileWrite( &zipCritSec );

    fopen_s( &f, fn.GetPointer(), "wb" );
    if ( !f )
    {
      LOG_ERR( "[GW2TacO] Package %s cannot be saved", downloadURL.GetPointer() );
      beingDownloaded = false;
      {
        CLightweightCriticalSection cs( &dlTextCritSec );
        currentDownload = "";
      }
      failed = true;
      return false;
    }

    if ( fwrite( markerPack.GetPointer(), 1, markerPack.Length(), f ) != markerPack.Length() )
      failed = true;
    fclose( f );
  }

  if ( !failed )
  {
    CString versionConfigValue = "MarkerPack_" + id + "_version\0";
    Config::SetString( versionConfigValue.GetPointer(), versionString );

    markerPackQueue.Add( fn );
    beingDownloaded = false;
    downloadFinished = true;
    {
      CLightweightCriticalSection cs( &dlTextCritSec );
      currentDownload = "";
    }
  }
  outdated = false;
  return true;
}

void UpdateMarkerPackList()
{
  markerPackFetcherThread = std::thread( []()
                                         {
                                           while ( !App->IsDone() )
                                           {
                                             if ( Config::GetValue( "FetchMarkerPacks" ) )
                                             {
                                               markerPacksBeingFetched = true;
                                               FetchMarkerPacks();
                                               markerPacksBeingFetched = false;
                                             }
                                             Sleep( 1000 );
                                           }
                                         } );
}

void WaitForMarkerPackUpdate()
{
  App->SetDone( true );
  if ( markerPackFetcherThread.joinable() )
    markerPackFetcherThread.join();
}
