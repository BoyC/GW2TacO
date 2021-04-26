#pragma once
#include "DX11Device.h"
#include "IndexBuffer.h"

#ifdef CORE_API_DX11

class CCoreDX11IndexBuffer : public CCoreIndexBuffer
{
	ID3D11Device *Dev;
	ID3D11DeviceContext *DeviceContext;
	ID3D11Buffer *IndexBufferHandle;

	TS32 IndexCount;
	TS32 IndexSize;

	virtual void Release();
	virtual TBOOL Apply();

public:

	CCoreDX11IndexBuffer(CCoreDX11Device *dev);
	virtual ~CCoreDX11IndexBuffer();

	virtual TBOOL Create(const TU32 IndexCount, const TU32 IndexSize = 2);
	virtual TBOOL Lock(void **Result);
	virtual TBOOL Lock(void **Result, const TU32 IndexOffset, const TS32 IndexCount);
	virtual TBOOL UnLock();
	virtual void* GetHandle() { return IndexBufferHandle; }
};

#endif