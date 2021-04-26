#pragma once
#include "GuiItem.h"

class CStyleManager
{
  CDictionaryEnumerable<CString, CDictionaryEnumerable<CString, CString>> dRules;

public:
  CStyleManager( void );
  ~CStyleManager( void );
  void Reset();
  static void ParseDeclarations( const CString & s, CDictionaryEnumerable<CString, CString> &dRuleset );
  bool ParseStyleData( CString & s );

  void ApplyStyles( CWBItem * pRootItem );

  static CArray<CWBItem*> GetElementsBySelector( CWBItem * pRootItem, const CString& selector );
  static void CollectElementsBySimpleSelector( CWBItem * pItem, CArray<CWBItem*> & itemset, const CString& selector, bool bIncludeRoot );
  static void ApplyStylesFromDeclarations( CWBItem * pRootItem, const CString& sDeclarations );
};

