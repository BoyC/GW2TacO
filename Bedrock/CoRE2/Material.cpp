#include "BasePCH.h"
#include "Material.h"
#include "Device.h"

//////////////////////////////////////////////////////////////////////////
// enum names

EnumNamePair MaterialParameterScopeNames[] =
{
	{ CORE_MATERIAL_CONSTANT, _T("Constant") },
	{ CORE_MATERIAL_VARIABLE, _T("Variable") },
	{ CORE_MATERIAL_ANIMATED, _T("Animated") },
	{ CORE_MATERIAL_EXTERNAL, _T("External") },

	{ -1, NULL } //leave this here
};

EnumNamePair MaterialParameterTypeNames[] =
{
	{ CORE_MATERIAL_FLOAT, _T("Float") },

	{ -1, NULL } //leave this here
};


//////////////////////////////////////////////////////////////////////////
// material param

CCoreMaterialParameter::CCoreMaterialParameter()
{
	Value = NULL;
	ValueSize = 0;
}

CCoreMaterialParameter::~CCoreMaterialParameter()
{
	SAFEDELETE(Value);
}

void CCoreMaterialParameter::SetData(TS32 Size, void *Data)
{
	SAFEDELETE(Value);
	ValueSize = Size;
	if (!Size) Value = NULL;
	Value = new TU8[Size];
	memcpy(Value, Data, Size);
}

void CCoreMaterialParameter::SetType(COREMATERIALPARAMETERTYPE t)
{
	Type = t;
}

void CCoreMaterialParameter::SetScope(COREMATERIALPARAMETERSCOPE s)
{
	Scope = s;
}

void CCoreMaterialParameter::SetName(CString &s)
{
	Name = s;
}

INLINE CString & CCoreMaterialParameter::GetName()
{
	return Name;
}

INLINE TS32 & CCoreMaterialParameter::GetDataSize()
{
	return ValueSize;
}

INLINE TU8 *& CCoreMaterialParameter::GetData()
{
	return Value;
}

INLINE COREMATERIALPARAMETERSCOPE & CCoreMaterialParameter::GetScope()
{
	return Scope;
}

void CCoreMaterialParameter::Export(CXMLNode *Node)
{
	Node->SetAttribute(_T("Name"), Name.GetPointer());
	Node->AddChild(_T("Scope"), false).SetText(FindNameByEnum(MaterialParameterScopeNames, Scope));
	Node->AddChild(_T("Type")).SetText(FindNameByEnum(MaterialParameterTypeNames, Type));
}

TBOOL CCoreMaterialParameter::Import(CXMLNode *Root)
{
	if (!Root->GetChildCount(_T("Name")))
	{
		LOG_ERR("[core] Error importing Material Tech Input: Missing Name");
		return false;
	}

	CString Name = Root->GetAttributeAsString(_T("Name"));

	if (!Root->GetChildCount(_T("Scope")))
	{
		LOG_ERR("[core] Error importing Material Tech Input '%s': Scope missing", Name.GetPointer());
		return false;
	}

	CString Scope = Root->GetChild(_T("Scope")).GetText();
	COREMATERIALPARAMETERSCOPE eScope;

	if (!FindEnumByName(MaterialParameterScopeNames, Scope, (TS32&)eScope))
	{
		LOG_ERR("[core] Error importing Material Tech Input '%s': Scope value '%s' unknown", Name.GetPointer(), Scope.GetPointer());
		return false;
	}

	if (!Root->GetChildCount(_T("Type")))
	{
		LOG_ERR("[core] Error importing Material Tech Input '%s': Type missing", Name.GetPointer());
		return false;
	}

	CString Type = Root->GetChild(_T("Type")).GetText();
	COREMATERIALPARAMETERTYPE eType;

	if (!FindEnumByName(MaterialParameterTypeNames, Type, (TS32&)eType))
	{
		LOG_ERR("[core] Error importing Material Tech Input '%s': Type value '%s' unknown", Name.GetPointer(), Type.GetPointer());
		return false;
	}

	SetName(Name);
	SetScope(eScope);
	SetType(eType);
	return true;
}

