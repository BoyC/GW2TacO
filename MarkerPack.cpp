#include "MarkerPack.h"
#include "Language.h"
#include "OverlayConfig.h"
CDictionary<CString, mz_zip_archive*> zipDict;

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
  if ( t->keepSaveState )
    return true;
  for ( TS32 x = 0; x < t->children.NumItems(); x++ )
    if ( FindSavedCategory( t->children[ x ] ) ) return true;
  return false;
}

void ExportSavedCategories( CXMLNode* n, GW2TacticalCategory* t )
{
  if ( !FindSavedCategory( t ) )
    return;
  auto nn = n->AddChild( "MarkerCategory" );
  nn.SetAttribute( "name", t->name.GetPointer() );
  if ( t->name != t->displayName )
    nn.SetAttribute( "DisplayName", t->displayName.GetPointer() );
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

void ExportPOIS()
{
  CXMLDocument d;
  CXMLNode& root = d.GetDocumentNode();
  root = root.AddChild( "OverlayData" );

  for ( TS32 x = 0; x < CategoryRoot.children.NumItems(); x++ )
    ExportSavedCategories( &root, CategoryRoot.children[ x ] );

  CXMLNode* n = &root.AddChild( "POIs" );

  for ( auto& POIs : POISet )
  {
    for ( TS32 x = 0; x < POIs.second.NumItems(); x++ )
    {
      auto& p = POIs.second.GetByIndex( x );
      if ( !p.external && !p.routeMember )
        ExportPOI( n, p );
    }
  }

  for ( auto& trails : trailSet )
  {
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

  d.SaveToFile( "poidata.xml" );
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

    c->data.Read( n, KeepSaveState );
    c->zipFile = AddStringToMap( zipFile );
    c->keepSaveState = KeepSaveState;

    RecursiveImportPOIType( n, c, newCatName, c->data, KeepSaveState, zipFile );
  }
}

void ImportPOITypes()
{
  CXMLDocument d;
  if ( !d.LoadFromFile( "categorydata.xml" ) ) return;

  if ( !d.GetDocumentNode().GetChildCount( "OverlayData" ) ) return;
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
  if ( !d.GetDocumentNode().GetChildCount( "OverlayData" ) ) return;
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
  if ( !d.LoadFromFile( s.GetPointer() ) ) return;
  ImportPOIDocument( App, d, External, CString( "" ) );
}

void ImportPOIString( CWBApplication* App, const CString& data, const CString& zipFile )
{
  CXMLDocument d;
  if ( !d.LoadFromString( data ) ) return;
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
    list.ExpandSearch( "*.xml", "POIs", false );
    for ( TS32 x = 0; x < list.Files.NumItems(); x++ )
      ImportPOIFile( App, list.Files[ x ].Path + list.Files[ x ].FileName, true );
  }

  {
    CFileList list;
    list.ExpandSearch( "*.zip", "POIs", false );
    for ( TS32 x = 0; x < list.Files.NumItems(); x++ )
      ImportMarkerPack( App, list.Files[ x ].Path + list.Files[ x ].FileName );
  }

  {
    CFileList list;
    list.ExpandSearch( "*.taco", "POIs", false );
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
  if ( !d.LoadFromFile( "activationdata.xml" ) ) return;

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

