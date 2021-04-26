#pragma once
#include "Material.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

class CCoreMesh
{
	CCoreVertexFormat *VxFormat;
	CCoreVertexBuffer *Vertices;
	CCoreIndexBuffer *Indices;
	TS32 TriCount;
	TS32 VxCount;

public:

	CCoreMesh();
	virtual ~CCoreMesh();

	CCoreVertexFormat *GetVertexFormat();
	CCoreVertexBuffer *GetVertices();
	CCoreIndexBuffer *GetIndices();
	TS32 GetTriCount();
	TS32 GetVxCount();

	void SetVertexFormat(CCoreVertexFormat *v);
	void SetVertexBuffer(CCoreVertexBuffer *b, TS32 Count);
	void SetIndexBuffer(CCoreIndexBuffer *i, TS32 TriangleCount);

};

class CCoreModel
{

	CCoreMaterial *Material;
	CArray<CCoreMesh*> Meshes;

	CDictionary<CCoreMaterialTechnique*, CCoreConstantBuffer*> Buffers[3]; //game, static, dynamic

public:

	CCoreModel();
	virtual ~CCoreModel();

	INLINE TS32 GetMeshCount() { return Meshes.NumItems(); }
	INLINE CCoreMesh *GetMesh(TS32 x) { return Meshes[x]; }
	virtual void AddMesh(CCoreMesh *m) { Meshes += m; }
	virtual void SetMaterial(CCoreMaterial *m) { Material = m; }

	virtual void UpdateData(CCoreDevice *Device, COREBUFFERSCOPE Scope, CCoreObjectGroup *Group, CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers);
	void CreateAtoms(CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers, CArray<CCoreAtom*> &Atoms, CCoreConstantBuffer *SceneBuffer, CCoreConstantBuffer *ObjectBuffer);

};

