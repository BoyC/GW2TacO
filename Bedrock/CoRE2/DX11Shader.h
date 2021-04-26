#pragma once
#include "DX11Device.h"
#include "Shader.h"

#ifdef CORE_API_DX11

class CCoreDX11VertexShader : public CCoreVertexShader
{
	ID3D11Device *Dev;
	ID3D11DeviceContext *DeviceContext;
	ID3D11VertexShader *VertexShaderHandle;
	//ID3D11ShaderReflection *Reflection;

	virtual void Release();
	virtual TBOOL Apply();

public:

	CCoreDX11VertexShader(CCoreDX11Device *Device);
	virtual ~CCoreDX11VertexShader();

	virtual TBOOL Create(void *Binary, TS32 Length);
	virtual TBOOL CompileAndCreate(CString *Err);
  virtual TBOOL CreateFromBlob( void *Code, TS32 CodeSize );
  //virtual TS32 GetConstantBufferIndex(TS8 *Name);
	virtual void *GetHandle() { return VertexShaderHandle; }
};

class CCoreDX11PixelShader : public CCorePixelShader
{
	ID3D11Device *Dev;
	ID3D11DeviceContext *DeviceContext;
	ID3D11PixelShader *PixelShaderHandle;
	//ID3D11ShaderReflection *Reflection;

	virtual void Release();
	virtual TBOOL Apply();

public:

	CCoreDX11PixelShader(CCoreDX11Device *Device);
	virtual ~CCoreDX11PixelShader();

	virtual TBOOL Create(void *Binary, TS32 Length);
	virtual TBOOL CompileAndCreate(CString *Err);
  virtual TBOOL CreateFromBlob( void *Code, TS32 CodeSize );
  //virtual TS32 GetConstantBufferIndex(TS8 *Name);
	virtual void *GetHandle() { return PixelShaderHandle; }
};

class CCoreDX11GeometryShader : public CCoreGeometryShader
{
	ID3D11Device *Dev;
	ID3D11DeviceContext *DeviceContext;
	ID3D11GeometryShader *GeometryShaderHandle;
	//ID3D11ShaderReflection *Reflection;

	virtual void Release();
	virtual TBOOL Apply();

public:

	CCoreDX11GeometryShader(CCoreDX11Device *Device);
	virtual ~CCoreDX11GeometryShader();

	virtual TBOOL Create(void *Binary, TS32 Length);
	virtual TBOOL CompileAndCreate(CString *Err);
  virtual TBOOL CreateFromBlob( void *Code, TS32 CodeSize );
  //virtual TS32 GetConstantBufferIndex(TS8 *Name);
	virtual void *GetHandle() { return GeometryShaderHandle; }
};

class CCoreDX11HullShader : public CCoreHullShader
{
	ID3D11Device *Dev;
	ID3D11DeviceContext *DeviceContext;
	ID3D11HullShader *HullShaderHandle;
	//ID3D11ShaderReflection *Reflection;

	virtual void Release();
	virtual TBOOL Apply();

public:

	CCoreDX11HullShader(CCoreDX11Device *Device);
	virtual ~CCoreDX11HullShader();

	virtual TBOOL Create(void *Binary, TS32 Length);
	virtual TBOOL CompileAndCreate(CString *Err);
  virtual TBOOL CreateFromBlob( void *Code, TS32 CodeSize );
  //virtual TS32 GetConstantBufferIndex(TS8 *Name);
	virtual void *GetHandle() { return HullShaderHandle; }
};

class CCoreDX11DomainShader : public CCoreDomainShader
{
	ID3D11Device *Dev;
	ID3D11DeviceContext *DeviceContext;
	ID3D11DomainShader *DomainShaderHandle;
	//ID3D11ShaderReflection *Reflection;

	virtual void Release();
	virtual TBOOL Apply();

public:

	CCoreDX11DomainShader(CCoreDX11Device *Device);
	virtual ~CCoreDX11DomainShader();

	virtual TBOOL Create(void *Binary, TS32 Length);
	virtual TBOOL CompileAndCreate(CString *Err);
  virtual TBOOL CreateFromBlob( void *Code, TS32 CodeSize );
  //virtual TS32 GetConstantBufferIndex(TS8 *Name);
	virtual void *GetHandle() { return DomainShaderHandle; }
};

class CCoreDX11ComputeShader : public CCoreComputeShader
{
	ID3D11Device *Dev;
	ID3D11DeviceContext *DeviceContext;
	ID3D11ComputeShader *ComputeShaderHandle;
	//ID3D11ShaderReflection *Reflection;

	virtual void Release();
	virtual TBOOL Apply();

public:

	CCoreDX11ComputeShader(CCoreDX11Device *Device);
	virtual ~CCoreDX11ComputeShader();

	virtual TBOOL Create(void *Binary, TS32 Length);
	virtual TBOOL CompileAndCreate(CString *Err);
  virtual TBOOL CreateFromBlob( void *Code, TS32 CodeSize );
  //virtual TS32 GetConstantBufferIndex(TS8 *Name);
	virtual void *GetHandle() { return ComputeShaderHandle; }
};

#endif