#pragma once
#include "Resource.h"

class CCoreVertexBuffer : public CCoreResource
{
	friend class CCoreDevice;
	virtual TBOOL Apply(const TU32 Offset) = 0;

public:

	INLINE CCoreVertexBuffer(CCoreDevice *Device) : CCoreResource(Device) {}

	virtual TBOOL Create(const TU8 *Data, const TU32 Size) = 0;
	virtual TBOOL CreateDynamic(const TU32 Size) = 0;
	virtual TBOOL Update(const TS32 Offset, const TU8 *Data, const TU32 Size) = 0;
	virtual TBOOL Lock(void **Result, const TU32 Offset, const TS32 size, const TS32 Flags) = 0;
	virtual TBOOL Lock(void **Result) = 0;
	virtual TBOOL UnLock() = 0;
	virtual void* GetHandle() = 0;
};