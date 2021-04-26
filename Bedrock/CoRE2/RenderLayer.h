#pragma once
#include "Atom.h"

class CCoreRenderLayer;

class CCoreRenderLayerDescriptor
{
	CString Name; //layer name
	CString Target; //target rendertexture
	CORERENDERLAYERID NameHash;

public:

	CCoreRenderLayerDescriptor(CString &Name);
	virtual ~CCoreRenderLayerDescriptor();

	INLINE CString &GetName() { return Name; }
	INLINE CORERENDERLAYERID GetID() { return NameHash; }

	CCoreRenderLayer *SpawnRenderLayer(CCoreDevice *dev);

};

class CCoreRenderLayer
{
	CCoreConstantBuffer *LayerData;

	CCoreRenderLayerDescriptor *Descriptor;
	CArray<CCoreAtom*> RenderList;

public:

	CCoreRenderLayer(CCoreDevice *dev, CCoreRenderLayerDescriptor *desc);
	virtual ~CCoreRenderLayer();

	void Optimize();
	void Render(CCoreDevice *Device);
	INLINE void AddAtom(CCoreAtom *Atom) { RenderList += Atom; }

	CCoreConstantBuffer *GetConstantBuffer() { return LayerData; }

};