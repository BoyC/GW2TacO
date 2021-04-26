#include "BasePCH.h"
#include "RenderLayer.h"
#include "Device.h"

//////////////////////////////////////////////////////////////////////////
// Descriptor

CCoreRenderLayerDescriptor::CCoreRenderLayerDescriptor(CString &name)
{
	Name = name;
	NameHash = DictionaryHash(Name);
}

CCoreRenderLayerDescriptor::~CCoreRenderLayerDescriptor()
{

}

CCoreRenderLayer * CCoreRenderLayerDescriptor::SpawnRenderLayer(CCoreDevice *dev)
{
	return new CCoreRenderLayer(dev, this);
}

//////////////////////////////////////////////////////////////////////////
// Renderlayer

CCoreRenderLayer::CCoreRenderLayer(CCoreDevice *dev, CCoreRenderLayerDescriptor *desc)
{
	LayerData = dev->CreateConstantBuffer();
	Descriptor = desc;
}

CCoreRenderLayer::~CCoreRenderLayer()
{
	SAFEDELETE(LayerData);
}

void CCoreRenderLayer::Optimize()
{
	//sort render atoms by material etc
}

void CCoreRenderLayer::Render(CCoreDevice *Device)
{
	//set layer constant buffer here
	Device->SetShaderConstants(1, 1, &LayerData);

	for (TS32 x = 0; x < RenderList.NumItems(); x++)
		RenderList[x]->Render(Device);
}