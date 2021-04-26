#include "BasePCH.h"
#include "Atom.h"
#include "Device.h"
#include "Model.h"

CCoreAtom::CCoreAtom()
{
	TargetLayer = 0;
	RenderPass = NULL;
	for (TS32 x = 0; x < 4; x++)
		Buffers[x] = NULL;
	Vertices = NULL;
	Indices = NULL;
	TriCount = 0;
	VxCount = 0;
	VxFormat = NULL;
}

CCoreAtom::CCoreAtom(CORERENDERLAYERID tl, CCoreMaterialRenderPass *Pass, CCoreConstantBuffer *b[4], CCoreMesh *Mesh)
{
	TriCount = Mesh->GetTriCount();
	VxCount = Mesh->GetVxCount();
	RenderPass = Pass;
	for (TS32 x = 0; x < 4; x++)
		Buffers[x] = b[x];
	Vertices = Mesh->GetVertices();
	Indices = Mesh->GetIndices();
	VxFormat = Mesh->GetVertexFormat();
}

CCoreAtom::~CCoreAtom()
{

}

void CCoreAtom::Render(CCoreDevice *Device)
{
	if (RenderPass) RenderPass->Apply();
	Device->SetShaderConstants(2, 4, Buffers);
	Device->SetVertexBuffer(Vertices, 0);
	Device->SetIndexBuffer(Indices);
	Device->DrawIndexedTriangles(TriCount, VxCount);
}