#pragma once
#include "Resource.h"
#include "Enums.h"

class CCoreVertexShader;

class CCoreVertexFormat : public CCoreResource
{
	friend class CCoreDevice;
	virtual TBOOL Apply() = 0;

public:
	CCoreVertexFormat(CCoreDevice *Device) : CCoreResource(Device) {}

	virtual TBOOL Create(const CArray<COREVERTEXATTRIBUTE> &Attributes, CCoreVertexShader *VxShader = NULL) = 0;
	virtual TS32 GetSize() = 0;
};