//////////////////////////////////////////////////////////////////////////
// render pass

CCoreAtom * CCoreMaterialRenderPass::SpawnAtom(CORERENDERLAYERID TargetLayer, CCoreMesh *Mesh, CCoreConstantBuffer *Buffers[COREBUFFERSCOPECOUNT])
{
	CCoreAtom *a = new CCoreAtom(TargetLayer, this, Buffers, Mesh);
	return a;
}

CCoreMaterialRenderPass::CCoreMaterialRenderPass(CCoreDevice *Device)
{
	Dev = Device;

	for (TS32 x = 0; x < COREBUFFERSCOPECOUNT; x++)
		for (TS32 y = 0; y < 3; y++)
			ConstantBufferIndices[y][x] = -1;

	PixelShader = Device->CreatePixelShader();
	VertexShader = Device->CreateVertexShader();
	GeometryShader = Device->CreateGeometryShader();
	BlendState = Device->CreateBlendState();
	DepthStencilState = Device->CreateDepthStencilState();
	RasterizerState = Device->CreateRasterizerState();
}

CCoreMaterialRenderPass::~CCoreMaterialRenderPass()
{
	SAFEDELETE(GeometryShader);
	SAFEDELETE(PixelShader);
	SAFEDELETE(VertexShader);
	SAFEDELETE(BlendState);
	SAFEDELETE(DepthStencilState);
	SAFEDELETE(RasterizerState);

	SamplerStates.FreeAll();
}

void CCoreMaterialRenderPass::SetName(CString &s)
{
	Name = s;
}

void CCoreMaterialRenderPass::Apply()
{
	Dev->SetRenderState(BlendState);
	Dev->SetRenderState(RasterizerState);
	Dev->SetRenderState(DepthStencilState);
	Dev->SetPixelShader(PixelShader);
	Dev->SetVertexShader(VertexShader);
	Dev->SetGeometryShader(GeometryShader);

	for (TS32 x = 0; x < SamplerStates.NumItems(); x++)
	{
		CDictionary<CORESAMPLER, CCoreSamplerState*>::KDPair *kp = SamplerStates.GetKDPair(x);
		Dev->SetSamplerState(kp->Key, kp->Data);
	}
}

void CCoreMaterialRenderPass::Export(CXMLNode *Node)
{
	Node->SetAttribute(_T("Name"), Name.GetPointer());
	CXMLNode n = Node->AddChild(_T("BlendState"));
	BlendState->Export(&n);
	n = Node->AddChild(_T("DepthStencilState"));
	DepthStencilState->Export(&n);
	n = Node->AddChild(_T("RasterizerState"));
	RasterizerState->Export(&n);
	for (TS32 x = 0; x < SamplerStates.NumItems(); x++)
	{
		n = Node->AddChild(_T("SamplerState"));
		SamplerStates.GetByIndex(x)->Export(&n);
	}
	Node->AddChild(_T("ShaderCode")).SetText(GetVertexShader()->GetCode().GetPointer());
}

