#pragma once
#include "RenderState.h"
#include "DX11Device.h"

#ifdef CORE_API_DX11

class CCoreDX11BlendState : public CCoreBlendState
{
	CCoreDX11Device *Device;
	ID3D11Device *Dev;
	ID3D11DeviceContext *Context;
	ID3D11BlendState *State;

public:
	CCoreDX11BlendState(CCoreDX11Device *Device);
	virtual ~CCoreDX11BlendState();

	virtual TBOOL Update();
	virtual TBOOL Apply();
	virtual void *GetHandle() { return State; }
};

class CCoreDX11DepthStencilState : public CCoreDepthStencilState
{
	CCoreDX11Device *Device;
	ID3D11Device *Dev;
	ID3D11DeviceContext *Context;
	ID3D11DepthStencilState *State;

public:
	CCoreDX11DepthStencilState(CCoreDX11Device *Device);
	virtual ~CCoreDX11DepthStencilState();

	virtual TBOOL Update();
	virtual TBOOL Apply();
	virtual void *GetHandle() { return State; }
};

class CCoreDX11RasterizerState : public CCoreRasterizerState
{
	CCoreDX11Device *Device;
	ID3D11Device *Dev;
	ID3D11DeviceContext *Context;
	ID3D11RasterizerState *State;

public:
	CCoreDX11RasterizerState(CCoreDX11Device *Device);
	virtual ~CCoreDX11RasterizerState();

	virtual TBOOL Update();
	virtual TBOOL Apply();
	virtual void *GetHandle() { return State; }
};

class CCoreDX11SamplerState : public CCoreSamplerState
{
	CCoreDX11Device *Device;
	ID3D11Device *Dev;
	ID3D11DeviceContext *Context;
	ID3D11SamplerState *State;

public:
	CCoreDX11SamplerState(CCoreDX11Device *Device);
	virtual ~CCoreDX11SamplerState();

	virtual TBOOL Update();
	virtual TBOOL Apply(CORESAMPLER Smp);
	virtual void *GetHandle() { return State; }
};

#endif