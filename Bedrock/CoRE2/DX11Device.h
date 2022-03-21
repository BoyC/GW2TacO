#pragma once
#include "Core2_Config.h"
#include "../BaseLib/BaseLib.h"
#include "Device.h"

#include "DX11Enums.h"
#include <dxgi1_3.h>

#ifdef CORE_API_DX11

class CCoreDX11Device : public CCoreDevice
{
	IDXGISwapChain1 *SwapChain;
	ID3D11Device *Device;
	ID3D11DeviceContext *DeviceContext;

	ID3D11RenderTargetView *BackBufferView;
	ID3D11Texture2D* DepthBuffer;
	ID3D11DepthStencilView *DepthBufferView;

	ID3D11BlendState *CurrentBlendState;
	ID3D11DepthStencilState *CurrentDepthStencilState;
	ID3D11RasterizerState *CurrentRasterizerState;

  ID3D11Query *OcclusionQuery = nullptr;
  HANDLE swapChainRetraceObject = 0;


	virtual void ResetPrivateResources();
	virtual TBOOL InitAPI(const TU32 hWnd, const TBOOL FullScreen, const TS32 XRes, const TS32 YRes, const TS32 AALevel = 0, const TS32 RefreshRate = 60);
	virtual TBOOL ApplyRenderState(const CORESAMPLER Sampler, const CORERENDERSTATE RenderState, const CORERENDERSTATEVALUE Value);
	virtual TBOOL SetNoVertexBuffer();
	virtual TBOOL CommitRenderStates();

	virtual TBOOL CreateBackBuffer(TS32 XRes, TS32 YRes);
	virtual TBOOL CreateDepthBuffer(TS32 XRes, TS32 YRes);

	TBOOL CreateClassicSwapChain( const TU32 hWnd, const TBOOL FullScreen, const TS32 XRes, const TS32 YRes, const TS32 AALevel, const TS32 RefreshRate );
	TBOOL CreateDirectCompositionSwapchain( const TU32 hWnd, const TBOOL FullScreen, const TS32 XRes, const TS32 YRes, const TS32 AALevel, const TS32 RefreshRate );

public:

	CCoreDX11Device();
	virtual ~CCoreDX11Device();
	INLINE ID3D11Device *GetDevice() { return Device; }
	INLINE ID3D11DeviceContext *GetDeviceContext() { return DeviceContext; }
	virtual COREDEVICEAPI GetAPIType() { return COREAPI_DX11; }

	//this initializer will change to accommodate multiple platforms at once once we get to that point:
	TBOOL Initialize(CCoreWindowHandler *Window, const TS32 AALevel = 0);

	virtual TBOOL DeviceOk();
	virtual TBOOL IsWindowed();
	virtual void Resize(const TS32 xr, const TS32 yr);
	virtual void SetFullScreenMode(const TBOOL FullScreen, const TS32 xr, const TS32 yr);

	ID3D11Texture2D *GetBackBuffer();

	//////////////////////////////////////////////////////////////////////////
	// texture functions

	virtual CCoreTexture2D *CreateTexture2D(const TS32 XRes, const TS32 YRes, const TU8 *Data, const TS8 BytesPerPixel = 4, const COREFORMAT Format = COREFMT_A8R8G8B8, const TBOOL RenderTarget = false);
	virtual CCoreTexture2D *CreateTexture2D(const TU8 *Data, const TS32 Size);
	virtual CCoreTexture2D *CopyTexture(CCoreTexture2D *Texture);

	//////////////////////////////////////////////////////////////////////////
	// vertexbuffer functions

	virtual CCoreVertexBuffer *CreateVertexBuffer(const TU8 *Data, const TS32 Size);
	virtual CCoreVertexBuffer *CreateVertexBufferDynamic(const TS32 Size);

	//////////////////////////////////////////////////////////////////////////
	// indexbuffer functions

	virtual CCoreIndexBuffer *CreateIndexBuffer(const TS32 IndexCount, const TS32 IndexSize = 2);

