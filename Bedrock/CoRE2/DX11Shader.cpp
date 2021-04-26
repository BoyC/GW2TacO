#include "BasePCH.h"
#include "DX11Shader.h"
#ifdef CORE_API_DX11

//////////////////////////////////////////////////////////////////////////
// this function is just a test to see how shader text and compiled
// shader sizes compare

//TS32 shadercnt = 0;
//void DumpShader(CString s, void *Binary, TS32 Length)
//{
//	CreateDirectory(_T("ShaderDump"), 0);
//
//	CString fname1 = CString::Format(_T("ShaderDump/%.3d.hlsl"), shadercnt);
//	CString fname2 = CString::Format(_T("ShaderDump/%.3d.bin"), shadercnt);
//
//	CStreamWriterFile f1;
//	f1.Open(fname1.GetPointer());
//	f1.Write(s.GetPointer(), s.Length());
//
//	CStreamWriterFile f2;
//	f2.Open(fname2.GetPointer());
//	f2.Write(Binary,Length);
//
//	shadercnt++;
//}

typedef HRESULT(__stdcall d3d_compile_func)(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName, const D3D_SHADER_MACRO *pDefines, ID3DInclude *pInclude, LPCSTR pEntrypoint, LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob **ppCode, ID3DBlob **ppErrorMsgs);
typedef HRESULT(__stdcall d3d_reflect_func)(LPCVOID pSrcData, SIZE_T SrcDataSize, REFIID pInterface, void** ppReflector);

typedef HRESULT( __stdcall D3DXCompileShader( LPCSTR pSrcData, UINT srcDataLen, const void *pDefines, void *pInclude, LPCSTR pFunctionName, LPCSTR pProfile, DWORD Flags, void **ppShader, void **ppErrorMsgs, void **ppConstantTable ) );

d3d_compile_func *D3DCompileFunc = NULL;
D3DXCompileShader *D3DXCompileFunc = NULL;

void *GetFunctionFromD3DXDLL( TCHAR *FunctName )
{
  HMODULE dll = NULL;

  CString fname = FunctName;
  TS8 *funcname = new TS8[fname.Length() + 1];
  fname.WriteAsMultiByte( funcname, fname.Length() + 1 );

  TCHAR *CompilerDLLs[] =
  {
    "d3dx11_47.dll",
    "d3dx11_46.dll",
    "d3dx11_45.dll",
    "d3dx11_44.dll",
    "d3dx11_43.dll",
    "d3dx11_42.dll",
    "d3dx11_41.dll",
    "d3dx11_40.dll",
    "d3dx11_39.dll",
    "d3dx11_38.dll",
    "d3dx11_37.dll",
    "d3dx11_36.dll",
    "d3dx11_35.dll",
    "d3dx11_34.dll",
    "d3dx11_33.dll",
    "d3dx11_32.dll",
    "d3dx11_31.dll",
    "d3dx10_47.dll",
    "d3dx10_46.dll",
    "d3dx10_45.dll",
    "d3dx10_44.dll",
    "d3dx10_43.dll",
    "d3dx10_42.dll",
    "d3dx10_41.dll",
    "d3dx10_40.dll",
    "d3dx10_39.dll",
    "d3dx10_38.dll",
    "d3dx10_37.dll",
    "d3dx10_36.dll",
    "d3dx10_35.dll",
    "d3dx10_34.dll",
    "d3dx10_33.dll",
    "d3dx10_32.dll",
    "d3dx10_31.dll",
  };

  for ( TS32 x = 0; x < sizeof( CompilerDLLs ) / sizeof( void* ); x++ )
  {
    dll = LoadLibraryA( CompilerDLLs[x] );
    if ( dll )
    {
      void *func = GetProcAddress( dll, funcname );
      if ( func )
      {
        LOG_NFO( "[core] Successfully loaded %s from %s", FunctName, CompilerDLLs[x] );
        SAFEDELETEA( funcname );
        return func;
      }
    }
  }

  LOG_ERR( "[core] Failed to load %s from d3dx**_xx.dll!", FunctName );
  SAFEDELETEA( funcname );
  return NULL;
}


