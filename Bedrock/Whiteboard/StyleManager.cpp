#include "BasePCH.h"
#include "StyleManager.h"

CStyleManager::CStyleManager( void )
{
}


CStyleManager::~CStyleManager( void )
{
}

void CStyleManager::ParseDeclarations( const CString &s, CDictionaryEnumerable<CString, CString> &dRuleset )
{
  CStringArray propertiesArr = s.Explode( _T( ";" ) );
  for ( int p = 0; p < propertiesArr.NumItems(); p++ )
  {
    CStringArray prop = propertiesArr[ p ].Explode( _T( ":" ) );
    if ( prop.NumItems() != 2 ) continue;
    CString key = prop[ 0 ].Trimmed();
    CString value = prop[ 1 ].Trimmed();
    dRuleset[ key ] = value;
  }
}

bool CStyleManager::ParseStyleData( CString & style )
{
  int nPos = 0;
  while ( ( nPos = style.Find( _T( "/*" ) ) ) != -1 )
  {
    int nEnd = style.Find( _T( "*/" ), nPos );
    if ( nEnd == -1 )
      nEnd = style.Length();

    style = style.Substring( 0, nPos ) + style.Substring( nEnd + 2 );
  }

  CStringArray rules = style.Explode( _T( "}" ) );
  for ( int j = 0; j < rules.NumItems(); j++ )
  {
    CStringArray properties = rules[ j ].Explode( _T( "{" ) );
    if ( properties.NumItems() != 2 ) continue;

    CDictionaryEnumerable<CString, CString> dRuleset;
    ParseDeclarations( properties[ 1 ], dRuleset );

    CStringArray selectors = properties[ 0 ].Explode( _T( "," ) );
    for ( int s = 0; s < selectors.NumItems(); s++ )
    {
      CString selector = selectors[ s ].Trimmed();
      dRules[ selector ] += dRuleset;
    }
  }
  return true;
}

void CStyleManager::Reset()
{
  dRules.Flush();
}

void CStyleManager::CollectElementsBySimpleSelector( CWBItem * pItem, CArray<CWBItem*> & itemset, const CString& selector, bool bIncludeRoot )
{
  if ( bIncludeRoot && pItem->IsFitForSelector( selector ) )
    itemset.Add( pItem );

  for ( unsigned int i = 0; i < pItem->NumChildren(); i++ )
  {
    CollectElementsBySimpleSelector( (CWBItem *)pItem->GetChild( i ), itemset, selector, true );
  }
}

CArray<CWBItem*> CStyleManager::GetElementsBySelector( CWBItem * pRootItem, const CString& selector )
{
  CArray<CWBItem*> result;
  result.Add( (CWBItem *)pRootItem );
  CStringArray components = selector.Explode( _T( " " ) );
  for ( int i = 0; i < components.NumItems(); i++ )
  {
    if ( components[ i ].Length() < 1 ) continue;
    CArray<CWBItem*> narrowResult;
    for ( int j = 0; j < result.NumItems(); j++ )
    {
      CollectElementsBySimpleSelector( result[ j ], narrowResult, components[ i ], i == 0 );
    }
    result = narrowResult;
  }

  return result;
}

void CStyleManager::ApplyStyles( CWBItem * pRootItem )
{
  for ( int i = 0; i < dRules.NumItems(); i++ )
  {
    CString selector;
    CDictionaryEnumerable<CString, CString>& rules = dRules.GetByIndex( i, selector );

    CStringArray PseudoTags = selector.Explode( _T( ":" ) );
    CArray<CWBItem*> items = GetElementsBySelector( pRootItem, PseudoTags[ 0 ] );
    //LOG_DEBUG("[css] Itemcount for Selector %s = %d",selector.GetPointer(),items.NumItems());

    for ( int j = 0; j < items.NumItems(); j++ )
    {
      for ( int r = 0; r < rules.NumItems(); r++ )
      {
        CString rule;
        CString value = rules.GetByIndex( r, rule );

        //LOG_DEBUG("[css] rule %s being applied to item %s | selector: %s",rule.GetPointer(),items[j]->GetID().GetPointer(),selector.GetPointer());

        if ( !items[ j ]->ApplyStyle( rule, value, PseudoTags ) )
        {
          LOG_DBG( "[css] rule %s was not handled by item '%s' #%s .%s", rule.GetPointer(), items[ j ]->GetType().GetPointer(), items[ j ]->GetID().GetPointer(), items[ j ]->GetClassString().GetPointer() );
        }
      }
    }
  }
}

void CStyleManager::ApplyStylesFromDeclarations( CWBItem * pRootItem, const CString& sDeclarations )
{
  CDictionaryEnumerable<CString, CString> dRuleset;
  ParseDeclarations( sDeclarations, dRuleset );

  for ( int r = 0; r < dRuleset.NumItems(); r++ )
  {
    CString rule;
    CString value = dRuleset.GetByIndex( r, rule );
    CStringArray PseudoTags = rule.Explode( _T( ":" ) );

    if ( !pRootItem->ApplyStyle( rule, value, PseudoTags ) )
    {
      LOG_DBG( "[css] rule '%s' was not handled by item '%s' #%s .%s", rule.GetPointer(), pRootItem->GetType().GetPointer(), pRootItem->GetID().GetPointer(), pRootItem->GetClassString().GetPointer() );
    }
  }
}