	//////////////////////////////////////////////////////////////////////////
	// vertexformat functions

	virtual CCoreVertexFormat *CreateVertexFormat(const CArray<COREVERTEXATTRIBUTE> &Attributes, CCoreVertexShader *vs = NULL);

	//////////////////////////////////////////////////////////////////////////
	// shader functions

	virtual CCoreVertexShader *CreateVertexShader(LPCSTR Code, TS32 CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString *Err = NULL);
	virtual CCorePixelShader *CreatePixelShader(LPCSTR Code, TS32 CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString *Err = NULL);
  virtual CCoreVertexShader *CreateVertexShaderFromBlob( TU8 *Code, TS32 CodeSize );
  virtual CCorePixelShader *CreatePixelShaderFromBlob( TU8 *Code, TS32 CodeSize );
  virtual CCoreGeometryShader *CreateGeometryShader( LPCSTR Code, TS32 CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString *Err = NULL );
	virtual CCoreDomainShader *CreateDomainShader(LPCSTR Code, TS32 CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString *Err = NULL);
	virtual CCoreHullShader *CreateHullShader(LPCSTR Code, TS32 CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString *Err = NULL);
	virtual CCoreComputeShader *CreateComputeShader(LPCSTR Code, TS32 CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString *Err = NULL);
	virtual CCoreVertexShader *CreateVertexShader();
	virtual CCorePixelShader *CreatePixelShader();
	virtual CCoreGeometryShader *CreateGeometryShader();
	virtual CCoreDomainShader *CreateDomainShader();
	virtual CCoreHullShader *CreateHullShader();
	virtual CCoreComputeShader *CreateComputeShader();
	virtual void SetShaderConstants(TS32 Slot, TS32 Count, CCoreConstantBuffer **Buffers);
	virtual CCoreConstantBuffer *CreateConstantBuffer();

	virtual CCoreBlendState *CreateBlendState();
	virtual CCoreDepthStencilState *CreateDepthStencilState();
	virtual CCoreRasterizerState *CreateRasterizerState();
	virtual CCoreSamplerState *CreateSamplerState();

	virtual TBOOL SetRenderTarget(CCoreTexture2D *RT);

	//////////////////////////////////////////////////////////////////////////
	// display functions

	virtual TBOOL BeginScene();
	virtual TBOOL EndScene();
	virtual TBOOL Clear(const TBOOL clearPixels = true, const TBOOL clearDepth = true, const CColor &Color = CColor((TU32)0), const TF32 Depth = 1, const TS32 Stencil = 0);
	virtual TBOOL Flip(TBOOL Vsync = true);
	virtual TBOOL DrawIndexedTriangles(TS32 Count, TS32 NumVertices);
	virtual TBOOL DrawTriangles(TS32 Count);
	virtual TBOOL DrawIndexedLines(TS32 Count, TS32 NumVertices);
	virtual TBOOL DrawLines(TS32 Count);

	//////////////////////////////////////////////////////////////////////////
	// renderstate functions

	virtual TBOOL SetViewport(CRect Viewport);
	ID3D11BlendState *GetCurrentBlendState();
	void SetCurrentBlendState(ID3D11BlendState *bs);
	ID3D11RasterizerState *GetCurrentRasterizerState();
	void SetCurrentRasterizerState(ID3D11RasterizerState *bs);
	ID3D11DepthStencilState *GetCurrentDepthStencilState();
	void SetCurrentDepthStencilState(ID3D11DepthStencilState *bs);

	virtual void ForceStateReset();

	virtual void TakeScreenShot(CString Filename);

	virtual void InitializeDebugAPI();
	virtual void CaptureCurrentFrame();

  //////////////////////////////////////////////////////////////////////////
  // queries

  virtual void BeginOcclusionQuery();
  virtual TBOOL EndOcclusionQuery();
  
  //////////////////////////////////////////////////////////////////////////
	// dx11 specific functions

	ID3D11DepthStencilView *GetDepthBufferView() { return DepthBufferView; }

	virtual void WaitRetrace();
};

#endif