void *GetFunctionFromD3DCompileDLL(TCHAR *FunctName)
{
	HMODULE dll = NULL;

	CString fname = FunctName;
	TS8 *funcname = new TS8[fname.Length() + 1];
	fname.WriteAsMultiByte(funcname, fname.Length() + 1);

	TCHAR *CompilerDLLs[]=
	{
		"d3dcompiler_47.dll",
		"d3dcompiler_46.dll",
		"d3dcompiler_45.dll",
		"d3dcompiler_44.dll",
		"d3dcompiler_43.dll",
		"d3dcompiler_42.dll",
		"d3dcompiler_41.dll",
		"d3dcompiler_40.dll",
		"d3dcompiler_39.dll",
		"d3dcompiler_38.dll",
		"d3dcompiler_37.dll",
		"d3dcompiler_36.dll",
		"d3dcompiler_35.dll",
		"d3dcompiler_34.dll",
		"d3dcompiler_33.dll"
	};

	for (TS32 x = 0; x < sizeof(CompilerDLLs) / sizeof(void*); x++)
	{
		dll = LoadLibraryA(CompilerDLLs[x]);
		if (dll)
		{
			void *func = GetProcAddress(dll, funcname);
			if (func)
			{
				LOG_NFO("[core] Successfully loaded %s from %s", FunctName, CompilerDLLs[x]);
				SAFEDELETEA(funcname);
				return func;
			}
		}
	}

	//dll = LoadLibraryA("d3dcompiler_47.dll");
	//if (dll)
	//{
	//	void *func = GetProcAddress(dll, funcname);
	//	if (func)
	//	{
	//		LOG_NFO("[core] Successfully loaded %s from d3dcompiler_43.dll", FunctName);
	//		SAFEDELETEA(funcname);
	//		return func;
	//	}
	//}

	//dll = LoadLibraryA("d3dcompiler_46.dll");
	//if (dll)
	//{
	//	void *func = GetProcAddress(dll, funcname);
	//	if (func)
	//	{
	//		LOG_NFO("[core] Successfully loaded %s from d3dcompiler_46.dll", FunctName);
	//		SAFEDELETEA(funcname);
	//		return func;
	//	}
	//}

	//dll = LoadLibraryA("d3dcompiler_43.dll");
	//if (dll)
	//{
	//	void *func = GetProcAddress(dll, funcname);
	//	if (func)
	//	{
	//		LOG_NFO("[core] Successfully loaded %s from d3dcompiler_47.dll", FunctName);
	//		SAFEDELETEA(funcname);
	//		return func;
	//	}
	//}

	LOG_ERR("[core] Failed to load %s from d3dcompile_xx.dll!", FunctName);
	SAFEDELETEA(funcname);
	return NULL;
}

TBOOL InitShaderCompiler()
{
	if (D3DCompileFunc || D3DXCompileFunc) return true;
	D3DCompileFunc = (d3d_compile_func *)GetFunctionFromD3DCompileDLL(_T("D3DCompile"));
  //if ( !D3DCompileFunc )
  //{
  //  D3DXCompileFunc = (D3DXCompileShader*)GetFunctionFromD3DXDLL( _T( "D3DXCompileShader" ) );
  //  if ( D3DXCompileFunc )
  //    return true;
  //}
  return D3DCompileFunc != NULL;
}

//////////////////////////////////////////////////////////////////////////
// vertex shader

CCoreDX11VertexShader::CCoreDX11VertexShader(CCoreDX11Device *dev) : CCoreVertexShader(dev)
{
	Dev = dev->GetDevice();
	DeviceContext = dev->GetDeviceContext();
	VertexShaderHandle = NULL;
	//Reflection = NULL;
}

CCoreDX11VertexShader::~CCoreDX11VertexShader()
{
	Release();
}

TBOOL CCoreDX11VertexShader::Create(void *Binary, TS32 Length)
{
	if (!Binary || Length <= 0) return false;
	FetchBinary(Binary, Length);
	HRESULT res = Dev->CreateVertexShader(Binary, Length, NULL, &VertexShaderHandle);
	if (res != S_OK)
	{
		_com_error err(res);
		LOG(LOG_ERROR, _T("[core] VertexShader Creation error (%s)"), err.ErrorMessage());
	}
	return res == S_OK;
}

void CCoreDX11VertexShader::Release()
{
	//if (Reflection)
	//	Reflection->Release();
	//Reflection = NULL;
	if (VertexShaderHandle)
		VertexShaderHandle->Release();
	VertexShaderHandle = NULL;
}

TBOOL CCoreDX11VertexShader::Apply()
{
	if (!VertexShaderHandle) return false;
	DeviceContext->VSSetShader(VertexShaderHandle, NULL, 0);
	return true;
}

