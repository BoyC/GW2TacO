#pragma once
#include "Resource.h"
#include "Enums.h"

enum EXPORTIMAGEFORMAT
{
  CORE_PNG = 0,
  CORE_TGA = 1,
  CORE_BMP = 2,
};

class CCoreTexture : public CCoreResource
{
  friend class CCoreDevice;
  virtual TBOOL SetToSampler( const CORESAMPLER Sampler ) = 0;

public:

  INLINE CCoreTexture( CCoreDevice *Device ) : CCoreResource( Device ) {}
  INLINE virtual ~CCoreTexture()
  {
    //should remove this texture from the device render state here
  }

};

class CCoreTexture2D : public CCoreTexture
{

protected:

  TS32 XRes, YRes;
  COREFORMAT Format;

public:

  INLINE CCoreTexture2D( CCoreDevice *Device ) : CCoreTexture( Device ) { XRes = YRes = 0; Format = COREFMT_UNKNOWN; }

  virtual TBOOL Create( const TS32 XRes, const TS32 YRes, const TU8 *Data, const TS8 BytesPerPixel = 4, const COREFORMAT Format = COREFMT_A8R8G8B8, const TBOOL RenderTarget = false ) = 0;
  virtual TBOOL Create( const TU8 *Data, TS32 const Size ) = 0;
  virtual TBOOL CreateDepthBuffer( const TS32 XRes, const TS32 YRes, const TS32 MSCount ) = 0;
  virtual TBOOL Lock( void **Result, TS32 &pitch ) = 0;
  virtual TBOOL UnLock() = 0;

  virtual TBOOL Update( const TU8 *Data, const TS32 XRes, const TS32 YRes, const TS8 BytesPerPixel = 4 ) = 0;

  virtual TS32 GetXRes() { return XRes; }
  virtual TS32 GetYRes() { return YRes; }

  virtual CCoreTexture2D *Copy() = 0;

  virtual void ExportToImage( CString &Filename, TBOOL ClearAlpha, EXPORTIMAGEFORMAT Format, bool degamma ) = 0;
};

class CCoreTexture3D : public CCoreTexture
{
public:

  INLINE CCoreTexture3D( CCoreDevice *Device ) : CCoreTexture( Device ) {}
};

class CCoreTextureCube : public CCoreTexture
{
public:

  INLINE CCoreTextureCube( CCoreDevice *Device ) : CCoreTexture( Device ) {}
};