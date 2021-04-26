#include "BasePCH.h"
#include "Model.h"
#include "Device.h"

void CCoreModel::UpdateData(CCoreDevice *Device, COREBUFFERSCOPE Scope, CCoreObjectGroup *Group, CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers)
{
	if (!Material) return;
	Material->GatherData(Device, Scope, Group, (CDictionary<CCoreMaterialTechnique*, CCoreConstantBuffer*>**)Buffers, Layers);
}

void CCoreModel::CreateAtoms(CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers, CArray<CCoreAtom*> &Atoms, CCoreConstantBuffer *SceneBuffer, CCoreConstantBuffer *ObjectBuffer)
{
	if (!Material) return;

	for (TS32 x = 0; x < Meshes.NumItems(); x++)
	{
		Material->CreateAtoms(Layers, SceneBuffer, ObjectBuffer, (CDictionary<CCoreMaterialTechnique*, CCoreConstantBuffer*>**)Buffers, Atoms, Meshes[x]);
	}
}

void CCoreMesh::SetIndexBuffer(CCoreIndexBuffer *i, TS32 TriangleCount)
{
	Indices = i;
	TriCount = TriangleCount;
}

void CCoreMesh::SetVertexBuffer(CCoreVertexBuffer *b, TS32 Count)
{
	Vertices = b;
	VxCount = Count;
}

void CCoreMesh::SetVertexFormat(CCoreVertexFormat *v)
{
	VxFormat = v;
}

TS32 CCoreMesh::GetVxCount()
{
	return VxCount;
}

TS32 CCoreMesh::GetTriCount()
{
	return TriCount;
}

CCoreIndexBuffer * CCoreMesh::GetIndices()
{
	return Indices;
}

CCoreVertexBuffer * CCoreMesh::GetVertices()
{
	return Vertices;
}

CCoreVertexFormat * CCoreMesh::GetVertexFormat()
{
	return VxFormat;
}
