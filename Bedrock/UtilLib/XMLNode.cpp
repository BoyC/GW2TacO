#include "../UtilLib/RapidXML/rapidxml.hpp"

#include "XMLNode.h"
#include "XMLDocument.h"

TS32 GetStringHash( TCHAR* string )
{
  if ( !string )
    return 0;

  TS32 c;
  TCHAR *str = string;

  //djb2 hash
  TS32 Hash = 5381;
  while ( c = *str++ )
    Hash = ( ( Hash << 5 ) + Hash ) + c; // hash * 33 + c

  return Hash;
}

CXMLNode::CXMLNode()
{
	pNode = NULL;
	pDoc = NULL;
	nLevel = 0;
}

//CXMLNode::CXMLNode(MSXML2::IXMLDOMNode * p, CXMLDocument * d, TS32 l)
//{
//	pNode = p;
//	pDoc = d;
//	nLevel = l;
//}

CXMLNode::CXMLNode(const CXMLNode &Original)
{
	nLevel = Original.nLevel;
	pNode = Original.pNode;
	//if (pNode) pNode->AddRef();
	pDoc = Original.pDoc;
}

CXMLNode::CXMLNode( xml_node<char> *p, CXMLDocument *d, TS32 l)
{
  pNode = p;
  pDoc = d;
  nLevel = l;
}

CXMLNode CXMLNode::operator=( const CXMLNode Original )
{
	//if (pNode) pNode->Release();

	nLevel = Original.nLevel;
	pNode = Original.pNode;
	//if (pNode) pNode->AddRef();
	pDoc = Original.pDoc;
	return *this;
}


CXMLNode::~CXMLNode()
{
  stringStore.FreeArray();
	//if (pNode)
	//	pNode->Release();
}

TS32 CXMLNode::GetChildCount()
{
  if ( !pNode )
    return 0;

  if ( childCount != -1 )
    return childCount;

  auto node = pNode->first_node();
  if ( !node )
  {
    childCount = 0;
    return 0;
  }

  TS32 count = 1;

  while ( node = node->next_sibling() )
    count++;

  childCount = count;

  return count;
}

TS32 CXMLNode::GetChildCount( TCHAR * szNodeName )
{
  TS32 hash = GetStringHash( szNodeName );

  if ( childCounts.HasKey( hash ) )
    return childCounts[ hash ];

  auto node = pNode->first_node( szNodeName );
  if ( !node )
  {
    childCounts[ hash ] = 0;
    return 0;
  }

  TS32 count = 1;

  while ( node = node->next_sibling( szNodeName ) )
    count++;

  childCounts[ hash ] = count;

  return count;
}

CString CXMLNode::GetNodeName()
{
  if ( !pNode )
    return CString();
  return CString( pNode->name() );
}

CXMLNode CXMLNode::GetChild(TS32 n)
{
  if ( !pNode )
    return CXMLNode();

  auto node = pNode->first_node();
  if ( !node )
    return CXMLNode();

  if ( n == 0 )
    return CXMLNode( node, pDoc, nLevel + 1 );

  TS32 count = 1;

  while ( node = node->next_sibling() )
  {
    if ( n == count )
      return CXMLNode( node, pDoc, nLevel + 1 );
    count++;
  }

  return CXMLNode();
}

CXMLNode CXMLNode::GetChild(TCHAR * szNodeName)
{
  if ( !pNode )
    return CXMLNode();

  auto node = pNode->first_node( szNodeName );
  if ( !node )
    return CXMLNode();

  return CXMLNode( node, pDoc, nLevel + 1 );
}

CXMLNode CXMLNode::GetChild(TCHAR * szNodeName, TS32 n)
{
  if ( !pNode )
    return CXMLNode();

  auto node = pNode->first_node( szNodeName );
  if ( !node )
    return CXMLNode();

  if ( n == 0 )
    return CXMLNode( node, pDoc, nLevel + 1 );

  TS32 count = 1;

  while ( node = node->next_sibling( szNodeName ) )
  {
    if ( n == count )
      return CXMLNode( node, pDoc, nLevel + 1 );
    count++;
  }

  return CXMLNode();
}

TBOOL CXMLNode::Next( CXMLNode& out )
{
  if ( !pNode )
    return false;

  auto node = pNode->next_sibling();
  if ( !node )
    return false;

  out = CXMLNode( node, pDoc, nLevel );
  return true;
}

TBOOL CXMLNode::Next( CXMLNode& out, TCHAR* szNodeName )
{
  if ( !pNode )
    return false;

  auto node = pNode->next_sibling( szNodeName );
  if ( !node )
    return false;

  out = CXMLNode( node, pDoc, nLevel );
  return true;
}

void CXMLNode::GetText(TCHAR * szBuffer, TS32 nBufferSize)
{
	//WCHAR * bStr;
	//pNode->get_text(&bStr);
	//CString s = bStr;
	//_tcsncpy_s(szBuffer, nBufferSize, s.GetPointer(), _TRUNCATE);
	//SysFreeString(bStr);
  int x = 0;
}
CString CXMLNode::GetText()
{
	//WCHAR * bStr;
	//pNode->get_text(&bStr);
	//CString s = CString(bStr);
	//SysFreeString(bStr);
	//return s;
  return CString();
}

TBOOL CXMLNode::GetAttribute(TCHAR * szAttribute, TCHAR * szBuffer, TS32 nBufferSize)
{
  if ( !pNode )
    return false;

  auto attr = pNode->first_attribute( szAttribute );
  if ( !attr )
    return false;

  _tcsncpy_s( szBuffer, nBufferSize, attr->value(), _TRUNCATE );
  return true;
}


