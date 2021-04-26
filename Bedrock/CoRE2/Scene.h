#pragma once
#include "RenderLayer.h"
#include "SceneNodes.h"

class CCoreScene
{

	CCoreConstantBuffer *ShaderData;

	CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> Layers;

	CCoreSceneNode *Root;

public:

	CCoreScene();
	virtual ~CCoreScene();

	void ProcessTree(CCoreDevice *Device);
	void Render(CCoreDevice *Device);

	void AddRenderLayer(CCoreDevice *Device, CString &LayerName);

};