#include "BasePCH.h"
#include "RenderState.h"
#include "../UtilLib/XMLDocument.h"

CCoreRenderStateBatch::CCoreRenderStateBatch(CCoreDevice *Device) : CCoreResource(Device)
{
	Dirty = true;
}

CCoreRenderStateBatch::~CCoreRenderStateBatch()
{

}

CCoreBlendState::CCoreBlendState(CCoreDevice *Device) : CCoreRenderStateBatch(Device)
{
	AlphaToCoverage = false;
	IndependentBlend = false;

	for (TS32 x = 0; x < 8; x++)
	{
		RenderTargetBlendStates[x].BlendEnable = false;
		RenderTargetBlendStates[x].SrcBlend = COREBLEND_ONE;
		RenderTargetBlendStates[x].DestBlend = COREBLEND_ZERO;
		RenderTargetBlendStates[x].BlendOp = COREBLENDOP_ADD;
		RenderTargetBlendStates[x].SrcBlendAlpha = COREBLEND_ONE;
		RenderTargetBlendStates[x].DestBlendAlpha = COREBLEND_ZERO;
		RenderTargetBlendStates[x].BlendOpAlpha = COREBLENDOP_ADD;
		RenderTargetBlendStates[x].RenderTargetWriteMask = 0x0f;
	}
}

CCoreBlendState::~CCoreBlendState()
{

}

void CCoreBlendState::SetRenderTargetWriteMask(TS32 rt, TU8 e)
{
	if (RenderTargetBlendStates[rt].RenderTargetWriteMask != e) Dirty = true;
	RenderTargetBlendStates[rt].RenderTargetWriteMask = e;
}

void CCoreBlendState::SetBlendOpAlpha(TS32 rt, COREBLENDOP e)
{
	if (RenderTargetBlendStates[rt].BlendOpAlpha != e) Dirty = true;
	RenderTargetBlendStates[rt].BlendOpAlpha = e;
}

void CCoreBlendState::SetDestBlendAlpha(TS32 rt, COREBLENDFACTOR e)
{
	if (RenderTargetBlendStates[rt].DestBlendAlpha != e) Dirty = true;
	RenderTargetBlendStates[rt].DestBlendAlpha = e;
}

void CCoreBlendState::SetSrcBlendAlpha(TS32 rt, COREBLENDFACTOR e)
{
	if (RenderTargetBlendStates[rt].SrcBlendAlpha != e) Dirty = true;
	RenderTargetBlendStates[rt].SrcBlendAlpha = e;
}

void CCoreBlendState::SetBlendOp(TS32 rt, COREBLENDOP e)
{
	if (RenderTargetBlendStates[rt].BlendOp != e) Dirty = true;
	RenderTargetBlendStates[rt].BlendOp = e;
}

void CCoreBlendState::SetDestBlend(TS32 rt, COREBLENDFACTOR e)
{
	if (RenderTargetBlendStates[rt].DestBlend != e) Dirty = true;
	RenderTargetBlendStates[rt].DestBlend = e;
}

void CCoreBlendState::SetSrcBlend(TS32 rt, COREBLENDFACTOR e)
{
	if (RenderTargetBlendStates[rt].SrcBlend != e) Dirty = true;
	RenderTargetBlendStates[rt].SrcBlend = e;
}

void CCoreBlendState::SetBlendEnable(TS32 rt, TBOOL e)
{
	if (RenderTargetBlendStates[rt].BlendEnable != e) Dirty = true;
	RenderTargetBlendStates[rt].BlendEnable = e;
}

void CCoreBlendState::SetIndependentBlend(TBOOL e)
{
	if (IndependentBlend != e) Dirty = true;
	IndependentBlend = e;
}

void CCoreBlendState::SetAlphaToCoverage(TBOOL e)
{
	if (AlphaToCoverage != e) Dirty = true;
	AlphaToCoverage = e;
}

