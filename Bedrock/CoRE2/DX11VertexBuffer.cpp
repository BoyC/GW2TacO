#include "BasePCH.h"
#include "DX11VertexBuffer.h"

#ifdef CORE_API_DX11

CCoreDX11VertexBuffer::CCoreDX11VertexBuffer(CCoreDX11Device *dev) : CCoreVertexBuffer(dev)
{
	Dev = dev->GetDevice();
	DeviceContext = dev->GetDeviceContext();
	VertexBufferHandle = NULL;
	Size = 0;
	Dynamic = false;
}


CCoreDX11VertexBuffer::~CCoreDX11VertexBuffer()
{
	Release();
}

void CCoreDX11VertexBuffer::Release()
{
	if (VertexBufferHandle) VertexBufferHandle->Release();
	VertexBufferHandle = NULL;
}

TBOOL CCoreDX11VertexBuffer::Apply(const TU32 Offset)
{
	if (!VertexBufferHandle) return false;
	TU32 stride = Device->GetVertexFormatSize();
	DeviceContext->IASetVertexBuffers(0, 1, &VertexBufferHandle, &stride, &Offset);
	return true;
}

TBOOL CCoreDX11VertexBuffer::Create(const TU8 *Data, const TU32 size)
{
	if (!Data) return false;
	if (size <= 0) return false;
	Release();

	D3D11_BUFFER_DESC bd;
	memset(&bd, 0, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = size;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vxData;
	vxData.pSysMem = Data;
	vxData.SysMemPitch = 0;
	vxData.SysMemSlicePitch = 0;

	HRESULT res = Dev->CreateBuffer(&bd, &vxData, &VertexBufferHandle);
	if (res != S_OK)
	{
		_com_error err(res);
		LOG(LOG_ERROR, _T("[core] CreateBuffer for vertexbuffer failed (%s)"), err.ErrorMessage());
		return false;
	}

	Size = size;
	return true;
}

TBOOL CCoreDX11VertexBuffer::CreateDynamic(const TU32 size)
{
	if (size <= 0) return false;
	Release();

	D3D11_BUFFER_DESC bd;
	memset(&bd, 0, sizeof(bd));

	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = size;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT res = Dev->CreateBuffer(&bd, NULL, &VertexBufferHandle);
	if (res != S_OK)
	{
		_com_error err(res);
		LOG(LOG_ERROR, _T("[core] CreateBuffer for vertexbuffer failed (%s)"), err.ErrorMessage());
		return false;
	}

	Size = size;
	Dynamic = true;

	return true;
}

TBOOL CCoreDX11VertexBuffer::Update(const TS32 Offset, const TU8 *Data, const TU32 Size)
{
	if (!VertexBufferHandle || !Data || Dynamic) return false;
	if (!Size) return true;

	D3D11_BOX box;
	memset(&box, 0, sizeof(D3D11_BOX));
	box.left = Offset;
	box.right = Offset + Size;

	DeviceContext->UpdateSubresource(VertexBufferHandle, 0, &box, Data, Size, 1);

	return true;
}


TBOOL CCoreDX11VertexBuffer::Lock(void **Result, const TU32 Offset, const TS32 size, const TS32 Flags)
{
	if (!Dynamic)
	{
		LOG(LOG_ERROR, _T("[core] Attempting to lock static vertexbuffer failed"));
		return false;
	}

	if (!VertexBufferHandle) return false;

	D3D11_MAPPED_SUBRESOURCE ms;
	if (DeviceContext->Map(VertexBufferHandle, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms) != S_OK) return false;

	*Result = ms.pData;
	return true;
}

TBOOL CCoreDX11VertexBuffer::Lock(void **Result)
{
	return Lock(Result, 0, Size);
}

TBOOL CCoreDX11VertexBuffer::UnLock()
{
	if (!VertexBufferHandle) return false;
	DeviceContext->Unmap(VertexBufferHandle, 0);
	return true;
}

#else
NoEmptyFile();
#endif