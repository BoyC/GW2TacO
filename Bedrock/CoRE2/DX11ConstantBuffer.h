#pragma once
#include "DX11Device.h"
#include "ConstantBuffer.h"

#ifdef CORE_API_DX11
class CCoreDX11ConstantBuffer : public CCoreConstantBuffer
{
	ID3D11Buffer *Buffer;
	ID3D11Device *Dev;
	ID3D11DeviceContext *DeviceContext;
	TS32 AllocatedBufferSize;

public:

	CCoreDX11ConstantBuffer(CCoreDX11Device *Device);
	virtual ~CCoreDX11ConstantBuffer();

	virtual void Upload();
	virtual void *GetBufferPointer();

};
#endif