CString CXMLNode::GetAttribute(TCHAR * szAttribute)
{
  if ( !pNode )
    return CString();

  auto attr = pNode->first_attribute( szAttribute );
  if ( !attr )
    return CString();

  return CString( attr->value(), attr->value_size() );
}


CString CXMLNode::GetAttributeAsString(TCHAR * szAttribute)
{
  return GetAttribute( szAttribute );
}

TBOOL CXMLNode::HasAttribute(TCHAR * szAttribute)
{
  if ( !pNode )
    return false;

  auto attr = pNode->first_attribute( szAttribute );
  return attr != nullptr;
}

TS32 CXMLNode::IsValid()
{
	return pNode != NULL;
}

void CXMLNode::GetAttributeAsInteger(TCHAR * szAttribute, TS32 * pnValue)
{
	TCHAR s[20];
	ZeroMemory(s, 20);
	GetAttribute(szAttribute, s, 20);
	_stscanf_s(s, _T("%d"), pnValue);
}

void CXMLNode::GetAttributeAsFloat(TCHAR * szAttribute, TF32 * pfValue)
{
	TCHAR s[20];
	ZeroMemory(s, 20);
	GetAttribute(szAttribute, s, 20);
	_stscanf_s(s, _T("%g"), pfValue);
}

//void CXMLNode::FlushNode() 
//{
//	MSXML2::IXMLDOMNode * pChild=NULL;
//	pNode->get_firstChild(&pChild);
//	while (pChild) 
//	{
//		MSXML2::IXMLDOMNode * pOldChild=NULL;
//		MSXML2::IXMLDOMNode * pRemoveChild=pChild;
//		//pChild->get_nextSibling(&pChild);
//		MSXML2::IXMLDOMNode * pSibling = NULL;
//		pChild->get_nextSibling(&pSibling);
//		pChild->Release();
//		pChild = pSibling;
//
//		pNode->removeChild(pRemoveChild,&pOldChild);
//		if (pOldChild) pOldChild->Release();
//	}
//}

CXMLNode& CXMLNode::AddChild(TCHAR * szNodeName, TBOOL PostEnter)
{
  TCHAR *tc = szNodeName;
  if ( szNodeName )
  {
    while ( *tc )
    {
      if ( *tc == ' ' )
      {
        *tc = '_';
      }
      tc++;
    };
  }

  if ( !pNode || !pDoc )
  {
    children.Add( new CXMLNode() );
    return *children.Last();
  }

  auto node = pNode->document()->allocate_node( node_type::node_element, szNodeName );

  pNode->append_node( node );

  children.Add( new CXMLNode( node, pDoc, nLevel + 1 ) );
  return *children.Last();
}

void CXMLNode::SetAttribute(TCHAR * szAttributeName, const TCHAR * szValue)
{
  if ( !pNode || !pDoc )
    return;

  CString* strVal = new CString( szValue );
  stringStore += strVal;

  CString* strNam = new CString( szAttributeName );
  stringStore += strNam;

  auto attr = pNode->first_attribute( strNam->GetPointer() );

  if ( !attr )
  {
    attr = pNode->document()->allocate_attribute( strNam->GetPointer(), strVal->GetPointer() );
    pNode->append_attribute( attr );
    return;
  }

  attr->value( strVal->GetPointer() );
}

void CXMLNode::SetAttributeFromInteger(TCHAR * szAttributeName, TS32 nValue)
{
	TCHAR s[64];
  memset( s, 0, sizeof( TCHAR ) * 64 );
	_sntprintf_s(s, 64, _T("%d"), nValue);
	SetAttribute(szAttributeName, s);
}

void CXMLNode::SetAttributeFromFloat(TCHAR * szAttributeName, TF32 fValue)
{
	TCHAR s[64];
	_sntprintf_s(s, 64, _T("%g"), fValue);
	SetAttribute(szAttributeName, s);
}

void CXMLNode::SetText(const TCHAR * sz)
{
  if ( !pNode )
    return;

  value = CString( sz );

  pNode->value( value.GetPointer() );

	//IGNOREFREEERRORS(true);
	//{
	//	_bstr_t bs = sz;
	//	pNode->put_text(bs);
	//}
	//IGNOREFREEERRORS(false);
}

void CXMLNode::SetText(CString &s)
{
	SetText(s.GetPointer());
}

void CXMLNode::SetInt(TS32 Int)
{
	TCHAR s[64];
	_sntprintf_s(s, 64, _T("%d"), Int);
	SetText(s);
}

void CXMLNode::SetFloat(TF32 Float)
{
	TCHAR s[64];
	_sntprintf_s(s, 64, _T("%g"), Float);
	SetText(s);
}

TBOOL CXMLNode::GetValue(TS32 &Int)
{
	TCHAR s[20];
	ZeroMemory(s, 20);
	GetText(s, 20);
	return _stscanf_s(s, _T("%d"), &Int) == 1;
}

TBOOL CXMLNode::GetValue(TBOOL &Int)
{
	TCHAR s[20];
	ZeroMemory(s, 20);
	GetText(s, 20);
	TS32 x = 0;
	TS32 r = _stscanf_s(s, _T("%d"), &x);
	if (r == 1)
		Int = x != 0;
	return r == 1;
}

TBOOL CXMLNode::GetValue(TF32 &Float)
{
	TCHAR s[20];
	ZeroMemory(s, 20);
	GetText(s, 20);
	return _stscanf_s(s, _T("%g"), &Float) == 1;
}

