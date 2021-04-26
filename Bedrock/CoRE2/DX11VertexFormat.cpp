#include "BasePCH.h"
#include "DX11VertexFormat.h"

#ifdef CORE_API_DX11

CCoreDX11VertexFormat::CCoreDX11VertexFormat(CCoreDX11Device *dev) : CCoreVertexFormat(dev)
{
	Dev = dev->GetDevice();
	DeviceContext = dev->GetDeviceContext();
	VertexFormatHandle = NULL;
	Size = 0;
}

CCoreDX11VertexFormat::~CCoreDX11VertexFormat()
{
	Release();
}

void CCoreDX11VertexFormat::Release()
{
	if (VertexFormatHandle) VertexFormatHandle->Release();
	VertexFormatHandle = NULL;
}

TBOOL CCoreDX11VertexFormat::Apply()
{
	if (!VertexFormatHandle) return false;
	DeviceContext->IASetInputLayout(VertexFormatHandle);
	return true;
}

TBOOL CCoreDX11VertexFormat::Create(const CArray<COREVERTEXATTRIBUTE> &Attributes, CCoreVertexShader *vs)
{
	if (!vs) return false;
	if (!Attributes.NumItems()) return false;
	Release();

	TS32 PosUsages = 0;
	TS32 NormUsages = 0;
	TS32 UVUsages = 0;
	TS32 ColUsages = 0;

	Size = 0;

	D3D11_INPUT_ELEMENT_DESC *vxdecl = new D3D11_INPUT_ELEMENT_DESC[Attributes.NumItems() + 1];
	memset(vxdecl, 0, sizeof(D3D11_INPUT_ELEMENT_DESC)*(Attributes.NumItems() + 1));

	for (TS32 x = 0; x < Attributes.NumItems(); x++)
	{
		vxdecl[x].InputSlot = 0;
		vxdecl[x].AlignedByteOffset = Size;
		vxdecl[x].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		vxdecl[x].InstanceDataStepRate = 0;

		switch (Attributes[x])
		{
			case COREVXATTR_POSITION3:
			{
				vxdecl[x].SemanticName = "Position";
				vxdecl[x].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				vxdecl[x].SemanticIndex = PosUsages++;
				Size += 12;
			}
				break;
			case COREVXATTR_POSITION4:
			{
				vxdecl[x].SemanticName = "Position";
				vxdecl[x].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				vxdecl[x].SemanticIndex = PosUsages++;
				Size += 16;
			}
				break;
			case COREVXATTR_NORMAL3:
			{
				vxdecl[x].SemanticName = "Normal";
				vxdecl[x].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				vxdecl[x].SemanticIndex = NormUsages++;
				Size += 12;
			}
				break;
			case COREVXATTR_TEXCOORD2:
			{
				vxdecl[x].SemanticName = "Texcoord";
				vxdecl[x].Format = DXGI_FORMAT_R32G32_FLOAT;
				vxdecl[x].SemanticIndex = UVUsages++;
				Size += 8;
			}
				break;
      case COREVXATTR_TEXCOORD4:
      {
        vxdecl[ x ].SemanticName = "Texcoord";
        vxdecl[ x ].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        vxdecl[ x ].SemanticIndex = UVUsages++;
        Size += 16;
      }
      break;
      case COREVXATTR_COLOR4:
			{
				vxdecl[x].SemanticName = "Color";
				vxdecl[x].Format = DXGI_FORMAT_B8G8R8A8_UNORM;
				vxdecl[x].SemanticIndex = ColUsages++;
				Size += 4;
			}
				break;
			case COREVXATTR_COLOR16:
			{
				vxdecl[x].SemanticName = "Color";
				vxdecl[x].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				vxdecl[x].SemanticIndex = ColUsages++;
				Size += 16;
			}
				break;
			case COREVXATTR_POSITIONT4:
			{
				vxdecl[x].SemanticName = "PositionT";
				vxdecl[x].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				vxdecl[x].SemanticIndex = PosUsages++;
				Size += 16;
			}
				break;

			default:
			{
				//unhandled format
				SAFEDELETEA(vxdecl);
				return false;
			}
		}
	}

	HRESULT res = Dev->CreateInputLayout(vxdecl, Attributes.NumItems(), vs->GetBinary(), vs->GetBinaryLength(), &VertexFormatHandle);
	if (res != S_OK)
	{
		_com_error err(res);
		LOG(LOG_ERROR, _T("[core] CreateInputLayout failed (%s)"), err.ErrorMessage());
		SAFEDELETEA(vxdecl);
		return false;
	}

	SAFEDELETEA(vxdecl);
	return true;
}

TS32 CCoreDX11VertexFormat::GetSize()
{
	return Size;
}

#else
NoEmptyFile();
#endif