TBOOL CCoreDX11VertexShader::CompileAndCreate(CString *Err)
{
	if (!D3DCompileFunc && !InitShaderCompiler()) return false;

	Release();

	TBOOL Success = true;

	ID3D10Blob *PS = NULL;
	ID3D10Blob *Error = NULL;

	TS8 *code = new TS8[Code.Length() + 2];
	memset(code, 0, Code.Length() + 2);
	Code.WriteAsMultiByte(code, Code.Length() + 1);

	TS8 *entry = new TS8[EntryFunction.Length() + 2];
	memset(entry, 0, EntryFunction.Length() + 2);
	EntryFunction.WriteAsMultiByte(entry, EntryFunction.Length() + 1);

	TS8 *version = new TS8[ShaderVersion.Length() + 2];
	memset(version, 0, ShaderVersion.Length() + 2);
	ShaderVersion.WriteAsMultiByte(version, ShaderVersion.Length() + 1);

	TU32 tmp;
	_controlfp_s(&tmp, _RC_NEAR, _MCW_RC);

#ifdef CORE_API_D3DX
	if (D3DX11CompileFromMemory(code, strlen(code), NULL, NULL, NULL, entry, version, D3DCOMPILE_OPTIMIZATION_LEVEL0, 0, NULL, &PS, &Error, 0) != S_OK)
#else
	if (D3DCompileFunc(code, strlen(code), NULL, NULL, NULL, entry, version, 0, 0, &PS, &Error) != S_OK)
#endif
	{
		if (!Err)
			LOG(LOG_ERROR, _T("[core] VertexShader compilation error: %s"), CString((char*)Error->GetBufferPointer()).GetPointer());
		Success = false;
	}

	if (Err) *Err = Error ? CString((char*)Error->GetBufferPointer()) : _T("");

	if (Success)
	{
    //FILE *f = fopen( "vxshader.bin", "w+b" );
    //fwrite( PS->GetBufferPointer(), PS->GetBufferSize(), 1, f );
    //fclose( f );
		//DumpShader(code, PS->GetBufferPointer(), PS->GetBufferSize());
		Success = Create(PS->GetBufferPointer(), (TS32)PS->GetBufferSize());
		//if (Success)
		//{
		//	HRESULT r = D3DReflectFunc(PS->GetBufferPointer(), PS->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&Reflection);
		//	if (r != S_OK)
		//	{
		//		Success = false;
		//		_com_error err(r);
		//		LOG_ERR("[core] Failed to acquire VertexShader reflection (%s)", err.ErrorMessage());
		//	}
		//}
		PS->Release();
	}

#ifndef _WIN64
	_controlfp_s(&tmp, tmp, 0xffffffff);
#endif

	delete[] code;
	delete[] entry;
	delete[] version;

	return Success;
}

TBOOL CCoreDX11VertexShader::CreateFromBlob( void *CodeBlob, TS32 CodeBlobSize )
{
  Release();
  TBOOL Success = true;
  TU32 tmp;
  _controlfp_s( &tmp, _RC_NEAR, _MCW_RC );
  Success = Create( CodeBlob, CodeBlobSize );
#ifndef _WIN64
  _controlfp_s( &tmp, tmp, 0xffffffff );
#endif
  return Success;
}

//TS32 CCoreDX11VertexShader::GetConstantBufferIndex(TS8 *Name)
//{
//	if (!Reflection) return -1;
//
//	CString s = Name;
//
//	for (TS32 x = 0; x < 15; x++)
//	{
//		ID3D11ShaderReflectionConstantBuffer *cb = Reflection->GetConstantBufferByIndex(x);
//		if (cb)
//		{
//			D3D11_SHADER_BUFFER_DESC Desc;
//			if (cb->GetDesc(&Desc) == S_OK)
//				if (s == Desc.Name) return x;
//		}
//	}
//
//	return -1;
//}

//////////////////////////////////////////////////////////////////////////
// Pixel shader

CCoreDX11PixelShader::CCoreDX11PixelShader(CCoreDX11Device *dev) : CCorePixelShader(dev)
{
	Dev = dev->GetDevice();
	DeviceContext = dev->GetDeviceContext();
	PixelShaderHandle = NULL;
	//Reflection = NULL;
}

CCoreDX11PixelShader::~CCoreDX11PixelShader()
{
	Release();
}

TBOOL CCoreDX11PixelShader::Create(void *Binary, TS32 Length)
{
	if (!Binary || Length <= 0) return false;
	FetchBinary(Binary, Length);
	HRESULT res = Dev->CreatePixelShader(Binary, Length, NULL, &PixelShaderHandle);
	if (res != S_OK)
	{
		_com_error err(res);
		LOG(LOG_ERROR, _T("[core] PixelShader Creation error (%s)"), err.ErrorMessage());
	}
	return res == S_OK;
}

void CCoreDX11PixelShader::Release()
{
	//if (Reflection)
	//	Reflection->Release();
	//Reflection = NULL;
	if (PixelShaderHandle)
		PixelShaderHandle->Release();
	PixelShaderHandle = NULL;
}

TBOOL CCoreDX11PixelShader::Apply()
{
	if (!PixelShaderHandle) return false;
	DeviceContext->PSSetShader(PixelShaderHandle, NULL, 0);
	return true;
}

