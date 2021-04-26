#pragma once
#include "Resource.h"

#define COREBUFFERSCOPECOUNT 6

enum COREBUFFERSCOPE
{
	CORE_BUFFER_SCENE = 0,
	CORE_BUFFER_LAYER,
	CORE_BUFFER_GAMEDATA,
	CORE_BUFFER_OBJECT,
	CORE_BUFFER_TECHSTATIC,
	CORE_BUFFER_TECHDYNAMIC
};

class CCoreConstantBuffer : public CCoreResource
{

protected:

	TU8 *Data;
	TS32 BufferLength;
	TS32 DataLength;

public:

	CCoreConstantBuffer(CCoreDevice *Device);
	virtual ~CCoreConstantBuffer();

	void Reset();
	void AddData(void *Data, TS32 Length);
	virtual void Upload();
	virtual void *GetBufferPointer() = 0;


};