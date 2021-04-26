#pragma once
#include "ConstantBuffer.h"
#include "RenderState.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

typedef TS32 CORERENDERLAYERID;

class CCoreMaterialRenderPass;
class CCoreMesh;

class CCoreAtom
{
	CORERENDERLAYERID TargetLayer;

	CCoreMaterialRenderPass *RenderPass;
	CCoreConstantBuffer *Buffers[4]; //objectdata, groupdata, constant tech data, dynamic tech data

	CCoreVertexBuffer *Vertices;
	CCoreIndexBuffer *Indices;

	CCoreVertexFormat *VxFormat;

	TS32 TriCount;
	TS32 VxCount;

public:

	CCoreAtom();
	CCoreAtom(CORERENDERLAYERID TargetLayer, CCoreMaterialRenderPass *Pass, CCoreConstantBuffer *Buffers[4], CCoreMesh *Mesh);
	virtual ~CCoreAtom();

	virtual void Render(CCoreDevice *Device);

	INLINE CORERENDERLAYERID &GetTargetLayer() { return TargetLayer; }

};