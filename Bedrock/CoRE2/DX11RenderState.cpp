#include "BasePCH.h"
#include "DX11RenderState.h"

#ifdef CORE_API_DX11

CCoreDX11BlendState::CCoreDX11BlendState(CCoreDX11Device *d) : CCoreBlendState(d)
{
	Device = d;
	Dev = Device->GetDevice();
	Context = Device->GetDeviceContext();
	State = NULL;
}

CCoreDX11BlendState::~CCoreDX11BlendState()
{
	if (State) State->Release();
}

TBOOL CCoreDX11BlendState::Update()
{
	if (!Dirty) return true;
	if (State) State->Release();
	State = NULL;

	D3D11_BLEND_DESC desc;
	desc.AlphaToCoverageEnable = AlphaToCoverage;
	desc.IndependentBlendEnable = IndependentBlend;
	for (TS32 x = 0; x < 8; x++)
	{
		desc.RenderTarget[x].BlendEnable = RenderTargetBlendStates[x].BlendEnable;
		desc.RenderTarget[x].SrcBlend = DX11BlendFactors[RenderTargetBlendStates[x].SrcBlend];
		desc.RenderTarget[x].DestBlend = DX11BlendFactors[RenderTargetBlendStates[x].DestBlend];
		desc.RenderTarget[x].BlendOp = DX11BlendOps[RenderTargetBlendStates[x].BlendOp];
		desc.RenderTarget[x].SrcBlendAlpha = DX11BlendFactors[RenderTargetBlendStates[x].SrcBlendAlpha];
		desc.RenderTarget[x].DestBlendAlpha = DX11BlendFactors[RenderTargetBlendStates[x].DestBlendAlpha];
		desc.RenderTarget[x].BlendOpAlpha = DX11BlendOps[RenderTargetBlendStates[x].BlendOpAlpha];
		desc.RenderTarget[x].RenderTargetWriteMask = RenderTargetBlendStates[x].RenderTargetWriteMask;
	}

	Dirty = false;

	HRESULT res = Dev->CreateBlendState(&desc, &State);
	if (res != S_OK)
	{
		_com_error err(res);
		LOG(LOG_ERROR, _T("[core] DirectX11 Blend state creation failed (%s)"), err.ErrorMessage());
		return false;
	}

	return true;
}

TBOOL CCoreDX11BlendState::Apply()
{
	Update();
	if (Device->GetCurrentBlendState() != State)
	{
		Context->OMSetBlendState(State, NULL, 0xffffffff);
		Device->SetCurrentBlendState(State);
	}
	return true;
}

CCoreDX11DepthStencilState::CCoreDX11DepthStencilState(CCoreDX11Device *d) : CCoreDepthStencilState(d)
{
	Device = d;
	Dev = Device->GetDevice();
	Context = Device->GetDeviceContext();
	State = NULL;
}

CCoreDX11DepthStencilState::~CCoreDX11DepthStencilState()
{
	if (State) State->Release();
}

TBOOL CCoreDX11DepthStencilState::Update()
{
	if (!Dirty) return true;
	if (State) State->Release();
	State = NULL;

	D3D11_DEPTH_STENCIL_DESC desc;
	memset(&desc, 0, sizeof(D3D11_DEPTH_STENCIL_DESC));

	desc.DepthEnable = DepthEnable;
	desc.DepthWriteMask = ZWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = DX11ComparisonFunctions[DepthFunc];
	desc.StencilEnable = false;

	Dirty = false;

	HRESULT res = Dev->CreateDepthStencilState(&desc, &State);
	if (res != S_OK)
	{
		_com_error err(res);
		LOG(LOG_ERROR, _T("[core] DirectX11 Depth Stencil state creation failed (%s)"), err.ErrorMessage());
		return false;
	}

	return true;
}

TBOOL CCoreDX11DepthStencilState::Apply()
{
	Update();
	if (Device->GetCurrentDepthStencilState() != State)
	{
		Context->OMSetDepthStencilState(State, 0);
		Device->SetCurrentDepthStencilState(State);
	}
	return true;
}

