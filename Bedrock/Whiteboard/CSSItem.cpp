#include "BasePCH.h"
#include "cssitem.h"

IWBCSS::IWBCSS()
{

}

IWBCSS::~IWBCSS()
{

}

void IWBCSS::AddClass( const CString& s )
{
  aClasses.AddUnique( s );
}

void IWBCSS::RemoveClass( const CString& s )
{
  int n = aClasses.Find( s );
  if ( n != -1 ) aClasses.DeleteByIndex( n );
}

void IWBCSS::ToggleClass( const CString& s )
{
  if ( HasClass( s ) )
    RemoveClass( s );
  else
    AddClass( s );
}

bool IWBCSS::HasClass( const CString& s )
{
  return aClasses.Find( s ) != -1;
}

void IWBCSS::SetID( const CString& s )
{
  sID = s;
}

CString& IWBCSS::GetID()
{
  return sID;
}

bool IWBCSS::IsFitForSelector( const CString& selector )
{
  //unsigned int nStart = 0;
  //do
  //{
  //  CString s = selector.Substring( nStart );
  //  for ( unsigned int i = 1; i < s.Length(); i++ )
  //  {
  //    if ( selector[ i ] == '#' || selector[ i ] == '.' )
  //    {
  //      s = s.Substring( 0, i );
  //      break;
  //    }
  //  }

    if ( selector[ 0 ] == '#' )
    {
      if ( selector.Substring( 1 ) != GetID() )
        return false;
    }
    else if ( selector[ 0 ] == '.' )
    {
      if ( !HasClass( selector.Substring( 1 ) ) )
        return false;
    }
    else if ( selector[ 0 ] == '*' )
    {
      return true;
    }
    else
    {
      //if (s != GetType())
      if ( !InstanceOf( selector ) )
        return false;
    }
  //  nStart += s.Length();
  //} while ( nStart < selector.Length() );
  return true;
}

TBOOL IWBCSS::ApplyStyle( CString & prop, CString & value, CStringArray &Pseudo )
{
  return false;
}

CString IWBCSS::GetClassString()
{
  return aClasses.Implode( _T( " " ) );
}