TBOOL CCoreMaterialRenderPass::Import(CXMLNode *Root)
{
	TBOOL Success = true;

	if (Root->GetChildCount(_T("Name")))
		SetName(Root->GetAttributeAsString(_T("Name")));

	if (Root->GetChildCount(_T("BlendState")))
	{
		CXMLNode n = Root->GetChild(_T("BlendState"));
		Success &= BlendState->Import(&n);
	}

	if (Root->GetChildCount(_T("DepthStencilState")))
	{
		CXMLNode n = Root->GetChild(_T("DepthStencilState"));
		Success &= DepthStencilState->Import(&n);
	}

	if (Root->GetChildCount(_T("RasterizerState")))
	{
		CXMLNode n = Root->GetChild(_T("RasterizerState"));
		Success &= RasterizerState->Import(&n);
	}

	for (TS32 x = 0; x < Root->GetChildCount(_T("SamplerState")); x++)
	{
		CXMLNode n = Root->GetChild(_T("SamplerState"), x);

		if (!n.GetChildCount(_T("Sampler"))) Success = false;

		if (Success)
		{
			CString sampler = n.GetAttributeAsString(_T("Sampler"));
			CORESAMPLER s;
			if (!FindEnumByName(SamplerNames, sampler, (TS32&)s)) Success = false;
			if (Success)
			{
				if (SamplerStates.HasKey(s)) SamplerStates.Free(s);
				CCoreSamplerState *ss = Dev->CreateSamplerState();
				SamplerStates[s] = ss;
				Success &= ss->Import(&n);
			}
		}
	}

	if (Root->GetChildCount(_T("ShaderCode")))
	{
		CString sh = Root->GetAttributeAsString(_T("ShaderCode"));
		GetVertexShader()->SetCode(sh, CString(_T("vsmain")), CString(_T("vs_5_0")));
		GetVertexShader()->CompileAndCreate(NULL);
		//SetConstantBufferIndices(CORE_SHADER_VERTEX, GetVertexShader());
		GetPixelShader()->SetCode(sh, CString(_T("psmain")), CString(_T("ps_5_0")));
		GetPixelShader()->CompileAndCreate(NULL);
		//SetConstantBufferIndices(CORE_SHADER_PIXEL, GetPixelShader());
		GetGeometryShader()->SetCode(sh, CString(_T("gsmain")), CString(_T("gs_5_0")));
		GetGeometryShader()->CompileAndCreate(NULL);
		//SetConstantBufferIndices(CORE_SHADER_GEOMETRY, GetGeometryShader());
	}

	return Success;
}

//void CCoreMaterialRenderPass::SetConstantBufferIndices(CORESHADERTYPE s, CCoreShader *sh)
//{
//	if (!sh)
//	{
//		for (TS32 x = 0; x < COREBUFFERSCOPECOUNT; x++)
//			ConstantBufferIndices[s][x] = -1;
//		return;
//	}
//
//	ConstantBufferIndices[s][CORE_BUFFER_SCENE] = sh->GetConstantBufferIndex("SceneData");
//	ConstantBufferIndices[s][CORE_BUFFER_LAYER] = sh->GetConstantBufferIndex("LayerData");
//	ConstantBufferIndices[s][CORE_BUFFER_OBJECT] = sh->GetConstantBufferIndex("ObjectData");
//	ConstantBufferIndices[s][CORE_BUFFER_GAMEDATA] = sh->GetConstantBufferIndex("GameData");
//	ConstantBufferIndices[s][CORE_BUFFER_TECHSTATIC] = sh->GetConstantBufferIndex("StaticTechData");
//	ConstantBufferIndices[s][CORE_BUFFER_TECHDYNAMIC] = sh->GetConstantBufferIndex("DynamicTechData");
//}

//////////////////////////////////////////////////////////////////////////
// material tech

CCoreMaterialTechnique::CCoreMaterialTechnique()
{

}

CCoreMaterialTechnique::~CCoreMaterialTechnique()
{
	Passes.FreeArray();
	Inputs.FreeArray();
}

void CCoreMaterialTechnique::GatherData(COREBUFFERSCOPE Target, CCoreConstantBuffer *Buffer, CCoreObjectGroup *Group)
{
	Buffer->Reset();

	for (TS32 x = 0; x < Inputs.NumItems(); x++)
	{
		TBOOL Gather = false;
		CCoreMaterialParameter *i = Inputs[x];

		switch (Target)
		{
			case CORE_BUFFER_TECHSTATIC:
				Gather = i->GetScope() != CORE_MATERIAL_ANIMATED && i->GetScope() != CORE_MATERIAL_EXTERNAL;
				break;
			case CORE_BUFFER_TECHDYNAMIC:
				Gather = i->GetScope() == CORE_MATERIAL_ANIMATED;
				break;
			case CORE_BUFFER_GAMEDATA:
				if (i->GetScope() == CORE_MATERIAL_EXTERNAL)
				{
					//special case
					TF32 value = 0;
					if (Group)
					{
						if (!Group->GetData(i->GetName(), value))
							LOG_WARN("[core] Object Group material parameter %s not set, defaulting to 0", i->GetName().GetPointer());
					}
					Buffer->AddData(&value, sizeof(TF32));
				}
				break;
		}

		if (Gather) Buffer->AddData(i->GetData(), i->GetDataSize());
	}

	Buffer->Upload();
}