TBOOL CCoreDX11PixelShader::CompileAndCreate(CString *Err)
{
	if (!D3DCompileFunc && !InitShaderCompiler()) return false;

	Release();

	TBOOL Success = true;

	ID3D10Blob *PS = NULL;
	ID3D10Blob *Error = NULL;

	TS8 *code = new TS8[Code.Length() + 2];
	memset(code, 0, Code.Length() + 2);
	Code.WriteAsMultiByte(code, Code.Length() + 1);

	TS8 *entry = new TS8[EntryFunction.Length() + 2];
	memset(entry, 0, EntryFunction.Length() + 2);
	EntryFunction.WriteAsMultiByte(entry, EntryFunction.Length() + 1);

	TS8 *version = new TS8[ShaderVersion.Length() + 2];
	memset(version, 0, ShaderVersion.Length() + 2);
	ShaderVersion.WriteAsMultiByte(version, ShaderVersion.Length() + 1);

	TU32 tmp;
	_controlfp_s(&tmp, _RC_NEAR, _MCW_RC);

#ifdef CORE_API_D3DX
	if (D3DX11CompileFromMemory(code, strlen(code), NULL, NULL, NULL, entry, version, D3DCOMPILE_OPTIMIZATION_LEVEL0, 0, NULL, &PS, &Error, 0) != S_OK)
#else
	if (D3DCompileFunc(code, strlen(code), NULL, NULL, NULL, entry, version, 0, 0, &PS, &Error) != S_OK)
#endif
	{
		if (!Err)
			LOG(LOG_ERROR, _T("[core] PixelShader compilation error: %s"), CString((char*)Error->GetBufferPointer()).GetPointer());
		Success = false;
	}

	if (Err) *Err = Error ? CString((char*)Error->GetBufferPointer()) : _T("");

	if (Success)
	{
    //FILE *f = fopen( "pxshader.bin", "w+b" );
    //fwrite( PS->GetBufferPointer(), PS->GetBufferSize(), 1, f );
    //fclose( f );
    //DumpShader(code, PS->GetBufferPointer(), PS->GetBufferSize());
		Success = Create(PS->GetBufferPointer(), (TS32)PS->GetBufferSize());
		//if (Success)
		//{
		//	HRESULT r = D3DReflectFunc(PS->GetBufferPointer(), PS->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&Reflection);
		//	if (r != S_OK)
		//	{
		//		Success = false;
		//		_com_error err(r);
		//		LOG_ERR("[core] Failed to acquire PixelShader reflection (%s)", err.ErrorMessage());
		//	}
		//}
		PS->Release();
	}

#ifndef _WIN64
	_controlfp_s(&tmp, tmp, 0xffffffff);
#endif

	delete[] code;
	delete[] entry;
	delete[] version;

	return Success;
}

TBOOL CCoreDX11PixelShader::CreateFromBlob( void *CodeBlob, TS32 CodeBlobSize )
{
  Release();
  TBOOL Success = true;
  TU32 tmp;
  _controlfp_s( &tmp, _RC_NEAR, _MCW_RC );
  Success = Create( CodeBlob, CodeBlobSize );
#ifndef _WIN64
  _controlfp_s( &tmp, tmp, 0xffffffff );
#endif
  return Success;
}

//TS32 CCoreDX11PixelShader::GetConstantBufferIndex(TS8 *Name)
//{
//	if (!Reflection) return -1;
//
//	CString s = Name;
//
//	for (TS32 x = 0; x < 15; x++)
//	{
//		ID3D11ShaderReflectionConstantBuffer *cb = Reflection->GetConstantBufferByIndex(x);
//		if (cb)
//		{
//			D3D11_SHADER_BUFFER_DESC Desc;
//			if (cb->GetDesc(&Desc) == S_OK)
//				if (s == Desc.Name) return x;
//		}
//	}
//
//	return -1;
//}

//////////////////////////////////////////////////////////////////////////
// Geometry shader

CCoreDX11GeometryShader::CCoreDX11GeometryShader(CCoreDX11Device *dev) : CCoreGeometryShader(dev)
{
	Dev = dev->GetDevice();
	DeviceContext = dev->GetDeviceContext();
	GeometryShaderHandle = NULL;
	//Reflection = NULL;
}

CCoreDX11GeometryShader::~CCoreDX11GeometryShader()
{
	Release();
}

TBOOL CCoreDX11GeometryShader::Create(void *Binary, TS32 Length)
{
	if (!Binary || Length <= 0) return false;
	FetchBinary(Binary, Length);
	HRESULT res = Dev->CreateGeometryShader(Binary, Length, NULL, &GeometryShaderHandle);
	if (res != S_OK)
	{
		_com_error err(res);
		LOG(LOG_ERROR, _T("[core] GeometryShader Creation error (%s)"), err.ErrorMessage());
	}
	return res == S_OK;
}

void CCoreDX11GeometryShader::Release()
{
	//if (Reflection)
	//	Reflection->Release();
	//Reflection = NULL;
	if (GeometryShaderHandle)
		GeometryShaderHandle->Release();
	GeometryShaderHandle = NULL;
}