CCoreDX11RasterizerState::CCoreDX11RasterizerState(CCoreDX11Device *d) : CCoreRasterizerState(d)
{
	Device = d;
	Dev = Device->GetDevice();
	Context = Device->GetDeviceContext();
	State = NULL;
}

CCoreDX11RasterizerState::~CCoreDX11RasterizerState()
{
	if (State) State->Release();
}

TBOOL CCoreDX11RasterizerState::Update()
{
	if (!Dirty) return true;
	if (State) State->Release();
	State = NULL;

	D3D11_RASTERIZER_DESC desc;
	desc.AntialiasedLineEnable = AntialiasedLineEnable;
	desc.CullMode = DX11CullModes[CullMode];
	desc.DepthBias = DepthBias;
	desc.DepthBiasClamp = DepthBiasClamp;
	desc.FillMode = DX11FillModes[FillMode];
	desc.FrontCounterClockwise = FrontCounterClockwise;
	desc.MultisampleEnable = MultisampleEnable;
	desc.ScissorEnable = ScissorEnable;
	desc.SlopeScaledDepthBias = SlopeScaledDepthBias;

	Dirty = false;

	HRESULT res = Dev->CreateRasterizerState(&desc, &State);
	if (res != S_OK)
	{
		_com_error err(res);
		LOG(LOG_ERROR, _T("[core] DirectX11 Rasterizer state creation failed (%s)"), err.ErrorMessage());
		return false;
	}

	return true;
}

TBOOL CCoreDX11RasterizerState::Apply()
{
	Update();
	if (Device->GetCurrentRasterizerState() != State)
	{
		Context->RSSetState(State);
		Device->SetCurrentRasterizerState(State);
	}
	return true;
}

CCoreDX11SamplerState::CCoreDX11SamplerState(CCoreDX11Device *d) : CCoreSamplerState(d)
{
	Device = d;
	Dev = Device->GetDevice();
	Context = Device->GetDeviceContext();
	State = NULL;
}

CCoreDX11SamplerState::~CCoreDX11SamplerState()
{
	if (State) State->Release();
}

TBOOL CCoreDX11SamplerState::Update()
{
	if (!Dirty) return true;
	if (State) State->Release();
	State = NULL;

	D3D11_SAMPLER_DESC desc;

	desc.AddressU = DX11TextureAddressModes[AddressU];
	desc.AddressV = DX11TextureAddressModes[AddressV];
	desc.AddressW = DX11TextureAddressModes[AddressW];
	desc.BorderColor[0] = BorderColor[0];
	desc.BorderColor[1] = BorderColor[1];
	desc.BorderColor[2] = BorderColor[2];
	desc.BorderColor[3] = BorderColor[3];
	desc.ComparisonFunc = DX11ComparisonFunctions[ComparisonFunc];
	desc.Filter = DX11Filters[Filter];
	desc.MaxAnisotropy = MaxAnisotropy;
	desc.MaxLOD = MaxLOD;
	desc.MinLOD = MinLOD;
	desc.MipLODBias = MipLODBias;

	Dirty = false;

	HRESULT res = Dev->CreateSamplerState(&desc, &State);
	if (res != S_OK)
	{
		_com_error err(res);
		LOG(LOG_ERROR, _T("[core] DirectX11 Sampler state creation failed (%s)"), err.ErrorMessage());
		return false;
	}

	return true;
}

TBOOL CCoreDX11SamplerState::Apply(CORESAMPLER Smp)
{
	Update();

	if (Smp >= CORESMP_PS0 && Smp <= CORESMP_PS15)
		Context->PSSetSamplers(Smp - CORESMP_PS0, 1, &State);

	if (Smp >= CORESMP_VS0 && Smp <= CORESMP_VS3)
		Context->VSSetSamplers(Smp - CORESMP_VS0, 1, &State);

	if (Smp >= CORESMP_GS0 && Smp <= CORESMP_GS3)
		Context->GSSetSamplers(Smp - CORESMP_GS0, 1, &State);

	return true;
}

#else
NoEmptyFile();
#endif
