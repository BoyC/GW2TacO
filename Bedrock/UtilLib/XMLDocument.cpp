#include "../UtilLib/RapidXML/rapidxml.hpp"
#include "../UtilLib/RapidXML/rapidxml_print.hpp"

#include "XMLDocument.h"

CXMLDocument::CXMLDocument(void)
{
	//HRESULT res = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	//if (res != S_OK && res!=S_FALSE)
	//	LOG_ERR("[XML] Error during CoInitializeEx");

	//pDoc = NULL;

	Allocate();
}

CXMLDocument::~CXMLDocument(void)
{
	Cleanup();
	CoUninitialize();
}

TBOOL CXMLDocument::LoadFromFile(TCHAR * szFileName)
{
  CStreamReaderMemory memStream;
  if ( !memStream.Open( szFileName ) )
    return false;

  memString = CString( (char*)memStream.GetData(), (TS32)memStream.GetLength() );

  try
  {
    doc.parse<0>( (char*)memString.GetPointer() );
  }
  catch ( const std::exception &e )
  {
    LOG_ERR( "[XML] Failed to load document: %s", e.what() );
    return false;
  }

	return true;
}

// throws HRESULT as a long on FAILED
#define HRCALL( a ) \
{ \
	HRESULT __hr; \
	__hr = (a); \
	if( FAILED( __hr ) ) \
	throw (long)__hr; \
}

// throws HRESULT as a long on FAILED or if !b is true
#define HRCALLRV( a, b ) \
{ \
	HRESULT __hr; \
	__hr = (a); \
	if( FAILED( __hr ) || !(b) ) \
	throw (long)__hr; \
}

TBOOL CXMLDocument::LoadFromString(CString s)
{
  memString = s;

  try
  {
    doc.parse<0>( (char*)memString.GetPointer() );
  }
  catch ( const std::exception &e )
  {
    LOG_ERR( "[XML] Failed to load document: %s", e.what() );
    return false;
  }

  return true;
}

CXMLNode CXMLDocument::GetDocumentNode()
{
  return CXMLNode( &doc, this, 0 );
}

#include <sstream>

CString CXMLDocument::SaveToString()
{
  std::stringstream ss;
  print<char>( ss, *doc.first_node() );
  std::string result_xml = ss.str();
  return CString( result_xml.data() );
}

TBOOL CXMLDocument::SaveToFile(TCHAR * sz)
{
	CString s = SaveToString();

	HANDLE h = CreateFile(sz, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
	if (h == INVALID_HANDLE_VALUE) return false;
	char * sz8 = new char[s.Length() * 3];
	s.WriteAsMultiByte(sz8, s.Length() * 3);
	DWORD b;
	WriteFile(h, sz8, (TS32)strlen(sz8), &b, NULL);
	CloseHandle(h);

	delete[] sz8;

	return true;
}

TBOOL CXMLDocument::Allocate()
{
	//if (pDoc) return true;

	//if (CoCreateInstance(MSXML2::CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, MSXML2::IID_IXMLDOMDocument, (void**)&pDoc) != S_OK)
	//{
	//	LOG_ERR("[XML] Error during CoCreateInstance!");
	//	return false;
	//}

	//if (!pDoc)
	//{
	//	LOG_ERR("[XML] Failed to create document object!");
	//	return false;
	//}

	//pDoc->put_async(VARIANT_FALSE);
	//pDoc->put_validateOnParse(VARIANT_FALSE);
	//pDoc->put_resolveExternals(VARIANT_FALSE);

	return true;
}

TBOOL CXMLDocument::Cleanup()
{
	//if (pDoc)
	//{
	//	pDoc->Release();
	//	pDoc = NULL;
	//}

	//CoFreeUnusedLibraries();
	return true;
}

//CXMLNode CXMLDocument::CreateNode(VARIANT type, BSTR Name, TS32 Level)
//{
//	//MSXML2::IXMLDOMNode * pNewNode = NULL;
//	//pDoc->createNode(_variant_t((TS32)MSXML2::NODE_TEXT), NULL, NULL, &pNewNode);
//	//return CXMLNode(pNewNode, this, Level);
//  return CXMLNode();
//}
