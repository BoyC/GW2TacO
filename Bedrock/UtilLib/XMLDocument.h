#pragma once

#include "../BaseLib/BaseLib.h"
#include "XMLNode.h"

class CXMLDocument
{
  //MSXML2::IXMLDOMDocument2 * pDoc;
  xml_document<> doc;
  CString memString;

public:
  CXMLDocument( void );
  ~CXMLDocument( void );

  TBOOL Allocate();
  TBOOL Cleanup();

  TBOOL LoadFromFile( TCHAR * );
  TBOOL LoadFromString( CString );
  TBOOL SaveToFile( TCHAR * );
  CString SaveToString();
  CXMLNode GetDocumentNode();

  //CXMLNode CreateNode(VARIANT type, BSTR Name, TS32 Level);
  //MSXML2::IXMLDOMDocument *GetDoc() { return pDoc; }
};
