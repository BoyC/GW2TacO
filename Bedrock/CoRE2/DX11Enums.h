#pragma once
#include "Core2_Config.h"
#include "Enums.h"

#ifdef CORE_API_DX11

extern DXGI_FORMAT DX11Formats[];
extern D3D11_CULL_MODE DX11CullModes[];
extern D3D11_FILL_MODE DX11FillModes[];
extern D3D11_COMPARISON_FUNC DX11ComparisonFunctions[];
extern D3D11_BLEND DX11BlendFactors[];
extern D3D11_BLEND_OP DX11BlendOps[];
extern D3D11_FILTER DX11Filters[];
extern D3D11_TEXTURE_ADDRESS_MODE DX11TextureAddressModes[];
extern DWORD DX11TextureWrapModes[];
extern DWORD DX11Samplers[];
extern /*D3DRENDERSTATETYPE*/DWORD DX11WrapModes[];

COREFORMAT GetFormat(DXGI_FORMAT Format);

#endif