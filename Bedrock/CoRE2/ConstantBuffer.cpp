#include "BasePCH.h"
#include "ConstantBuffer.h"


CCoreConstantBuffer::CCoreConstantBuffer(CCoreDevice *Device) : CCoreResource(Device)
{
	Data = NULL;
	DataLength = 0;
	BufferLength = 0;
}

CCoreConstantBuffer::~CCoreConstantBuffer()
{
	SAFEDELETEA(Data);
}

void CCoreConstantBuffer::Reset()
{
	DataLength = 0;
}

void CCoreConstantBuffer::AddData(void *DataIn, TS32 Length)
{
	if (DataLength + Length > BufferLength)
	{
		TU8 *OldData = Data;
		Data = new TU8[DataLength + Length];

		if (OldData)
			memcpy(Data, OldData, DataLength);

		BufferLength = DataLength + Length;
		SAFEDELETEA(OldData);
	}

	memcpy(Data + DataLength, DataIn, Length);
	DataLength += Length;
}

void CCoreConstantBuffer::Upload()
{

}