TBOOL CCoreMaterialTechnique::HasParameters(COREBUFFERSCOPE Scope)
{
	for (TS32 x = 0; x < Inputs.NumItems(); x++)
	{
		CCoreMaterialParameter *i = Inputs[x];
		switch (Scope)
		{
			case CORE_BUFFER_TECHSTATIC:
				if (i->GetScope() != CORE_MATERIAL_ANIMATED && i->GetScope() != CORE_MATERIAL_EXTERNAL) return true;
				break;
			case CORE_BUFFER_TECHDYNAMIC:
				if (i->GetScope() == CORE_MATERIAL_ANIMATED) return true;
				break;
			case CORE_BUFFER_GAMEDATA:
				if (i->GetScope() == CORE_MATERIAL_EXTERNAL) return true;
				break;
		}
	}

	return false;
}

void CCoreMaterialTechnique::CreateAtoms(CArray<CCoreAtom*> &Atoms, CCoreMesh *Mesh, CCoreConstantBuffer *Buffers[COREBUFFERSCOPECOUNT])
{
	for (TS32 x = 0; x < Passes.NumItems(); x++)
		Atoms += Passes[x]->SpawnAtom(TargetLayer, Mesh, Buffers);
}

void CCoreMaterialTechnique::AddParameter(CString &nme, COREMATERIALPARAMETERSCOPE Scope, COREMATERIALPARAMETERTYPE Type, TS32 ValueSize, void *Value)
{
	CCoreMaterialParameter *p = new CCoreMaterialParameter();
	Inputs += p;
	Inputs.Last()->SetName(nme);
	Inputs.Last()->SetScope(Scope);
	Inputs.Last()->SetType(Type);
	Inputs.Last()->SetData(ValueSize, Value);
}

CString & CCoreMaterialTechnique::GetName()
{
	return Name;
}

void CCoreMaterialTechnique::SetName(CString &name)
{
	Name = name;
}

CORERENDERLAYERID CCoreMaterialTechnique::GetTargetLayer()
{
	return TargetLayer;
}

TBOOL CCoreMaterialTechnique::Import(CXMLNode *Root, CCoreDevice *Device)
{
	if (!Root->GetChildCount(_T("Name")))
	{
		LOG_ERR("[core] Error importing Material Technique: Missing Name");
		return false;
	}

	CString Name = Root->GetAttributeAsString(_T("Name"));

	SetName(Name);

	//import passes

	TS32 passcount = Root->GetChildCount(_T("RenderPass"));
	if (!passcount)
		LOG_WARN("[core] Material Technique '%s' has no renderpasses and will do nothing", Name.GetPointer());

	for (TS32 x = 0; x < passcount; x++)
	{
		CCoreMaterialRenderPass *p = new CCoreMaterialRenderPass(Device);
		if (!p->Import(&Root->GetChild(_T("RenderPass"), x)))
			delete p;
		else
			Passes += p;
	}

	//import inputs

	TS32 inputcount = Root->GetChildCount(_T("InputParameter"));
	for (TS32 x = 0; x < inputcount; x++)
	{
		CCoreMaterialParameter *p = new CCoreMaterialParameter();
		if (!p->Import(&Root->GetChild(_T("InputParameter"), x)))
			delete p;
		else
		{
			for (TS32 y = 0; y < Inputs.NumItems(); y++)
				if (Inputs[y]->GetName() == p->GetName())
				{
					LOG_WARN("[core] Material Tech Input name '%s' not unique", Name.GetPointer());
					break;
				}
			Inputs += p;
		}
	}

	return true;
}

