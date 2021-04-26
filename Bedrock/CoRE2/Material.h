#pragma once
#include "RenderLayer.h"
#include "ObjectGroup.h"
#include "../UtilLib/XMLDocument.h"

class CCoreMesh;

enum COREMATERIALPARAMETERSCOPE //if you add an enum here also add it in the .cpp to the name list!
{
	CORE_MATERIAL_CONSTANT = 0,	//stored per material
	CORE_MATERIAL_VARIABLE,		//stored per object
	CORE_MATERIAL_ANIMATED,		//stored per object as a spline
	CORE_MATERIAL_EXTERNAL		//not stored, set by the application
};

enum COREMATERIALPARAMETERTYPE //if you add an enum here also add it in the .cpp to the name list!
{
	CORE_MATERIAL_FLOAT = 0,
};

class CCoreMaterialParameter
{
	CString Name;
	COREMATERIALPARAMETERSCOPE Scope;
	COREMATERIALPARAMETERTYPE Type;
	TS32 ValueSize;
	TU8 *Value;

public:

	CCoreMaterialParameter();
	virtual ~CCoreMaterialParameter();
	INLINE COREMATERIALPARAMETERSCOPE &GetScope();
	INLINE TU8 *&GetData();
	INLINE TS32 &GetDataSize();
	INLINE CString &GetName();
	void SetName(CString &s);
	void SetScope(COREMATERIALPARAMETERSCOPE s);
	void SetType(COREMATERIALPARAMETERTYPE t);
	void SetData(TS32 Size, void *Data);

	TBOOL Import(CXMLNode *n);
	void Export(CXMLNode *Node);
};

class CCoreMaterialRenderPass
{
	CCoreDevice *Dev;

	CString Name;

	CCorePixelShader *PixelShader;
	CCoreVertexShader *VertexShader;
	CCoreGeometryShader *GeometryShader;

	CCoreBlendState *BlendState;
	CCoreDepthStencilState *DepthStencilState;
	CCoreRasterizerState *RasterizerState;

	CDictionary<CORESAMPLER, CCoreSamplerState*> SamplerStates;

	TS32 ConstantBufferIndices[CORESHADERTYPECOUNT][COREBUFFERSCOPECOUNT];
	//void SetConstantBufferIndices(CORESHADERTYPE s, CCoreShader *sh);

public:

	CCoreMaterialRenderPass(CCoreDevice *Device);
	virtual ~CCoreMaterialRenderPass();

	CCoreAtom *SpawnAtom(CORERENDERLAYERID TargetLayer, CCoreMesh *Mesh, CCoreConstantBuffer *Buffers[COREBUFFERSCOPECOUNT]);
	void SetName(CString &s);

	void Apply();

	TBOOL Import(CXMLNode *n);
	void Export(CXMLNode *Node);

	CCorePixelShader *GetPixelShader() { return PixelShader; }
	CCoreVertexShader *GetVertexShader() { return VertexShader; }
	CCoreGeometryShader *GetGeometryShader() { return GeometryShader; }
};

class CCoreMaterialTechnique
{
	CString Name;
	CORERENDERLAYERID TargetLayer;

	CArray<CCoreMaterialRenderPass*> Passes;
	CArray<CCoreMaterialParameter*> Inputs;

public:

	CCoreMaterialTechnique();
	virtual ~CCoreMaterialTechnique();

	CString &GetName();
	void SetName(CString &name);

	TBOOL HasParameters(COREBUFFERSCOPE Scope);
	void AddParameter(CString &Name, COREMATERIALPARAMETERSCOPE Scope, COREMATERIALPARAMETERTYPE Type, TS32 ValueSize, void *Value);
	void AddPass(CCoreMaterialRenderPass *Pass);
	void GatherData(COREBUFFERSCOPE TargetBuffer, CCoreConstantBuffer *Buffer, CCoreObjectGroup *Group);
	void CreateAtoms(CArray<CCoreAtom*> &Atoms, CCoreMesh *Mesh, CCoreConstantBuffer *Buffers[COREBUFFERSCOPECOUNT]);

	TBOOL Import(CXMLNode *Root, CCoreDevice *Device);
	void Export(CXMLNode *Node);

	CORERENDERLAYERID GetTargetLayer();
};

class CCoreMaterial
{

	CArray<CCoreMaterialTechnique*> Techniques;

public:

	TS32 GetTechCount();
	CCoreMaterialTechnique *GetTech(TS32 x);
	void GatherData(CCoreDevice *Device, COREBUFFERSCOPE Scope, CCoreObjectGroup *Group, CDictionary<CCoreMaterialTechnique*, CCoreConstantBuffer*> *Buffers[3], CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers);
	void CreateAtoms(CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers, CCoreConstantBuffer *SceneBuffer, CCoreConstantBuffer *ObjectBuffer, CDictionary<CCoreMaterialTechnique*, CCoreConstantBuffer*> *Buffers[3], CArray<CCoreAtom*> &Atoms, CCoreMesh *Mesh);
};