TBOOL CCoreBlendState::Import(CXMLNode *n)
{
	CString s;

	if (n->HasAttribute(_T("AlphaToCoverage"))) { n->GetChild(_T("AlphaToCoverage")).GetValue(AlphaToCoverage); }
	if (n->HasAttribute(_T("IndependentBlend"))) { n->GetChild(_T("IndependentBlend")).GetValue(IndependentBlend); }

	for (TS32 x = 0; x < n->GetChildCount(_T("RenderTarget")); x++)
	{
		CXMLNode c = n->GetChild(_T("RenderTarget"), x);
		TS32 id = 0;
		c.GetAttributeAsInteger(_T("Target"), &id);

		if (c.GetChildCount(_T("BlendEnable"))) { c.GetChild(_T("BlendEnable")).GetValue(RenderTargetBlendStates[id].BlendEnable); }
		if (c.GetChildCount(_T("SrcBlend"))) { s = c.GetChild(_T("SrcBlend")).GetText(); FindEnumByName(BlendFactorNames, s, (TS32&)RenderTargetBlendStates[id].SrcBlend); }
		if (c.GetChildCount(_T("DestBlend"))) { s = c.GetChild(_T("DestBlend")).GetText(); FindEnumByName(BlendFactorNames, s, (TS32&)RenderTargetBlendStates[id].DestBlend); }
		if (c.GetChildCount(_T("BlendOp"))) { s = c.GetChild(_T("BlendOp")).GetText(); FindEnumByName(BlendOpNames, s, (TS32&)RenderTargetBlendStates[id].BlendOp); }

		if (c.GetChildCount(_T("SrcBlendAlpha"))) { s = c.GetChild(_T("SrcBlendAlpha")).GetText(); FindEnumByName(BlendFactorNames, s, (TS32&)RenderTargetBlendStates[id].SrcBlendAlpha); }
		if (c.GetChildCount(_T("DestBlendAlpha"))) { s = c.GetChild(_T("DestBlendAlpha")).GetText(); FindEnumByName(BlendFactorNames, s, (TS32&)RenderTargetBlendStates[id].DestBlendAlpha); }
		if (c.GetChildCount(_T("BlendOpAlpha"))) { s = c.GetChild(_T("BlendOpAlpha")).GetText(); FindEnumByName(BlendOpNames, s, (TS32&)RenderTargetBlendStates[id].BlendOpAlpha); }

		if (c.GetChildCount(_T("RenderTargetWriteMask"))) { c.GetChild(_T("RenderTargetWriteMask")).GetValue(RenderTargetBlendStates[id].RenderTargetWriteMask); }
	}

	Dirty = true;
	return true;
}

void CCoreBlendState::Export(CXMLNode *n)
{
	n->AddChild(_T("AlphaToCoverage"), false).SetInt(AlphaToCoverage);
	n->AddChild(_T("IndependentBlend"), false).SetInt(IndependentBlend);

	for (TS32 x = 0; x < 8; x++)
	{
		CXMLNode b = n->AddChild(_T("RenderTarget"));
		b.SetAttributeFromInteger(_T("Target"), x);

		b.AddChild(_T("BlendEnable"), false).SetInt(RenderTargetBlendStates[x].BlendEnable);
		b.AddChild(_T("SrcBlend"), false).SetText(FindNameByEnum(BlendFactorNames, RenderTargetBlendStates[x].SrcBlend));
		b.AddChild(_T("DestBlend"), false).SetText(FindNameByEnum(BlendFactorNames, RenderTargetBlendStates[x].DestBlend));
		b.AddChild(_T("BlendOp"), false).SetText(FindNameByEnum(BlendOpNames, RenderTargetBlendStates[x].BlendOp));

		b.AddChild(_T("SrcBlendAlpha"), false).SetText(FindNameByEnum(BlendFactorNames, RenderTargetBlendStates[x].SrcBlendAlpha));
		b.AddChild(_T("DestBlendAlpha"), false).SetText(FindNameByEnum(BlendFactorNames, RenderTargetBlendStates[x].DestBlendAlpha));
		b.AddChild(_T("BlendOpAlpha"), false).SetText(FindNameByEnum(BlendOpNames, RenderTargetBlendStates[x].BlendOpAlpha));

		b.AddChild(_T("RenderTargetWriteMask")).SetInt(RenderTargetBlendStates[x].RenderTargetWriteMask);
	}
}