TBOOL CCoreDX11GeometryShader::Apply()
{
	if (!GeometryShaderHandle) return false;
	DeviceContext->GSSetShader(GeometryShaderHandle, NULL, 0);
	return true;
}

TBOOL CCoreDX11GeometryShader::CompileAndCreate(CString *Err)
{
	if (!D3DCompileFunc && !InitShaderCompiler()) return false;

	Release();

	TBOOL Success = true;

	ID3D10Blob *PS = NULL;
	ID3D10Blob *Error = NULL;

	TS8 *code = new TS8[Code.Length() + 2];
	memset(code, 0, Code.Length() + 2);
	Code.WriteAsMultiByte(code, Code.Length() + 1);

	TS8 *entry = new TS8[EntryFunction.Length() + 2];
	memset(entry, 0, EntryFunction.Length() + 2);
	EntryFunction.WriteAsMultiByte(entry, EntryFunction.Length() + 1);

	TS8 *version = new TS8[ShaderVersion.Length() + 2];
	memset(version, 0, ShaderVersion.Length() + 2);
	ShaderVersion.WriteAsMultiByte(version, ShaderVersion.Length() + 1);

	TU32 tmp;
	_controlfp_s(&tmp, _RC_NEAR, _MCW_RC);

#ifdef CORE_API_D3DX
	if (D3DX11CompileFromMemory(code, strlen(code), NULL, NULL, NULL, entry, version, D3DCOMPILE_OPTIMIZATION_LEVEL0, 0, NULL, &PS, &Error, 0) != S_OK)
#else
	if (D3DCompileFunc(code, strlen(code), NULL, NULL, NULL, entry, version, 0, 0, &PS, &Error) != S_OK)
#endif
	{
		if (!Err)
			LOG(LOG_ERROR, _T("[core] GeometryShader compilation error: %s"), CString((char*)Error->GetBufferPointer()).GetPointer());
		Success = false;
	}

	if (Err) *Err = Error ? CString((char*)Error->GetBufferPointer()) : _T("");

	if (Success)
	{
		//DumpShader(code, PS->GetBufferPointer(), PS->GetBufferSize());
		Success = Create(PS->GetBufferPointer(), (TS32)PS->GetBufferSize());
		//if (Success)
		//{
		//	HRESULT r = D3DReflectFunc(PS->GetBufferPointer(), PS->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&Reflection);
		//	if (r != S_OK)
		//	{
		//		Success = false;
		//		_com_error err(r);
		//		LOG_ERR("[core] Failed to acquire GeometryShader reflection (%s)", err.ErrorMessage());
		//	}
		//}
		PS->Release();
	}

#ifndef _WIN64
  _controlfp_s( &tmp, tmp, 0xffffffff );
#endif

	delete[] code;
	delete[] entry;
	delete[] version;

	return Success;
}

TBOOL CCoreDX11GeometryShader::CreateFromBlob( void *CodeBlob, TS32 CodeBlobSize )
{
  Release();
  TBOOL Success = true;
  TU32 tmp;
  _controlfp_s( &tmp, _RC_NEAR, _MCW_RC );
  Success = Create( CodeBlob, CodeBlobSize );
#ifndef _WIN64
  _controlfp_s( &tmp, tmp, 0xffffffff );
#endif
  return Success;
}

//TS32 CCoreDX11GeometryShader::GetConstantBufferIndex(TS8 *Name)
//{
//	if (!Reflection) return -1;
//
//	CString s = Name;
//
//	for (TS32 x = 0; x < 15; x++)
//	{
//		ID3D11ShaderReflectionConstantBuffer *cb = Reflection->GetConstantBufferByIndex(x);
//		if (cb)
//		{
//			D3D11_SHADER_BUFFER_DESC Desc;
//			if (cb->GetDesc(&Desc) == S_OK)
//				if (s == Desc.Name) return x;
//		}
//	}
//
//	return -1;
//}

//////////////////////////////////////////////////////////////////////////
// Domain shader

CCoreDX11DomainShader::CCoreDX11DomainShader(CCoreDX11Device *dev) : CCoreDomainShader(dev)
{
	Dev = dev->GetDevice();
	DeviceContext = dev->GetDeviceContext();
	DomainShaderHandle = NULL;
	//Reflection = NULL;
}

CCoreDX11DomainShader::~CCoreDX11DomainShader()
{
	Release();
}

TBOOL CCoreDX11DomainShader::Create(void *Binary, TS32 Length)
{
	if (!Binary || Length <= 0) return false;
	FetchBinary(Binary, Length);
	HRESULT res = Dev->CreateDomainShader(Binary, Length, NULL, &DomainShaderHandle);
	if (res != S_OK)
	{
		_com_error err(res);
		LOG(LOG_ERROR, _T("[core] DomainShader Creation error (%s)"), err.ErrorMessage());
	}
	return res == S_OK;
}

