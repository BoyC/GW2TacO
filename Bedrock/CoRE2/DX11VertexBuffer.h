#pragma once
#include "DX11Device.h"
#include "VertexBuffer.h"

#ifdef CORE_API_DX11

class CCoreDX11VertexBuffer : public CCoreVertexBuffer
{
	ID3D11Device *Dev;
	ID3D11DeviceContext *DeviceContext;
	ID3D11Buffer *VertexBufferHandle;

	TS32 Size;
	TBOOL Dynamic;

	virtual void Release();
	virtual TBOOL Apply(const TU32 Offset);

public:

	CCoreDX11VertexBuffer(CCoreDX11Device *Device);
	virtual ~CCoreDX11VertexBuffer();

	virtual TBOOL Create(const TU8 *Data, const TU32 Size);
	virtual TBOOL CreateDynamic(const TU32 Size);
	virtual TBOOL Update(const TS32 Offset, const TU8 *Data, const TU32 Size);
	virtual TBOOL Lock(void **Result);
	virtual TBOOL Lock(void **Result, const TU32 Offset, const TS32 size, const TS32 Flags = 0);
	virtual TBOOL UnLock();
	virtual void* GetHandle() { return VertexBufferHandle; }
};

#endif