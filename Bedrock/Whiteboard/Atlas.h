#pragma once
#include "../CoRE2/Core2.h"

//cache size must be 2^x
#define ATLASCACHESIZE 32

typedef TS32 WBATLASHANDLE;

class CAtlasImage;

class CAtlasNode //stores a node for the rectpacker
{
  friend class CAtlas;
  CRect Area;
  CAtlasNode *Children[ 2 ];
  TBOOL Occupied;

  CAtlasImage *Image;

public:
  CAtlasNode();
  virtual ~CAtlasNode();
  CAtlasNode *AddNode( TS32 width, TS32 height );
  CRect &GetArea();
  CAtlasImage *GetImage();
};

class CAtlasImage //stores image data not currently in the atlas
{
  TU8 *Image;
  TS32 XRes, YRes;
  WBATLASHANDLE Handle;

  TBOOL Required;

public:

  CAtlasImage();
  CAtlasImage( TU8 *SourceImage, TS32 SrcXRes, TS32 SrcYRes, CRect &Source );
  virtual ~CAtlasImage();

  WBATLASHANDLE GetHandle();
  TU8 *GetImage();
  CSize GetSize() const;
  void TagRequired();
  void ClearRequired();
  TBOOL IsRequired();
};

struct CAtlasCacheElement
{
  WBATLASHANDLE Handle;
  CAtlasNode *Node;
};

class CAtlas
{
  friend class CWBDrawAPI;
  TS32 XRes, YRes;
  TU8 *Image;
  CCoreTexture2D *Atlas;

  TBOOL TextureUpdateNeeded;

  CAtlasCacheElement AtlasCache[ ATLASCACHESIZE ];

  CDictionary<WBATLASHANDLE, CAtlasNode *> Dictionary;
  CDictionaryEnumerable<WBATLASHANDLE, CAtlasImage*> ImageStorage;

  CAtlasNode *Root;

  CAtlasImage *WhitePixel;
  CPoint WhitePixelPosition; //recalculated on each optimization and reset

  TBOOL PackImage( CAtlasImage *img );

  void FlushCache();
  CAtlasNode *GetNodeCached( WBATLASHANDLE Handle );

  LIGHTWEIGHT_CRITICALSECTION critsec;

public:

  CAtlas( TS32 XSize, TS32 YSize );
  virtual ~CAtlas();

  TBOOL InitializeTexture( CCoreDevice *Device );
  TBOOL UpdateTexture();
  CCoreTexture2D *GetTexture();

  WBATLASHANDLE AddImage( TU8 *Image, TS32 XRes, TS32 YRes, CRect &SourceArea );
  void DeleteImage( WBATLASHANDLE h ); //doesn't immediately remove image from atlas

  TBOOL Optimize( TBOOL DebugMode = false );
  TBOOL Reset();

  CSize GetSize( WBATLASHANDLE h );
  TBOOL RequestImageUse( WBATLASHANDLE h, CRect &UV ); //returns false only if there was not enough room in the atlas to add the requested image
  CPoint GetWhitePixelUV();

  void ClearImageUsageflags();

  INLINE TS32 GetXRes() const { return XRes; }
  INLINE TS32 GetYRes() const { return YRes; }

  TBOOL Resize( CCoreDevice *Device, TS32 XSize, TS32 YSize );
};