void CCoreDX11DomainShader::Release()
{
	//if (Reflection)
	//	Reflection->Release();
	//Reflection = NULL;
	if (DomainShaderHandle)
		DomainShaderHandle->Release();
	DomainShaderHandle = NULL;
}

TBOOL CCoreDX11DomainShader::Apply()
{
	if (!DomainShaderHandle) return false;
	DeviceContext->DSSetShader(DomainShaderHandle, NULL, 0);
	return true;
}

TBOOL CCoreDX11DomainShader::CompileAndCreate(CString *Err)
{
	if (!D3DCompileFunc && !InitShaderCompiler()) return false;

	Release();

	TBOOL Success = true;

	ID3D10Blob *PS = NULL;
	ID3D10Blob *Error = NULL;

	TS8 *code = new TS8[Code.Length() + 2];
	memset(code, 0, Code.Length() + 2);
	Code.WriteAsMultiByte(code, Code.Length() + 1);

	TS8 *entry = new TS8[EntryFunction.Length() + 2];
	memset(entry, 0, EntryFunction.Length() + 2);
	EntryFunction.WriteAsMultiByte(entry, EntryFunction.Length() + 1);

	TS8 *version = new TS8[ShaderVersion.Length() + 2];
	memset(version, 0, ShaderVersion.Length() + 2);
	ShaderVersion.WriteAsMultiByte(version, ShaderVersion.Length() + 1);

	TU32 tmp;
	_controlfp_s(&tmp, _RC_NEAR, _MCW_RC);

#ifdef CORE_API_D3DX
	if (D3DX11CompileFromMemory(code, strlen(code), NULL, NULL, NULL, entry, version, D3DCOMPILE_OPTIMIZATION_LEVEL0, 0, NULL, &PS, &Error, 0) != S_OK)
#else
	if (D3DCompileFunc(code, strlen(code), NULL, NULL, NULL, entry, version, 0, 0, &PS, &Error) != S_OK)
#endif
	{
		if (!Err)
			LOG(LOG_ERROR, _T("[core] DomainShader compilation error: %s"), CString((char*)Error->GetBufferPointer()).GetPointer());
		Success = false;
	}

	if (Err) *Err = Error ? CString((char*)Error->GetBufferPointer()) : _T("");

	if (Success)
	{
		//DumpShader(code, PS->GetBufferPointer(), PS->GetBufferSize());
		Success = Create(PS->GetBufferPointer(), (TS32)PS->GetBufferSize());
		//if (Success)
		//{
		//	HRESULT r = D3DReflectFunc(PS->GetBufferPointer(), PS->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&Reflection);
		//	if (r != S_OK)
		//	{
		//		Success = false;
		//		_com_error err(r);
		//		LOG_ERR("[core] Failed to acquire DomainShader reflection (%s)", err.ErrorMessage());
		//	}
		//}
		PS->Release();
	}

#ifndef _WIN64
  _controlfp_s( &tmp, tmp, 0xffffffff );
#endif

	delete[] code;
	delete[] entry;
	delete[] version;

	return Success;
}

TBOOL CCoreDX11DomainShader::CreateFromBlob( void *CodeBlob, TS32 CodeBlobSize )
{
  Release();
  TBOOL Success = true;
  TU32 tmp;
  _controlfp_s( &tmp, _RC_NEAR, _MCW_RC );
  Success = Create( CodeBlob, CodeBlobSize );
#ifndef _WIN64
  _controlfp_s( &tmp, tmp, 0xffffffff );
#endif
  return Success;
}

//TS32 CCoreDX11DomainShader::GetConstantBufferIndex(TS8 *Name)
//{
//	if (!Reflection) return -1;
//
//	CString s = Name;
//
//	for (TS32 x = 0; x < 15; x++)
//	{
//		ID3D11ShaderReflectionConstantBuffer *cb = Reflection->GetConstantBufferByIndex(x);
//		if (cb)
//		{
//			D3D11_SHADER_BUFFER_DESC Desc;
//			if (cb->GetDesc(&Desc) == S_OK)
//				if (s == Desc.Name) return x;
//		}
//	}
//
//	return -1;
//}

//////////////////////////////////////////////////////////////////////////
// Hull shader

CCoreDX11HullShader::CCoreDX11HullShader(CCoreDX11Device *dev) : CCoreHullShader(dev)
{
	Dev = dev->GetDevice();
	DeviceContext = dev->GetDeviceContext();
	HullShaderHandle = NULL;
	//Reflection = NULL;
}

CCoreDX11HullShader::~CCoreDX11HullShader()
{
	Release();
}