void CCoreMaterialTechnique::Export(CXMLNode *Node)
{
	Node->SetAttribute(_T("Name"), Name.GetPointer());
	for (TS32 x = 0; x < Passes.NumItems(); x++)
	{
		CXMLNode n = Node->AddChild(_T("RenderPass"));
		Passes[x]->Export(&n);
	}
	for (TS32 x = 0; x < Inputs.NumItems(); x++)
	{
		CXMLNode n = Node->AddChild(_T("InputParameter"));
		Inputs[x]->Export(&n);
	}
}

void CCoreMaterialTechnique::AddPass(CCoreMaterialRenderPass *Pass)
{
	Passes += Pass;
}

//////////////////////////////////////////////////////////////////////////
// material

void CCoreMaterial::GatherData(CCoreDevice *Device, COREBUFFERSCOPE Scope, CCoreObjectGroup *Group, CDictionary<CCoreMaterialTechnique*, CCoreConstantBuffer*> *Buffers[3], CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers)
{
	TS32 TargetBuffer = -1;
	if (Scope == CORE_BUFFER_GAMEDATA) TargetBuffer = 0;
	if (Scope == CORE_BUFFER_TECHSTATIC) TargetBuffer = 1;
	if (Scope == CORE_BUFFER_TECHDYNAMIC) TargetBuffer = 2;
	if (TargetBuffer == -1) return;

	for (TS32 x = 0; x < Techniques.NumItems(); x++)
	{
		CCoreMaterialTechnique *tech = Techniques[x];

		if (!Layers.HasKey(tech->GetTargetLayer())) continue;
		if (!tech->HasParameters(Scope)) continue;

		CCoreConstantBuffer *Buf = Buffers[TargetBuffer]->GetExisting(tech);
		if (!Buf) Buf = (*Buffers[TargetBuffer])[tech] = Device->CreateConstantBuffer();

		tech->GatherData(Scope, Buf, Group);
	}
}

void CCoreMaterial::CreateAtoms(CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers, CCoreConstantBuffer *SceneBuffer, CCoreConstantBuffer *ObjectBuffer, CDictionary<CCoreMaterialTechnique*, CCoreConstantBuffer*> *Buffers[3], CArray<CCoreAtom*> &Atoms, CCoreMesh *Mesh)
{
	for (TS32 x = 0; x < Techniques.NumItems(); x++)
	{
		CCoreRenderLayer *Layer = Layers.GetExisting(Techniques[x]->GetTargetLayer());
		if (!Layer) continue;

		CCoreConstantBuffer *Buf[COREBUFFERSCOPECOUNT];

		Buf[CORE_BUFFER_SCENE] = SceneBuffer;
		Buf[CORE_BUFFER_LAYER] = Layer->GetConstantBuffer();
		Buf[CORE_BUFFER_OBJECT] = ObjectBuffer;
		Buf[CORE_BUFFER_GAMEDATA] = Buffers[0]->GetExisting(Techniques[x]);
		Buf[CORE_BUFFER_TECHSTATIC] = Buffers[1]->GetExisting(Techniques[x]);
		Buf[CORE_BUFFER_TECHDYNAMIC] = Buffers[2]->GetExisting(Techniques[x]);

		//for (TS32 y=0; y<3; y++)
		//	Buf[y+1]=Buffers[y]->GetExisting(Techniques[x]);

		Techniques[x]->CreateAtoms(Atoms, Mesh, Buf);
	}
}

CCoreMaterialTechnique * CCoreMaterial::GetTech(TS32 x)
{
	return Techniques[x];
}

TS32 CCoreMaterial::GetTechCount()
{
	return Techniques.NumItems();
}
