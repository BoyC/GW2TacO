#pragma once
#include "Resource.h"

class CCoreIndexBuffer : public CCoreResource
{
	friend class CCoreDevice;
	virtual TBOOL Apply() = 0;

public:

	INLINE CCoreIndexBuffer(CCoreDevice *Device) : CCoreResource(Device) {}

	virtual TBOOL Create(const TU32 IndexCount, const TU32 IndexSize = 2) = 0;
	virtual TBOOL Lock(void **Result, const TU32 IndexOffset, const TS32 IndexCount) = 0;
	virtual TBOOL Lock(void **Result) = 0;
	virtual TBOOL UnLock() = 0;
	virtual void* GetHandle() = 0;
};