TBOOL CCoreDX11HullShader::Create(void *Binary, TS32 Length)
{
	if (!Binary || Length <= 0) return false;
	FetchBinary(Binary, Length);
	HRESULT res = Dev->CreateHullShader(Binary, Length, NULL, &HullShaderHandle);
	if (res != S_OK)
	{
		_com_error err(res);
		LOG(LOG_ERROR, _T("[core] HullShader Creation error (%s)"), err.ErrorMessage());
	}
	return res == S_OK;
}

void CCoreDX11HullShader::Release()
{
	//if (Reflection)
	//	Reflection->Release();
	//Reflection = NULL;
	if (HullShaderHandle)
		HullShaderHandle->Release();
	HullShaderHandle = NULL;
}

TBOOL CCoreDX11HullShader::Apply()
{
	if (!HullShaderHandle) return false;
	DeviceContext->HSSetShader(HullShaderHandle, NULL, 0);
	return true;
}

TBOOL CCoreDX11HullShader::CompileAndCreate(CString *Err)
{
	if (!D3DCompileFunc && !InitShaderCompiler()) return false;

	Release();

	TBOOL Success = true;

	ID3D10Blob *PS = NULL;
	ID3D10Blob *Error = NULL;

	TS8 *code = new TS8[Code.Length() + 2];
	memset(code, 0, Code.Length() + 2);
	Code.WriteAsMultiByte(code, Code.Length() + 1);

	TS8 *entry = new TS8[EntryFunction.Length() + 2];
	memset(entry, 0, EntryFunction.Length() + 2);
	EntryFunction.WriteAsMultiByte(entry, EntryFunction.Length() + 1);

	TS8 *version = new TS8[ShaderVersion.Length() + 2];
	memset(version, 0, ShaderVersion.Length() + 2);
	ShaderVersion.WriteAsMultiByte(version, ShaderVersion.Length() + 1);

	TU32 tmp;
	_controlfp_s(&tmp, _RC_NEAR, _MCW_RC);

#ifdef CORE_API_D3DX
	if (D3DX11CompileFromMemory(code, strlen(code), NULL, NULL, NULL, entry, version, D3DCOMPILE_OPTIMIZATION_LEVEL0, 0, NULL, &PS, &Error, 0) != S_OK)
#else
	if (D3DCompileFunc(code, strlen(code), NULL, NULL, NULL, entry, version, 0, 0, &PS, &Error) != S_OK)
#endif
	{
		if (!Err)
			LOG(LOG_ERROR, _T("[core] HullShader compilation error: %s"), CString((char*)Error->GetBufferPointer()).GetPointer());
		Success = false;
	}

	if (Err) *Err = Error ? CString((char*)Error->GetBufferPointer()) : _T("");

	if (Success)
	{
		//DumpShader(code, PS->GetBufferPointer(), PS->GetBufferSize());
		Success = Create(PS->GetBufferPointer(), (TS32)PS->GetBufferSize());
		//if (Success)
		//{
		//	HRESULT r = D3DReflectFunc(PS->GetBufferPointer(), PS->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&Reflection);
		//	if (r != S_OK)
		//	{
		//		Success = false;
		//		_com_error err(r);
		//		LOG_ERR("[core] Failed to acquire HullShader reflection (%s)", err.ErrorMessage());
		//	}
		//}
		PS->Release();
	}

#ifndef _WIN64
  _controlfp_s( &tmp, tmp, 0xffffffff );
#endif

	delete[] code;
	delete[] entry;
	delete[] version;

	return Success;
}

TBOOL CCoreDX11HullShader::CreateFromBlob( void *CodeBlob, TS32 CodeBlobSize )
{
  Release();
  TBOOL Success = true;
  TU32 tmp;
  _controlfp_s( &tmp, _RC_NEAR, _MCW_RC );
  Success = Create( CodeBlob, CodeBlobSize );
#ifndef _WIN64
  _controlfp_s( &tmp, tmp, 0xffffffff );
#endif
  return Success;
}

//TS32 CCoreDX11HullShader::GetConstantBufferIndex(TS8 *Name)
//{
//	if (!Reflection) return -1;
//
//	CString s = Name;
//
//	for (TS32 x = 0; x < 15; x++)
//	{
//		ID3D11ShaderReflectionConstantBuffer *cb = Reflection->GetConstantBufferByIndex(x);
//		if (cb)
//		{
//			D3D11_SHADER_BUFFER_DESC Desc;
//			if (cb->GetDesc(&Desc) == S_OK)
//				if (s == Desc.Name) return x;
//		}
//	}
//
//	return -1;
//}


//////////////////////////////////////////////////////////////////////////
// Compute shader

CCoreDX11ComputeShader::CCoreDX11ComputeShader(CCoreDX11Device *dev) : CCoreComputeShader(dev)
{
	Dev = dev->GetDevice();
	DeviceContext = dev->GetDeviceContext();
	ComputeShaderHandle = NULL;
	//Reflection = NULL;
}

