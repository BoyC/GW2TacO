#include "TPTracker.h"
#include "GW2API.h"
#include "OverlayConfig.h"
#include "Language.h"
#include "Bedrock/UtilLib/PNGDecompressor.h"
#include "ThirdParty/BugSplat/inc/BugSplat.h"

using namespace jsonxx;

LIGHTWEIGHT_CRITICALSECTION itemCacheCritSec;

CDictionary<TS32, GW2ItemData> itemDataCache;

TBOOL HasGW2ItemData( TS32 itemID )
{
  CLightweightCriticalSection cs( &itemCacheCritSec );
  return itemDataCache.HasKey( itemID );
}

GW2ItemData GetGW2ItemData( TS32 itemID )
{
  CLightweightCriticalSection cs( &itemCacheCritSec );
  if ( itemDataCache.HasKey( itemID ) )
    return itemDataCache[ itemID ];
  return GW2ItemData();
}

void SetGW2ItemData( GW2ItemData& data )
{
  CLightweightCriticalSection cs( &itemCacheCritSec );
  itemDataCache[ data.itemID ] = data;
}

CString FetchHTTPS( LPCWSTR url, LPCWSTR path );

__inline CString ToGold( TS32 value )
{
  TS32 copper = value % 100;
  value /= 100;
  TS32 silver = value % 100;
  value /= 100;

  CString result;
  if ( value )
  {
    result += CString::Format( "%d", value ) + DICT( "gold" ) + " ";
    result += CString::Format( "%.2d", silver ) + DICT( "silver" ) + " ";
    result += CString::Format( "%.2d", copper ) + DICT( "copper" );
  }
  else
  {
    if ( silver )
    {
      result += CString::Format( "%d", silver ) + DICT( "silver" ) + " ";
      result += CString::Format( "%.2d", copper ) + DICT( "copper" );
    }
    else
      result = CString::Format( "%d", copper ) + DICT( "copper" );
  }

  return result;
}

