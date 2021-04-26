#pragma once
#include "DX11Device.h"
#include "Texture.h"

#ifdef CORE_API_DX11

class CCoreDX11Texture2D : public CCoreTexture2D
{
	ID3D11Device *Dev;
	ID3D11DeviceContext *DeviceContext;
	ID3D11Texture2D *TextureHandle;
	ID3D11ShaderResourceView *View;
	ID3D11RenderTargetView *RTView;
	ID3D11DepthStencilView *DepthView;

	TBOOL RenderTarget;

	virtual void Release();
	virtual TBOOL SetToSampler(const CORESAMPLER Sampler);

public:

	CCoreDX11Texture2D(CCoreDX11Device *Device);
	virtual ~CCoreDX11Texture2D();

	virtual void OnDeviceLost();
	virtual void OnDeviceReset();

	virtual TBOOL Create(const TS32 XRes, const TS32 YRes, const TU8 *Data, const TS8 BytesPerPixel = 4, const COREFORMAT Format = COREFMT_A8R8G8B8, const TBOOL RenderTarget = false);
	virtual TBOOL Create(const TU8 *Data, const TS32 Size);
	virtual TBOOL CreateDepthBuffer(const TS32 XRes, const TS32 YRes, const TS32 MSCount = 1);
	virtual TBOOL Lock(void **Result, TS32 &pitch);
	virtual TBOOL UnLock();

	virtual TBOOL Update(const TU8 *Data, const TS32 XRes, const TS32 YRes, const TS8 BytesPerPixel = 4);
	void SetTextureHandle(ID3D11Texture2D *Hnd) { TextureHandle = Hnd; }
	void SetView(ID3D11ShaderResourceView *v) { View = v; }

	ID3D11Texture2D *GetTextureHandle() { return TextureHandle; }
	ID3D11DepthStencilView *GetDepthView() { return DepthView; }
	ID3D11RenderTargetView *GetRenderTargetView() { return RTView; }
	ID3D11ShaderResourceView *GetShaderResourceView() { return View; }
  ID3D11DeviceContext *GetDeviceContext() { return DeviceContext; }

	virtual CCoreTexture2D *Copy();
	virtual void ExportToImage(CString &Filename, TBOOL ClearAlpha, EXPORTIMAGEFORMAT Format, bool degamma);
};

class CCoreDX11Texture3D : public CCoreTexture3D
{
	//LPDIRECT3DDEVICE9 Dev;

public:

	CCoreDX11Texture3D(CCoreDX11Device *Device);

};

class CCoreDX11TextureCube : public CCoreTextureCube
{
	//LPDIRECT3DDEVICE9 Dev;

public:

	CCoreDX11TextureCube(CCoreDX11Device *Device);

};

HRESULT SaveDDSTexture(_In_ ID3D11DeviceContext* pContext, _In_ ID3D11Resource* pSource, CStreamWriter &Writer);

#endif