#include "BasePCH.h"
#include "SceneNodes.h"

CCoreSceneNode::CCoreSceneNode()
{
	Group = NULL;
}

CCoreSceneNode::~CCoreSceneNode()
{

}

void CCoreSceneNode::ProcessTree(CCoreDevice *Device, CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers)
{

	for (TS32 x = 0; x < Children.NumItems(); x++)
		Children[x]->ProcessTree(Device, Layers);

}

void CCoreSceneNode::SetGroup(CCoreObjectGroup *group)
{
	Group = group;
}

CCoreNodeRenderable::CCoreNodeRenderable()
{
	ObjectBuffer = NULL;
}

CCoreNodeRenderable::~CCoreNodeRenderable()
{
	SAFEDELETE(ObjectBuffer);
}

void CCoreNodeRenderable::ProcessTree(CCoreDevice *Device, CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers)
{
	GatherAtomsToRender(Device, Layers);
	return CCoreSceneNode::ProcessTree(Device, Layers);
}

void CCoreModelNode::GatherAtomsToRender(CCoreDevice *Device, CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers)
{
	if (!Models.NumItems()) return;

	//update animation here

	//update dynamic tech constant buffers here
	for (TS32 x = 0; x < Models.NumItems(); x++)
		Models[x]->UpdateData(Device, CORE_BUFFER_TECHDYNAMIC, Group, Layers);

	for (TS32 x = 0; x < Atoms.NumItems(); x++)
		Layers[Atoms[x]->GetTargetLayer()]->AddAtom(Atoms[x]);
}

CCoreModelNode::CCoreModelNode()
{
}

CCoreModelNode::~CCoreModelNode()
{
	//Models.FreeArray();
	Atoms.FreeArray();
}

void CCoreModelNode::CreateAtoms(CCoreDevice *Device, CCoreConstantBuffer *SceneBuffer, CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers)
{
	Atoms.FreeArray();

	for (TS32 x = 0; x < Models.NumItems(); x++)
	{
		Models[x]->UpdateData(Device, CORE_BUFFER_GAMEDATA, Group, Layers);
		Models[x]->UpdateData(Device, CORE_BUFFER_TECHSTATIC, Group, Layers);
		Models[x]->UpdateData(Device, CORE_BUFFER_TECHDYNAMIC, Group, Layers);
	}

	for (TS32 x = 0; x < Models.NumItems(); x++)
		Models[x]->CreateAtoms(Layers, Atoms, SceneBuffer, ObjectBuffer);
}