CCoreDepthStencilState::CCoreDepthStencilState(CCoreDevice *Device) : CCoreRenderStateBatch(Device)
{
	DepthEnable = true;
	ZWriteEnable = true;
	DepthFunc = CORECMP_LESS;
}

CCoreDepthStencilState::~CCoreDepthStencilState()
{

}

void CCoreDepthStencilState::SetDepthFunc(CORECOMPARISONFUNCTION e)
{
	if (e != DepthFunc) Dirty = true;
	DepthFunc = e;
}

void CCoreDepthStencilState::SetZWriteEnable(TBOOL e)
{
	if (e != ZWriteEnable) Dirty = true;
	ZWriteEnable = e;
}

void CCoreDepthStencilState::SetDepthEnable(TBOOL e)
{
	if (e != DepthEnable) Dirty = true;
	DepthEnable = e;
}

TBOOL CCoreDepthStencilState::Import(CXMLNode *n)
{
	if (n->GetChildCount(_T("DepthEnable"))) { n->GetChild(_T("DepthEnable")).GetValue(DepthEnable); }
	if (n->GetChildCount(_T("ZWriteEnable"))) { n->GetChild(_T("ZWriteEnable")).GetValue(ZWriteEnable); }
	if (n->GetChildCount(_T("DepthFunc"))) { CString s = n->GetChild(_T("DepthFunc")).GetText(); FindEnumByName(ComparisonFunctionNames, s, (TS32&)DepthFunc); }

	Dirty = true;
	return true;
}

void CCoreDepthStencilState::Export(CXMLNode *n)
{
	n->AddChild(_T("DepthEnable"), false).SetInt(DepthEnable);
	n->AddChild(_T("ZWriteEnable"), false).SetInt(ZWriteEnable);
	n->AddChild(_T("DepthFunc")).SetText(FindNameByEnum(ComparisonFunctionNames, DepthFunc));
}

CCoreRasterizerState::CCoreRasterizerState(CCoreDevice *Device) : CCoreRenderStateBatch(Device)
{
	FillMode = COREFILL_SOLID;
	CullMode = CORECULL_CCW;
	FrontCounterClockwise = false;
	DepthBias = 0;
	DepthBiasClamp = 0;
	SlopeScaledDepthBias = 0;
	DepthClipEnable = true;
	ScissorEnable = false;
	MultisampleEnable = false;
	AntialiasedLineEnable = false;
}

CCoreRasterizerState::~CCoreRasterizerState()
{

}

void CCoreRasterizerState::SetAntialiasedLineEnable(TBOOL e)
{
	if (AntialiasedLineEnable != e) Dirty = true;
	AntialiasedLineEnable = e;
}

void CCoreRasterizerState::SetMultisampleEnable(TBOOL e)
{
	if (MultisampleEnable != e) Dirty = true;
	MultisampleEnable = e;
}

void CCoreRasterizerState::SetScissorEnable(TBOOL e)
{
	if (ScissorEnable != e) Dirty = true;
	ScissorEnable = e;
}

void CCoreRasterizerState::SetDepthClipEnable(TBOOL e)
{
	if (DepthClipEnable != e) Dirty = true;
	DepthClipEnable = e;
}

void CCoreRasterizerState::SetSlopeScaledDepthBias(TF32 e)
{
	if (SlopeScaledDepthBias != e) Dirty = true;
	SlopeScaledDepthBias = e;
}

void CCoreRasterizerState::SetDepthBiasClamp(TF32 e)
{
	if (DepthBiasClamp != e) Dirty = true;
	DepthBiasClamp = e;
}

void CCoreRasterizerState::SetDepthBias(TS32 e)
{
	if (DepthBias != e) Dirty = true;
	DepthBias = e;
}

void CCoreRasterizerState::SetFrontCounterClockwise(TBOOL e)
{
	if (FrontCounterClockwise != e) Dirty = true;
	FrontCounterClockwise = e;
}

void CCoreRasterizerState::SetCullMode(CORECULLMODE e)
{
	if (CullMode != e) Dirty = true;
	CullMode = e;
}

void CCoreRasterizerState::SetFillMode(COREFILLMODE e)
{
	if (FillMode != e) Dirty = true;
	FillMode = e;
}

