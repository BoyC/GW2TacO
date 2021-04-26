#pragma once
#include "RenderLayer.h"
#include "Model.h"

class CCoreSceneNode
{
	CMatrix4x4 Transformation;
	CArray<CCoreSceneNode*> Children;

protected:

	CCoreObjectGroup *Group;

public:

	CCoreSceneNode();
	virtual ~CCoreSceneNode();

	virtual void ProcessTree(CCoreDevice *Device, CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers);
	void SetGroup(CCoreObjectGroup *group);

};

class CCoreNodeRenderable : public CCoreSceneNode
{
	virtual void GatherAtomsToRender(CCoreDevice *Device, CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers) = 0;

protected:
	CArray<CCoreAtom*> Atoms;
	CCoreConstantBuffer *ObjectBuffer;

public:

	CCoreNodeRenderable();
	virtual ~CCoreNodeRenderable();

	virtual void ProcessTree(CCoreDevice *Device, CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers);
	virtual void CreateAtoms(CCoreDevice *Device, CCoreConstantBuffer *SceneBuffer, CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers) = 0;

};

class CCoreModelNode : public CCoreNodeRenderable
{

	CArray<CCoreModel*> Models;

	virtual void GatherAtomsToRender(CCoreDevice *Device, CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers);

public:

	CCoreModelNode();
	virtual ~CCoreModelNode();
	virtual void CreateAtoms(CCoreDevice *Device, CCoreConstantBuffer *SceneBuffer, CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers);
	virtual void AddModel(CCoreModel *m) { Models += m; }

};