CCoreDX11ComputeShader::~CCoreDX11ComputeShader()
{
	Release();
}

TBOOL CCoreDX11ComputeShader::Create(void *Binary, TS32 Length)
{
	if (!Binary || Length <= 0) return false;
	FetchBinary(Binary, Length);
	HRESULT res = Dev->CreateComputeShader(Binary, Length, NULL, &ComputeShaderHandle);
	if (res != S_OK)
	{
		_com_error err(res);
		LOG(LOG_ERROR, _T("[core] ComputeShader Creation error (%s)"), err.ErrorMessage());
	}
	return res == S_OK;
}

void CCoreDX11ComputeShader::Release()
{
	//if (Reflection)
	//	Reflection->Release();
	//Reflection = NULL;
	if (ComputeShaderHandle)
		ComputeShaderHandle->Release();
	ComputeShaderHandle = NULL;
}

TBOOL CCoreDX11ComputeShader::Apply()
{
	if (!ComputeShaderHandle) return false;
	DeviceContext->CSSetShader(ComputeShaderHandle, NULL, 0);
	return true;
}

TBOOL CCoreDX11ComputeShader::CompileAndCreate(CString *Err)
{
	if (!D3DCompileFunc && !InitShaderCompiler()) return false;

	Release();

	TBOOL Success = true;

	ID3D10Blob *PS = NULL;
	ID3D10Blob *Error = NULL;

	TS8 *code = new TS8[Code.Length() + 2];
	memset(code, 0, Code.Length() + 2);
	Code.WriteAsMultiByte(code, Code.Length() + 1);

	TS8 *entry = new TS8[EntryFunction.Length() + 2];
	memset(entry, 0, EntryFunction.Length() + 2);
	EntryFunction.WriteAsMultiByte(entry, EntryFunction.Length() + 1);

	TS8 *version = new TS8[ShaderVersion.Length() + 2];
	memset(version, 0, ShaderVersion.Length() + 2);
	ShaderVersion.WriteAsMultiByte(version, ShaderVersion.Length() + 1);

	TU32 tmp;
	_controlfp_s(&tmp, _RC_NEAR, _MCW_RC);

#ifdef CORE_API_D3DX
	if (D3DX11CompileFromMemory(code, strlen(code), NULL, NULL, NULL, entry, version, D3DCOMPILE_OPTIMIZATION_LEVEL0, 0, NULL, &PS, &Error, 0) != S_OK)
#else
	if (D3DCompileFunc(code, strlen(code), NULL, NULL, NULL, entry, version, 0, 0, &PS, &Error) != S_OK)
#endif
	{
		if (!Err)
			LOG(LOG_ERROR, _T("[core] ComputeShader compilation error: %s"), CString((char*)Error->GetBufferPointer()).GetPointer());
		Success = false;
	}

	if (Err) *Err = Error ? CString((char*)Error->GetBufferPointer()) : _T("");

	if (Success)
	{
		//DumpShader(code, PS->GetBufferPointer(), PS->GetBufferSize());
		Success = Create(PS->GetBufferPointer(), (TS32)PS->GetBufferSize());
		//if (Success)
		//{
		//	HRESULT r = D3DReflectFunc(PS->GetBufferPointer(), PS->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&Reflection);
		//	if (r != S_OK)
		//	{
		//		Success = false;
		//		_com_error err(r);
		//		LOG_ERR("[core] Failed to acquire ComputeShader reflection (%s)", err.ErrorMessage());
		//	}
		//}
		PS->Release();
	}

#ifndef _WIN64
  _controlfp_s( &tmp, tmp, 0xffffffff );
#endif

	delete[] code;
	delete[] entry;
	delete[] version;

	return Success;
}

TBOOL CCoreDX11ComputeShader::CreateFromBlob( void *CodeBlob, TS32 CodeBlobSize )
{
  Release();
  TBOOL Success = true;
  TU32 tmp;
  _controlfp_s( &tmp, _RC_NEAR, _MCW_RC );
  Success = Create( CodeBlob, CodeBlobSize );
#ifndef _WIN64
  _controlfp_s( &tmp, tmp, 0xffffffff );
#endif
  return Success;
}

//TS32 CCoreDX11ComputeShader::GetConstantBufferIndex(TS8 *Name)
//{
//	if (!Reflection) return -1;
//
//	CString s = Name;
//
//	for (TS32 x = 0; x < 15; x++)
//	{
//		ID3D11ShaderReflectionConstantBuffer *cb = Reflection->GetConstantBufferByIndex(x);
//		if (cb)
//		{
//			D3D11_SHADER_BUFFER_DESC Desc;
//			if (cb->GetDesc(&Desc) == S_OK)
//				if (s == Desc.Name) return x;
//		}
//	}
//
//	return -1;
//}


#else
NoEmptyFile();
#endif