TBOOL CCoreRasterizerState::Import(CXMLNode *n)
{
	if (n->GetChildCount(_T("FillMode"))) { CString s = n->GetChild(_T("FillMode")).GetText(); FindEnumByName(FillModeNames, s, (TS32&)FillMode); }
	if (n->GetChildCount(_T("CullMode"))) { CString s = n->GetChild(_T("CullMode")).GetText(); FindEnumByName(CullModeNames, s, (TS32&)CullMode); }

	if (n->GetChildCount(_T("DepthBias"))) { n->GetChild(_T("DepthBias")).GetValue(DepthBias); }
	if (n->GetChildCount(_T("DepthBiasClamp"))) { n->GetChild(_T("DepthBiasClamp")).GetValue(DepthBiasClamp); }
	if (n->GetChildCount(_T("SlopeScaledDepthBias"))) { n->GetChild(_T("SlopeScaledDepthBias")).GetValue(SlopeScaledDepthBias); }

	if (n->GetChildCount(_T("FrontCounterClockwise"))) { n->GetChild(_T("FrontCounterClockwise")).GetValue(FrontCounterClockwise); }
	if (n->GetChildCount(_T("DepthClipEnable"))) { n->GetChild(_T("DepthClipEnable")).GetValue(DepthClipEnable); }
	if (n->GetChildCount(_T("ScissorEnable"))) { n->GetChild(_T("ScissorEnable")).GetValue(ScissorEnable); }
	if (n->GetChildCount(_T("MultisampleEnable"))) { n->GetChild(_T("MultisampleEnable")).GetValue(MultisampleEnable); }
	if (n->GetChildCount(_T("AntialiasedLineEnable"))) { n->GetChild(_T("AntialiasedLineEnable")).GetValue(AntialiasedLineEnable); }

	Dirty = true;
	return true;
}

void CCoreRasterizerState::Export(CXMLNode *n)
{
	n->AddChild(_T("FillMode"), false).SetText(FindNameByEnum(FillModeNames, FillMode));
	n->AddChild(_T("CullMode"), false).SetText(FindNameByEnum(CullModeNames, CullMode));

	n->AddChild(_T("DepthBias"), false).SetInt(DepthBias);
	n->AddChild(_T("DepthBiasClamp"), false).SetFloat(DepthBiasClamp);
	n->AddChild(_T("SlopeScaledDepthBias"), false).SetFloat(SlopeScaledDepthBias);
	n->AddChild(_T("FrontCounterClockwise"), false).SetInt(FrontCounterClockwise);
	n->AddChild(_T("DepthClipEnable"), false).SetInt(DepthClipEnable);
	n->AddChild(_T("ScissorEnable"), false).SetInt(ScissorEnable);
	n->AddChild(_T("MultisampleEnable"), false).SetInt(MultisampleEnable);
	n->AddChild(_T("AntialiasedLineEnable")).SetInt(AntialiasedLineEnable);
}

CCoreSamplerState::CCoreSamplerState(CCoreDevice *Device) : CCoreRenderStateBatch(Device)
{
	Filter = COREFILTER_MIN_MAG_MIP_LINEAR;
	AddressU = CORETEXADDRESS_CLAMP;
	AddressV = CORETEXADDRESS_CLAMP;
	AddressW = CORETEXADDRESS_CLAMP;
	MinLOD = -FLT_MAX;
	MaxLOD = FLT_MAX;
	MipLODBias = 0;
	MaxAnisotropy = 1;
	ComparisonFunc = CORECMP_NEVER;
	BorderColor[0] = BorderColor[1] = BorderColor[2] = BorderColor[3] = 1;
}

CCoreSamplerState::~CCoreSamplerState()
{

}

void CCoreSamplerState::SetBorderColor(TF32 r, TF32 g, TF32 b, TF32 a)
{
	if (BorderColor[0] != r || BorderColor[1] != g || BorderColor[2] != b || BorderColor[3] != a) Dirty = true;
	BorderColor[0] = r;
	BorderColor[1] = g;
	BorderColor[2] = b;
	BorderColor[3] = a;
}

