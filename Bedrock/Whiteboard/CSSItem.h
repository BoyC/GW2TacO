#pragma once

#include "../BaseLib/BaseLib.h"

class IWBCSS
{
  CStringArray aClasses;
  CString sID;
public:

  IWBCSS();
  virtual ~IWBCSS();

  void SetID( const CString& s );
  CString& GetID();
  void AddClass( const CString& s );
  void RemoveClass( const CString& s );
  void ToggleClass( const CString& s );
  bool HasClass( const CString& s );
  bool IsFitForSelector( const CString& selector );

  virtual const CString &GetType() const = 0;
  static const CString &GetClassName()
  {
    static const CString type = _T( "IWBCSS" );
    return type;
  }
  virtual TBOOL InstanceOf( const CString &name ) const = 0;

  virtual TBOOL ApplyStyle( CString & prop, CString & value, CStringArray &Pseudo );
  CString GetClassString();
};