void TPTracker::OnDraw( CWBDrawAPI* API )
{
  CWBFont* f = GetFont( GetState() );
  TS32 size = f->GetLineHeight();

  TS32 onlyShowOutbid = Config::GetValue( "TPTrackerOnlyShowOutbid" );
  TS32 nextSellOnly = Config::GetValue( "TPTrackerNextSellOnly" );

  GW2::APIKeyManager::Status status = GW2::apiKeyManager.DisplayStatusText( API, f );
  GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();

  if ( key && key->valid && ( globalTimer.GetTime() - lastFetchTime > 150000 || !lastFetchTime ) && !beingFetched && !fetchThread.joinable() )
  {
    beingFetched = true;
    fetchThread = std::thread( [this, key]()
                               {
                                 SetPerThreadCRTExceptionBehavior();
                                 CString qbuys = CString( "{\"buys\":" ) + key->QueryAPI( "v2/commerce/transactions/current/buys" );
                                 CString qsells = CString( "{\"sells\":" ) + key->QueryAPI( "v2/commerce/transactions/current/sells" );

                                 Object json;
                                 Object json2;
                                 json.parse( qbuys.GetPointer() );
                                 json2.parse( qsells.GetPointer() );

                                 CArray<TransactionItem> incoming;
                                 CArray<TransactionItem> outgoing;

                                 CArray<TS32> unknownItems;
                                 CArray<TS32> priceCheckList;

                                 if ( json.has<Array>( "buys" ) )
                                 {
                                   auto buyData = json.get<Array>( "buys" ).values();

                                   for ( unsigned int x = 0; x < buyData.size(); x++ )
                                   {
                                     if ( !buyData[ x ]->is<Object>() )
                                       continue;

                                     Object& item = buyData[ x ]->get<Object>();

                                     TransactionItem itemData;
                                     if ( !TPTracker::ParseTransaction( item, itemData ) )
                                       continue;
                                     incoming += itemData;

                                     if ( !itemDataCache.HasKey( itemData.itemID ) )
                                       unknownItems += itemData.itemID;

                                     priceCheckList.AddUnique( itemData.itemID );
                                   }
                                 }

                                 if ( json2.has<Array>( "sells" ) )
                                 {
                                   auto buyData = json2.get<Array>( "sells" ).values();

                                   for ( unsigned int x = 0; x < buyData.size(); x++ )
                                   {
                                     if ( !buyData[ x ]->is<Object>() )
                                       continue;

                                     Object& item = buyData[ x ]->get<Object>();

                                     TransactionItem itemData;
                                     if ( !TPTracker::ParseTransaction( item, itemData ) )
                                       continue;
                                     outgoing += itemData;

                                     if ( !itemDataCache.HasKey( itemData.itemID ) )
                                       unknownItems += itemData.itemID;

                                     priceCheckList.AddUnique( itemData.itemID );
                                   }
                                 }

                                 CString itemIds;

                                 if ( unknownItems.NumItems() )
                                 {
                                   for ( int x = 0; x < unknownItems.NumItems(); x++ )
                                     itemIds += CString::Format( "%d,", unknownItems[ x ] );

                                   //https://api.guildwars2.com/v2/items?ids=28445,12452
                                   CString items = CString( "{\"items\":" ) + key->QueryAPI( ( CString( "v2/items?ids=" ) + itemIds ).GetPointer() ) + "}";

                                   Object itemjson;
                                   itemjson.parse( items.GetPointer() );

                                   if ( itemjson.has<Array>( "items" ) )
                                   {
                                     auto items = itemjson.get<Array>( "items" ).values();

                                     for ( unsigned int x = 0; x < items.size(); x++ )
                                     {
                                       if ( !items[ x ]->is<Object>() )
                                         continue;

                                       Object& item = items[ x ]->get<Object>();

                                       GW2ItemData itemData;
                                       if ( !item.has<String>( "name" ) || !item.has<Number>( "id" ) )
                                         continue;
                                       itemData.name = CString( item.get<String>( "name" ).data() );
                                       itemData.itemID = TS32( item.get<Number>( "id" ) );
                                       if ( item.has<String>( "icon" ) )
                                       {
                                         CString iconFile = CString( item.get<String>( "icon" ).data() );
                                         if ( iconFile.Find( "https://render.guildwars2.com/" ) == 0 )
                                         {
                                           WCHAR wpath[ 4096 ];
                                           memset( wpath, 0, sizeof( wpath ) );
                                           iconFile.Substring( 29 ).WriteAsWideChar( wpath, 4096 );

                                           CString png = FetchHTTPS( L"render.guildwars2.com", wpath );

                                           TU8* imageData = nullptr;
                                           TS32 xres, yres;
                                           if ( DecompressPNG( (TU8*)png.GetPointer(), png.Length(), imageData, xres, yres ) )
                                           {
                                             ARGBtoABGR( imageData, xres, yres );
                                             CRect area = CRect( 0, 0, xres, yres );
                                             itemData.icon = GetApplication()->GetAtlas()->AddImage( imageData, xres, yres, area );
                                           }
                                         }
                                       }

                                       SetGW2ItemData( itemData );
                                     }
                                   }
                                 }

                                 {
                                   for ( int x = 0; x < priceCheckList.NumItems(); x++ )
                                     itemIds += CString::Format( "%d,", priceCheckList[ x ] );

                                   //https://api.guildwars2.com/v2/commerce/prices?ids=19684,19709
                                   CString items = CString( "{\"items\":" ) + key->QueryAPI( ( CString( "v2/commerce/prices?ids=" ) + itemIds ).GetPointer() ) + "}";

                                   Object itemjson;
                                   itemjson.parse( items.GetPointer() );

                                   if ( itemjson.has<Array>( "items" ) )
                                   {
                                     auto items = itemjson.get<Array>( "items" ).values();

                                     for ( unsigned int x = 0; x < items.size(); x++ )
                                     {
                                       if ( !items[ x ]->is<Object>() )
                                         continue;

                                       Object& item = items[ x ]->get<Object>();

                                       if ( !item.has<Number>( "id" ) || !item.has<Object>( "buys" ) || !item.has<Object>( "sells" ) )
                                         continue;

                                       TS32 id = TS32( item.get<Number>( "id" ) );
                                       if ( !HasGW2ItemData( id ) )
                                         continue;

                                       Object buys = item.get<Object>( "buys" );
                                       Object sells = item.get<Object>( "sells" );
                                       if ( !buys.has<Number>( "unit_price" ) || !sells.has<Number>( "unit_price" ) )
                                         continue;

                                       GW2ItemData itemData = GetGW2ItemData( id );
                                       itemData.buyPrice = TS32( buys.get<Number>( "unit_price" ) );
                                       itemData.sellPrice = TS32( sells.get<Number>( "unit_price" ) );
                                       SetGW2ItemData( itemData );
                                     }
                                   }
                                 }

                                 {
                                   CLightweightCriticalSection cs( &itemCacheCritSec );
                                   buys = incoming;
                                   sells = outgoing;
                                 }

                                 beingFetched = false;
                               } );
  }

  if ( !beingFetched && fetchThread.joinable() )
  {
    lastFetchTime = globalTimer.GetTime();
    fetchThread.join();
  }

  {
    CLightweightCriticalSection cs( &itemCacheCritSec );

    TS32 posy = 0;
    TS32 lh = f->GetLineHeight();

    if ( buys.NumItems() && Config::GetValue( "TPTrackerShowBuys" ) )
    {
      CArray<TS32> showedAlready;

      TS32 textPosy = posy;
      TS32 writtenCount = 0;

      posy += lh + 2;

      for ( int x = 0; x < buys.NumItems(); x++ )
      {
        if ( !HasGW2ItemData( buys[ x ].itemID ) )
          continue;

        auto& itemData = GetGW2ItemData( buys[ x ].itemID );
        TBOOL outbid = buys[ x ].price < itemData.buyPrice;

        if ( nextSellOnly && showedAlready.Find( itemData.itemID ) >= 0 )
          continue;

        if ( !onlyShowOutbid || outbid )
        {
          TS32 price = buys[ x ].price;
          if ( nextSellOnly )
          {
            for ( int y = x; y < buys.NumItems(); y++ )
              if ( buys[ y ].itemID == buys[ x ].itemID )
                price = max( buys[ y ].price, buys[ x ].price );
          }

          if ( itemData.icon )
            API->DrawAtlasElement( itemData.icon, CRect( lh, posy, lh * 2 + 5, posy + lh + 5 ), false, false, true, true, 0xffffffff );
          CString text = itemData.name + " " + ToGold( price );
          if ( buys[ x ].quantity > 1 )
            text = CString::Format( "%d ", buys[ x ].quantity ) + text;
          f->Write( API, text, CPoint( int( lh * 2.5 + 3 ), posy + 3 ), !outbid ? 0xffffffff : 0xffee6655 );
          writtenCount++;
          posy += lh + 6;
          if ( nextSellOnly )
            showedAlready.AddUnique( itemData.itemID );
        }
      }
      posy += 2;

      if ( writtenCount )
        f->Write( API, DICT( onlyShowOutbid ? "outbidbuys" : "buylist" ), CPoint( 0, textPosy ), 0xffffffff );
      else
        posy -= lh + 4;
    }

    if ( sells.NumItems() && Config::GetValue( "TPTrackerShowSells" ) )
    {
      CArray<TS32> showedAlready;

      TS32 textPosy = posy;
      TS32 writtenCount = 0;

      posy += lh + 2;

      for ( int x = 0; x < sells.NumItems(); x++ )
      {
        if ( !HasGW2ItemData( sells[ x ].itemID ) )
          continue;
        auto& itemData = GetGW2ItemData( sells[ x ].itemID );
        TBOOL outbid = sells[ x ].price > itemData.sellPrice;

        if ( nextSellOnly && showedAlready.Find( itemData.itemID ) >= 0 )
          continue;

        if ( !onlyShowOutbid || outbid )
        {
          TS32 price = sells[ x ].price;
          if ( nextSellOnly )
          {
            for ( int y = x; y < sells.NumItems(); y++ )
              if ( sells[ y ].itemID == sells[ x ].itemID )
                price = min( sells[ y ].price, sells[ x ].price );
          }

          if ( itemData.icon )
            API->DrawAtlasElement( itemData.icon, CRect( lh, posy, lh * 2 + 5, posy + lh + 5 ), false, false, true, true, 0xffffffff );
          CString text = itemData.name + " " + ToGold( price );
          if ( sells[ x ].quantity > 1 )
            text = CString::Format( "%d ", sells[ x ].quantity ) + text;
          f->Write( API, text, CPoint( int( lh * 2.5 + 3 ), posy + 3 ), !outbid ? 0xffffffff : 0xffee6655 );
          writtenCount++;
          posy += lh + 6;
          if ( nextSellOnly )
            showedAlready.AddUnique( itemData.itemID );
        }
      }

      if ( writtenCount )
        f->Write( API, DICT( onlyShowOutbid ? "outbidsells" : "selllist" ), CPoint( 0, textPosy ), 0xffffffff );

    }
  }

  DrawBorder( API );
}

TBOOL TPTracker::ParseTransaction( Object& object, TransactionItem& output )
{
  if ( !object.has<Number>( "id" ) || !object.has<Number>( "item_id" ) || !object.has<Number>( "price" ) || !object.has<Number>( "quantity" ) )
    return false;
  output.transactionID = TS32( object.get<Number>( "id" ) );
  output.itemID = TS32( object.get<Number>( "item_id" ) );
  output.price = TS32( object.get<Number>( "price" ) );
  output.quantity = TS32( object.get<Number>( "quantity" ) );
  return true;
}

TPTracker::TPTracker( CWBItem* Parent, CRect Position ) : CWBItem( Parent, Position )
{}

TPTracker::~TPTracker()
{
  if ( fetchThread.joinable() )
    fetchThread.join();
}

CWBItem* TPTracker::Factory( CWBItem* Root, CXMLNode& node, CRect& Pos )
{
  return new TPTracker( Root, Pos );
}

TBOOL TPTracker::IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType )
{
  return true;
}

