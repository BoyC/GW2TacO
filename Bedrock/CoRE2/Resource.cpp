#include "BasePCH.h"
#include "Resource.h"
#include "Device.h"

CCoreResource::CCoreResource()
{
	Device = NULL;
}

CCoreResource::CCoreResource(CCoreDevice *h)
{
	Device = h;
	Device->AddResource(this);
}

CCoreResource::~CCoreResource()
{
	if (Device) Device->RemoveResource(this);
}

void CCoreResource::OnDeviceLost()
{

}

void CCoreResource::OnDeviceReset()
{

}