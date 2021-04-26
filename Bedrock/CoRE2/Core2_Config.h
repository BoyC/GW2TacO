#pragma once

//////////////////////////////////////////////////////////////////////////
//configuration file for CoRE2
//defines high level configuration options like the used graphics api

//////////////////////////////////////////////////////////////////////////
//config macros

//this enables debugging features
#ifdef _DEBUG
#define ENABLE_CORE_DEBUG_MODE
#endif

//pix api should only be enabled when vs2012 is installed as well
//#define ENABLE_PIX_API

//DX9 build is not up to date and will not compile.
//#define CORE_API_DX9
#define CORE_API_DX11

//#define CORE_API_D3DX

#define CORE_VERBOSE_LOG

//////////////////////////////////////////////////////////////////////////
//logic
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <Windows.h>

#ifdef CORE_API_DX9

#include <d3d9.h>
#pragma comment(lib,"d3d9.lib")

#ifdef CORE_API_D3DX
#include <d3dx9.h>
#pragma comment(lib,"d3dx9.lib")
#endif

#endif

#ifdef CORE_API_DX11

#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")

//#include <D3Dcompiler.h>
//#pragma comment(lib,"dxguid.lib")
//#pragma comment(lib,"D3DCompiler.lib")

#ifdef CORE_API_D3DX
#include <D3DX11.h>

#pragma comment(lib,"d3dx11.lib")
#endif

#endif