void CCoreSamplerState::SetMaxLOD(TF32 e)
{
	if (MaxLOD != e) Dirty = true; MaxLOD = e;
}

void CCoreSamplerState::SetMinLOD(TF32 e)
{
	if (MinLOD != e) Dirty = true;
	MinLOD = e;
}

void CCoreSamplerState::SetComparisonFunc(CORECOMPARISONFUNCTION e)
{
	if (ComparisonFunc != e) Dirty = true;
	ComparisonFunc = e;
}

void CCoreSamplerState::SetMaxAnisotropy(TS32 e)
{
	if (MaxAnisotropy != e) Dirty = true;
	MaxAnisotropy = e;
}

void CCoreSamplerState::SetMipLODBias(TF32 e)
{
	if (MipLODBias != e) Dirty = true;
	MipLODBias = e;
}

void CCoreSamplerState::SetAddressW(CORETEXTUREADDRESSMODE e)
{
	if (AddressW != e) Dirty = true;
	AddressW = e;
}

void CCoreSamplerState::SetAddressV(CORETEXTUREADDRESSMODE e)
{
	if (AddressV != e) Dirty = true;
	AddressV = e;
}

void CCoreSamplerState::SetAddressU(CORETEXTUREADDRESSMODE e)
{
	if (AddressU != e) Dirty = true;
	AddressU = e;
}

void CCoreSamplerState::SetFilter(COREFILTER e)
{
	if (Filter != e) Dirty = true;
	Filter = e;
}

TBOOL CCoreSamplerState::Import(CXMLNode *n)
{
	if (n->GetChildCount(_T("Filter"))) { CString s = n->GetChild(_T("Filter")).GetText(); FindEnumByName(FilterNames, s, (TS32&)Filter); }
	if (n->GetChildCount(_T("AddressU"))) { CString s = n->GetChild(_T("AddressU")).GetText(); FindEnumByName(AddressModeNames, s, (TS32&)AddressU); }
	if (n->GetChildCount(_T("AddressV"))) { CString s = n->GetChild(_T("AddressV")).GetText(); FindEnumByName(AddressModeNames, s, (TS32&)AddressV); }
	if (n->GetChildCount(_T("AddressW"))) { CString s = n->GetChild(_T("AddressW")).GetText(); FindEnumByName(AddressModeNames, s, (TS32&)AddressW); }

	if (n->GetChildCount(_T("ComparisonFunc"))) { CString s = n->GetChild(_T("ComparisonFunc")).GetText(); FindEnumByName(ComparisonFunctionNames, s, (TS32&)ComparisonFunc); }

	if (n->GetChildCount(_T("MipLODBias"))) { n->GetChild(_T("MipLODBias")).GetValue(MipLODBias); }
	if (n->GetChildCount(_T("MinLOD"))) { n->GetChild(_T("MinLOD")).GetValue(MinLOD); }
	if (n->GetChildCount(_T("MaxLOD"))) { n->GetChild(_T("MaxLOD")).GetValue(MaxLOD); }

	if (n->GetChildCount(_T("MaxAnisotropy"))) { n->GetChild(_T("MaxAnisotropy")).GetValue(MaxAnisotropy); }

	Dirty = true;
	return true;
}

void CCoreSamplerState::Export(CXMLNode *n)
{
	n->AddChild(_T("Filter"), false).SetText(FindNameByEnum(FilterNames, Filter));
	n->AddChild(_T("AddressU"), false).SetText(FindNameByEnum(AddressModeNames, AddressU));
	n->AddChild(_T("AddressV"), false).SetText(FindNameByEnum(AddressModeNames, AddressV));
	n->AddChild(_T("AddressW"), false).SetText(FindNameByEnum(AddressModeNames, AddressW));
	n->AddChild(_T("ComparisonFunc"), false).SetText(FindNameByEnum(ComparisonFunctionNames, ComparisonFunc));
	n->AddChild(_T("MipLODBias"), false).SetFloat(MipLODBias);
	n->AddChild(_T("MinLOD"), false).SetFloat(MinLOD);
	n->AddChild(_T("MaxLOD"), false).SetFloat(MaxLOD);
	n->AddChild(_T("MaxAnisotropy")).SetInt(MaxAnisotropy);
}
