#include "BasePCH.h"
#include "Scene.h"
#include "Device.h"

CCoreScene::CCoreScene()
{
	Root = NULL;
	ShaderData = NULL;
}

CCoreScene::~CCoreScene()
{
	SAFEDELETE(ShaderData);
	SAFEDELETE(Root);
}

void CCoreScene::ProcessTree(CCoreDevice *Device)
{
	if (!Root) return;

	Root->ProcessTree(Device, Layers);
}

void CCoreScene::Render(CCoreDevice *Device)
{
	//set scene constant buffer here
	Device->SetShaderConstants(0, 1, &ShaderData);

	for (TS32 x = 0; x < Layers.NumItems(); x++)
		Layers[x]->Render(Device);
}

void CCoreScene::AddRenderLayer(CCoreDevice *Device, CString &LayerName)
{
	CCoreRenderLayerDescriptor *d = Device->GetRenderLayer(LayerName);
	if (!d) return;
	if (Layers.HasKey(d->GetID())) return;

	Layers[d->GetID()] = d->SpawnRenderLayer(Device);
}
