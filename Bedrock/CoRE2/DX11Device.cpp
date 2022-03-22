#include "BasePCH.h"
#include "DX11Device.h"
#include "DX11Texture.h"
#include "DX11VertexBuffer.h"
#include "DX11IndexBuffer.h"
#include "DX11VertexFormat.h"
#include "DX11Shader.h"
#include "DX11ConstantBuffer.h"
#include "DX11RenderState.h"
#include <DComp.h>
#pragma comment(lib,"DXGI.lib")
//#pragma comment(lib,"dcomp.lib")

typedef HRESULT( __stdcall* DCompositionCreateDeviceCallback )(
  _In_opt_ IDXGIDevice* dxgiDevice,
  _In_ REFIID iid,
  _Outptr_ void** dcompositionDevice
  );
DCompositionCreateDeviceCallback DCompositionCreateDeviceFunc = nullptr;

#ifdef CORE_API_DX11

CCoreDX11Device::CCoreDX11Device()
{
  Device = NULL;
  DeviceContext = NULL;
  SwapChain = NULL;
  BackBufferView = NULL;
  DepthBufferView = NULL;
  DepthBuffer = NULL;
  CurrentBlendState = NULL;
  CurrentDepthStencilState = NULL;
  CurrentRasterizerState = NULL;
}

CCoreDX11Device::~CCoreDX11Device()
{
  if ( swapChainRetraceObject )
    CloseHandle(swapChainRetraceObject);

  if ( OcclusionQuery ) OcclusionQuery->Release();

  if ( BackBufferView ) BackBufferView->Release();
  if ( DepthBufferView ) DepthBufferView->Release();
  if ( DepthBuffer ) DepthBuffer->Release();
  if ( SwapChain )
  {
    SwapChain->SetFullscreenState( false, NULL );
    SwapChain->Release();
  }

  if ( DeviceContext )
  {
    DeviceContext->ClearState();
    DeviceContext->Flush();
    DeviceContext->Release();
  }

  if ( Device )
  {
    ID3D11Debug* dbg = NULL;
    Device->QueryInterface( __uuidof( ID3D11Debug ), reinterpret_cast<void**>( &dbg ) );
    if ( dbg )
    {
      LOG_NFO( "[core] Dumping Live objects before freeing device:" );
      dbg->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );
      dbg->Release();
    }
  }
  if ( Device ) Device->Release();

}

void CCoreDX11Device::ResetPrivateResources()
{
  //if (BackBufferSurface) BackBufferSurface->Release();
  //BackBufferSurface=NULL;

  //Device->Reset(&D3DPP);

  //Device->GetRenderTarget(0,&BackBufferSurface);
}

#define BACKBUFFERFORMAT D3DFMT_A8R8G8B8

TBOOL CCoreDX11Device::CreateBackBuffer( TS32 XRes, TS32 YRes )
{
  if ( BackBufferView ) BackBufferView->Release();

  FORCEDDEBUGLOG( "creating backbuffer" );

  HRESULT res = S_OK;
  ID3D11Texture2D* bb;

  res = SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (LPVOID*)&bb );
  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_ERROR, _T( "[core] DirectX11 Swapchain buffer acquisition failed (%s)" ), err.ErrorMessage() );
    return false;
  }

  FORCEDDEBUGLOG( "getbuffer called" );

  res = Device->CreateRenderTargetView( bb, NULL, &BackBufferView );
  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_ERROR, _T( "[core] DirectX11 Rendertarget View creation failed (%s)" ), err.ErrorMessage() );
    return false;
  }

  FORCEDDEBUGLOG( "createrendertargetview called" );

  res = bb->Release();
  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_ERROR, _T( "[core] DirectX11 Swapchain buffer texture release failed (%s)" ), err.ErrorMessage() );
    //return false;
  }

  FORCEDDEBUGLOG( "previous buffer released" );

  return true;
}

TBOOL CCoreDX11Device::CreateDepthBuffer( TS32 XRes, TS32 YRes )
{
  if ( DepthBufferView ) DepthBufferView->Release();
  if ( DepthBuffer ) DepthBuffer->Release();

  FORCEDDEBUGLOG( "creating depthbuffer" );

  HRESULT res = S_OK;

  D3D11_TEXTURE2D_DESC depthBufferDesc;
  D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

  memset( &depthBufferDesc, 0, sizeof( depthBufferDesc ) );

  depthBufferDesc.Width = XRes;
  depthBufferDesc.Height = YRes;
  depthBufferDesc.MipLevels = 1;
  depthBufferDesc.ArraySize = 1;
  depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthBufferDesc.SampleDesc.Count = 1;
  depthBufferDesc.SampleDesc.Quality = 0;
  depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  depthBufferDesc.CPUAccessFlags = 0;
  depthBufferDesc.MiscFlags = 0;

  res = Device->CreateTexture2D( &depthBufferDesc, NULL, &DepthBuffer );
  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_ERROR, _T( "[core] DirectX11 Depth Texture creation failed (%s)" ), err.ErrorMessage() );
    return false;
  }

  FORCEDDEBUGLOG( "createtexture called" );

  ZeroMemory( &depthStencilViewDesc, sizeof( depthStencilViewDesc ) );

  depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  depthStencilViewDesc.Texture2D.MipSlice = 0;

  res = Device->CreateDepthStencilView( DepthBuffer, &depthStencilViewDesc, &DepthBufferView );
  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_ERROR, _T( "[core] DirectX11 DepthStencil View creation failed (%s)" ), err.ErrorMessage() );
    return false;
  }

  FORCEDDEBUGLOG( "createdepthstencilview called" );

  return true;
}

TBOOL CCoreDX11Device::CreateClassicSwapChain( const TU32 hWnd, const TBOOL FullScreen, const TS32 XRes, const TS32 YRes, const TS32 AALevel, const TS32 RefreshRate )
{
  LOG_NFO( "[core] Creating classic swap chain" );

  FORCEDDEBUGLOG( "Initapi" );
  HRESULT res = S_OK;

  DXGI_SWAP_CHAIN_DESC scd;
  memset( &scd, 0, sizeof( DXGI_SWAP_CHAIN_DESC ) );

  scd.BufferCount = 2;
  scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  scd.OutputWindow = (HWND)hWnd;
  scd.SampleDesc.Count = 1;
  scd.Windowed = !FullScreen;
/*
  scd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
  scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
*/
  //scd.BufferDesc.RefreshRate.Numerator=RefreshRate;
  //scd.BufferDesc.RefreshRate.Denominator=1;


#ifdef ENABLE_CORE_DEBUG_MODE
  res = D3D11CreateDeviceAndSwapChain( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, NULL, NULL, D3D11_SDK_VERSION, &scd, (IDXGISwapChain**)&SwapChain, &Device, NULL, &DeviceContext );
  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_WARNING, _T( "[core] DirectX11 debug mode device creation failed. (%s) Trying without debug mode..." ), err.ErrorMessage() );
#endif
    FORCEDDEBUGLOG( "About to create d3d device" );
    res = D3D11CreateDeviceAndSwapChain( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, NULL, D3D11_SDK_VERSION, &scd, (IDXGISwapChain**)&SwapChain, &Device, NULL, &DeviceContext );
    if ( res != S_OK )
    {
      _com_error error( res );
      LOG( LOG_ERROR, _T( "[core] DirectX11 Device creation failed (%s)" ), error.ErrorMessage() );
      return false;
    }
    FORCEDDEBUGLOG( "D3D device created" );
#ifdef ENABLE_CORE_DEBUG_MODE
  }
#endif

  if ( !CreateBackBuffer( XRes, YRes ) ) return false;
  FORCEDDEBUGLOG( "Backbuffer created" );
  if ( !CreateDepthBuffer( XRes, YRes ) ) return false;
  FORCEDDEBUGLOG( "Depthbuffer created" );
  DeviceContext->OMSetRenderTargets( 1, &BackBufferView, DepthBufferView );

  FORCEDDEBUGLOG( "rendertargets set" );

  SetViewport( CRect( 0, 0, XRes, YRes ) );
  FORCEDDEBUGLOG( "viewports set" );

  if ( CreateDefaultRenderStates() )
    LOG( LOG_INFO, _T( "[core] DirectX11 Device initialization successful." ) );

  FORCEDDEBUGLOG( "default renderstates created" );

  D3D11_QUERY_DESC queryDesc;
  memset( &queryDesc, 0, sizeof( queryDesc ) );
  queryDesc.Query = D3D11_QUERY_OCCLUSION;
  queryDesc.MiscFlags = 0;

  Device->CreateQuery( &queryDesc, &OcclusionQuery );

  return true;
}

TBOOL CCoreDX11Device::CreateDirectCompositionSwapchain( const TU32 hWnd, const TBOOL FullScreen, const TS32 XRes, const TS32 YRes, const TS32 AALevel, const TS32 RefreshRate )
{
  LOG_NFO( "[core] Creating DirectComposition swap chain" );

  FORCEDDEBUGLOG( "Initapi" );
  HRESULT res = S_OK;


  IDXGIFactory2* dxgiFactory;
#ifdef _DEBUG
  res = CreateDXGIFactory2( DXGI_CREATE_FACTORY_DEBUG, __uuidof( IDXGIFactory2 ), (void**)&dxgiFactory );
#else
  res = CreateDXGIFactory1( __uuidof( IDXGIFactory2 ), (void**)&dxgiFactory );
#endif
  if ( res != S_OK )
  {
    _com_error error( res );
    LOG( LOG_ERROR, _T( "[core] DXGI factory creation failed (%s)" ), error.ErrorMessage() );
    return false;
  }

  /*
    DXGI_SWAP_CHAIN_DESC1 scd;
    memset(&scd, 0, sizeof(DXGI_SWAP_CHAIN_DESC1));

    scd.BufferCount = 1;
    scd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    //scd.OutputWindow = (HWND)hWnd;
    scd.SampleDesc.Count = 1;
    //scd.Windowed = !FullScreen;

    scd.BufferCount = 2;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scd.AlphaMode = DXGI_ALPHA_MODE_STRAIGHT;
    //scd.BufferDesc.RefreshRate.Numerator=RefreshRate;
    //scd.BufferDesc.RefreshRate.Denominator=1;
  */

#ifdef ENABLE_CORE_DEBUG_MODE
  res = D3D11CreateDevice( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, NULL, NULL, D3D11_SDK_VERSION, &Device, NULL, &DeviceContext );
  //res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, NULL, NULL, D3D11_SDK_VERSION, &scd, &SwapChain, &Device, NULL, &DeviceContext);
  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_WARNING, _T( "[core] DirectX11 debug mode device creation failed. (%s) Trying without debug mode..." ), err.ErrorMessage() );
#endif
    FORCEDDEBUGLOG( "About to create d3d device" );
    res = D3D11CreateDevice( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, NULL, D3D11_SDK_VERSION, &Device, NULL, &DeviceContext );
    //res = D3D11CreateDeviceAndSwapChain( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, NULL, D3D11_SDK_VERSION, &scd, &SwapChain, &Device, NULL, &DeviceContext );
    if ( res != S_OK )
    {
      _com_error error( res );
      LOG( LOG_ERROR, _T( "[core] DirectX11 Device creation failed (%s)" ), error.ErrorMessage() );
      return false;
    }
    FORCEDDEBUGLOG( "D3D device created" );
#ifdef ENABLE_CORE_DEBUG_MODE
  }
#endif

  unsigned int backBufferCount = 2;
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc{ (UINT)XRes, (UINT)YRes, DXGI_FORMAT_R8G8B8A8_UNORM, false, {1, 0}, DXGI_USAGE_RENDER_TARGET_OUTPUT, backBufferCount, DXGI_SCALING_STRETCH, DXGI_SWAP_EFFECT_FLIP_DISCARD, DXGI_ALPHA_MODE_PREMULTIPLIED, 0 };
  swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

  res = dxgiFactory->CreateSwapChainForComposition( Device, &swapChainDesc, nullptr, &SwapChain );
  //res = dxgiFactory->CreateSwapChainForHwnd( Device, (HWND)hWnd, &swapChainDesc, nullptr, nullptr, &SwapChain );

  //res = dxgiFactory->CreateSwapChain( Device, (DXGI_SWAP_CHAIN_DESC*)&swapChainDesc, (IDXGISwapChain**)&SwapChain );
  if ( res != S_OK )
  {
    _com_error error( res );
    LOG( LOG_ERROR, _T( "[core] DirectX11 SwapChain creation failed (%s)" ), error.ErrorMessage() );
    return false;
  }

  IDCompositionDevice* dcompDevice = nullptr;

  res = DCompositionCreateDeviceFunc( (IDXGIDevice*)Device, __uuidof( IDCompositionDevice ), (void**)&dcompDevice );

  if ( res != S_OK )
  {
    _com_error error( res );
    LOG( LOG_ERROR, _T( "[core] DirectComposition device creation failed (%s)" ), error.ErrorMessage() );
    return false;
  }

  IDCompositionTarget* dcompTarget = nullptr;
  res = dcompDevice->CreateTargetForHwnd( HWND( hWnd ), true, &dcompTarget );

  if ( res != S_OK )
  {
    _com_error error( res );
    LOG( LOG_ERROR, _T( "[core] DirectComposition target creation failed (%s)" ), error.ErrorMessage() );
    return false;
  }

  IDCompositionVisual* dcompVisual = nullptr;
  res = dcompDevice->CreateVisual( &dcompVisual );

  if ( res != S_OK )
  {
    _com_error error( res );
    LOG( LOG_ERROR, _T( "[core] DirectComposition visual creation failed (%s)" ), error.ErrorMessage() );
    return false;
  }

  res = dcompVisual->SetContent( SwapChain );

  if ( res != S_OK )
  {
    _com_error error( res );
    LOG( LOG_ERROR, _T( "[core] DirectComposition visual swapchain content setting failed (%s)" ), error.ErrorMessage() );
    return false;
  }

  res = dcompTarget->SetRoot( dcompVisual );

  if ( res != S_OK )
  {
    _com_error error( res );
    LOG( LOG_ERROR, _T( "[core] DirectComposition setting target root visual failed (%s)" ), error.ErrorMessage() );
    return false;
  }


  res = dcompDevice->Commit();

  if ( res != S_OK )
  {
    _com_error error( res );
    LOG( LOG_ERROR, _T( "[core] DirectComposition commit failed (%s)" ), error.ErrorMessage() );
    return false;
  }


  //res = dxgiFactory->MakeWindowAssociation( (HWND)hWnd, DXGI_MWA_NO_ALT_ENTER );
  //if ( res != S_OK )
  //{
  //  _com_error error( res );
  //  LOG( LOG_ERROR, _T( "[core] DirectX11 failed to unset alt-enter association (%s)" ), error.ErrorMessage() );
  //  return false;
  //}



  dxgiFactory->Release();

  if ( !CreateBackBuffer( XRes, YRes ) ) return false;
  FORCEDDEBUGLOG( "Backbuffer created" );
  if ( !CreateDepthBuffer( XRes, YRes ) ) return false;
  FORCEDDEBUGLOG( "Depthbuffer created" );
  DeviceContext->OMSetRenderTargets( 1, &BackBufferView, DepthBufferView );

  FORCEDDEBUGLOG( "rendertargets set" );

  SetViewport( CRect( 0, 0, XRes, YRes ) );
  FORCEDDEBUGLOG( "viewports set" );

  if ( CreateDefaultRenderStates() )
    LOG( LOG_INFO, _T( "[core] DirectX11 Device initialization successful." ) );

  FORCEDDEBUGLOG( "default renderstates created" );

  D3D11_QUERY_DESC queryDesc;
  memset( &queryDesc, 0, sizeof( queryDesc ) );
  queryDesc.Query = D3D11_QUERY_OCCLUSION;
  queryDesc.MiscFlags = 0;

  Device->CreateQuery( &queryDesc, &OcclusionQuery );

  IDXGISwapChain2* swapChain2;
  if ( SUCCEEDED( SwapChain->QueryInterface( __uuidof( IDXGISwapChain2 ), (void**)&swapChain2 ) ) )
  {
    swapChainRetraceObject = swapChain2->GetFrameLatencyWaitableObject();
    swapChain2->Release();
  }

  return true;
}

TBOOL CCoreDX11Device::InitAPI( const TU32 hWnd, const TBOOL FullScreen, const TS32 XRes, const TS32 YRes, const TS32 AALevel/* =0 */, const TS32 RefreshRate/* =60 */ )
{
  auto dcomp = LoadLibrary( "dcomp.dll" );

  if ( dcomp )
  {
    DCompositionCreateDeviceFunc = DCompositionCreateDeviceCallback( GetProcAddress( dcomp, "DCompositionCreateDevice" ) );
  }

  if ( !dcomp || !DCompositionCreateDeviceFunc )
    return CreateClassicSwapChain( hWnd, FullScreen, XRes, YRes, AALevel, RefreshRate );
  else
    return CreateDirectCompositionSwapchain( hWnd, FullScreen, XRes, YRes, AALevel, RefreshRate );

  if ( dcomp )
    FreeLibrary( dcomp );
}

TBOOL CCoreDX11Device::Initialize( CCoreWindowHandler* window, const TS32 AALevel )
{
  FORCEDDEBUGLOG( "Initializing DX11 device" );
  Window = window;

  if ( !InitAPI( Window->GetHandle(), Window->GetInitParameters().FullScreen, Window->GetXRes(), Window->GetYRes(), AALevel, 60 ) ) return false;

  FORCEDDEBUGLOG( "InitAPI ran" );

  ShowWindow( (HWND)Window->GetHandle(), Window->GetInitParameters().Maximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL );
  FORCEDDEBUGLOG( "Showwindow ran" );
  SetForegroundWindow( (HWND)Window->GetHandle() );
  FORCEDDEBUGLOG( "Setforegroundwindow ran" );
  SetFocus( (HWND)Window->GetHandle() );
  FORCEDDEBUGLOG( "Setfocus ran" );
  return true;
}

TBOOL CCoreDX11Device::IsWindowed()
{
  BOOL fs = false;
  IDXGIOutput* i = NULL;

  if ( SwapChain->GetFullscreenState( &fs, &i ) != S_OK )
  {
    LOG( LOG_ERROR, _T( "[core] Failed to get fullscreen state" ) );
    return false;
  }

  if ( i ) i->Release();
  return fs;
}

void CCoreDX11Device::Resize( const TS32 xr, const TS32 yr )
{
  //LOG(LOG_INFO,_T("[core] Resizing backbuffer to %dx%d"),xr,yr);

  if ( xr <= 0 || yr <= 0 )
  {
    LOG( LOG_WARNING, _T( "[core] Trying to resize swapchain to invalid resolution: %d %d"), xr, yr );
    return;
  }


  DXGI_SWAP_CHAIN_DESC desc;
  HRESULT res = SwapChain->GetDesc( &desc );
  if ( res != S_OK )
  {
    LOG( LOG_ERROR, _T( "[core] Failed to get swapchain description" ) );
    return;
  }

  if ( desc.BufferDesc.Width == xr && desc.BufferDesc.Height == yr ) return;

  if ( BackBufferView ) BackBufferView->Release();
  if ( DepthBufferView ) DepthBufferView->Release();
  if ( DepthBuffer ) DepthBuffer->Release();
  BackBufferView = NULL;
  DepthBufferView = NULL;
  DepthBuffer = NULL;

  res = SwapChain->ResizeBuffers( desc.BufferCount, xr, yr, DXGI_FORMAT_UNKNOWN, desc.Flags );
  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_ERROR, _T( "[core] Failed to resize swapchain to %d %d (bufferCount: %d, flags: %d) (%s)" ), xr, yr, desc.BufferCount, desc.Flags, err.ErrorMessage() );
    return;
  }

  if ( !CreateBackBuffer( xr, yr ) ) return;
  if ( !CreateDepthBuffer( xr, yr ) ) return;

  DeviceContext->OMSetRenderTargets( 1, &BackBufferView, DepthBufferView );
  SetViewport( CRect( 0, 0, xr, yr ) );

}

void CCoreDX11Device::SetFullScreenMode( const TBOOL FullScreen, const TS32 xr, const TS32 yr )
{
  //if (xr && yr)
  //{
  //	DXGI_MODE_DESC m;
  //	SwapChain->GetDesc()
  //	SwapChain->ResizeTarget();
  //}

  LOG( LOG_INFO, _T( "[core] Switching fullscreen mode to %d" ), FullScreen );

  HRESULT res = SwapChain->SetFullscreenState( FullScreen, NULL );
  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_ERROR, _T( "[core] Failed to set FullScreen mode to %d. (%s)" ), FullScreen, err.ErrorMessage() );
    return;
  }
}

TBOOL CCoreDX11Device::DeviceOk()
{
  return true;
}

//////////////////////////////////////////////////////////////////////////
// texture functions

CCoreTexture2D* CCoreDX11Device::CreateTexture2D( const TS32 XRes, const TS32 YRes, const TU8* Data, const TS8 BytesPerPixel, const COREFORMAT Format/* =COREFMT_A8R8G8B8 */, const TBOOL RenderTarget/* =false */ )
{
  CCoreTexture2D* Result = new CCoreDX11Texture2D( this );
  if ( !Result->Create( XRes, YRes, Data, BytesPerPixel, Format, RenderTarget ) )
    SAFEDELETE( Result );
  return Result;
}

CCoreTexture2D* CCoreDX11Device::CreateTexture2D( const TU8* Data, const TS32 Size )
{
  CCoreTexture2D* Result = new CCoreDX11Texture2D( this );
  if ( !Result->Create( Data, Size ) )
    SAFEDELETE( Result );
  return Result;
}

//////////////////////////////////////////////////////////////////////////
// vertexbuffer functions

CCoreVertexBuffer* CCoreDX11Device::CreateVertexBuffer( const TU8* Data, const TS32 Size )
{
  CCoreDX11VertexBuffer* Result = new CCoreDX11VertexBuffer( this );
  if ( !Result->Create( Data, Size ) )
    SAFEDELETE( Result );
  return Result;
}

CCoreVertexBuffer* CCoreDX11Device::CreateVertexBufferDynamic( const TS32 Size )
{
  CCoreDX11VertexBuffer* Result = new CCoreDX11VertexBuffer( this );
  if ( !Result->CreateDynamic( Size ) )
    SAFEDELETE( Result );
  return Result;
}

//////////////////////////////////////////////////////////////////////////
// indexbuffer functions

CCoreIndexBuffer* CCoreDX11Device::CreateIndexBuffer( const TS32 IndexCount, const TS32 IndexSize )
{
  CCoreDX11IndexBuffer* Result = new CCoreDX11IndexBuffer( this );
  if ( !Result->Create( IndexCount, IndexSize ) )
    SAFEDELETE( Result );
  return Result;
}

//////////////////////////////////////////////////////////////////////////
// vertexformat functions

CCoreVertexFormat* CCoreDX11Device::CreateVertexFormat( const CArray<COREVERTEXATTRIBUTE>& Attributes, CCoreVertexShader* vs )
{
  CCoreDX11VertexFormat* Result = new CCoreDX11VertexFormat( this );
  if ( !Result->Create( Attributes, vs ) )
    SAFEDELETE( Result );
  return Result;
}

//////////////////////////////////////////////////////////////////////////
// shader functions

//#ifndef CORE_API_D3DX
//#include <D3Dcompiler.h>
//#pragma comment(lib,"d3dcompiler.lib")
//#endif

CCoreVertexShader* CCoreDX11Device::CreateVertexShader( LPCSTR Code, TS32 CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString* Err )
{
  if ( Err ) *Err = _T( "" );
  if ( !Code || !CodeSize || !EntryFunction || !ShaderVersion ) return NULL;

  CCoreDX11VertexShader* s = new CCoreDX11VertexShader( this );
  s->SetCode( CString( Code ), CString( EntryFunction ), CString( ShaderVersion ) );

  if ( !s->CompileAndCreate( Err ) )
  {
    SAFEDELETE( s );
    return NULL;
  }

  return s;
}

CCorePixelShader* CCoreDX11Device::CreatePixelShader( LPCSTR Code, TS32 CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString* Err )
{
  if ( Err ) *Err = _T( "" );
  if ( !Code || !CodeSize || !EntryFunction || !ShaderVersion ) return NULL;

  CCoreDX11PixelShader* s = new CCoreDX11PixelShader( this );
  s->SetCode( CString( Code ), CString( EntryFunction ), CString( ShaderVersion ) );

  if ( !s->CompileAndCreate( Err ) )
  {
    SAFEDELETE( s );
    return NULL;
  }

  return s;
}

CCoreVertexShader* CCoreDX11Device::CreateVertexShaderFromBlob( TU8* Code, TS32 CodeSize )
{
  CCoreDX11VertexShader* s = new CCoreDX11VertexShader( this );
  if ( !s->CreateFromBlob( Code, CodeSize ) )
  {
    SAFEDELETE( s );
    return NULL;
  }

  return s;
}

CCorePixelShader* CCoreDX11Device::CreatePixelShaderFromBlob( TU8* Code, TS32 CodeSize )
{
  CCoreDX11PixelShader* s = new CCoreDX11PixelShader( this );
  if ( !s->CreateFromBlob( Code, CodeSize ) )
  {
    SAFEDELETE( s );
    return NULL;
  }

  return s;
}

CCoreGeometryShader* CCoreDX11Device::CreateGeometryShader( LPCSTR Code, TS32 CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString* Err )
{
  if ( Err ) *Err = _T( "" );
  if ( !Code || !CodeSize || !EntryFunction || !ShaderVersion ) return NULL;

  CCoreDX11GeometryShader* s = new CCoreDX11GeometryShader( this );
  s->SetCode( CString( Code ), CString( EntryFunction ), CString( ShaderVersion ) );

  if ( !s->CompileAndCreate( Err ) )
  {
    SAFEDELETE( s );
    return NULL;
  }

  return s;
}

CCoreDomainShader* CCoreDX11Device::CreateDomainShader( LPCSTR Code, TS32 CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString* Err )
{
  if ( Err ) *Err = _T( "" );
  if ( !Code || !CodeSize || !EntryFunction || !ShaderVersion ) return NULL;

  CCoreDX11DomainShader* s = new CCoreDX11DomainShader( this );
  s->SetCode( CString( Code ), CString( EntryFunction ), CString( ShaderVersion ) );

  if ( !s->CompileAndCreate( Err ) )
  {
    SAFEDELETE( s );
    return NULL;
  }

  return s;
}

CCoreHullShader* CCoreDX11Device::CreateHullShader( LPCSTR Code, TS32 CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString* Err )
{
  if ( Err ) *Err = _T( "" );
  if ( !Code || !CodeSize || !EntryFunction || !ShaderVersion ) return NULL;

  CCoreDX11HullShader* s = new CCoreDX11HullShader( this );
  s->SetCode( CString( Code ), CString( EntryFunction ), CString( ShaderVersion ) );

  if ( !s->CompileAndCreate( Err ) )
  {
    SAFEDELETE( s );
    return NULL;
  }

  return s;
}

CCoreComputeShader* CCoreDX11Device::CreateComputeShader( LPCSTR Code, TS32 CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString* Err )
{
  if ( Err ) *Err = _T( "" );
  if ( !Code || !CodeSize || !EntryFunction || !ShaderVersion ) return NULL;

  CCoreDX11ComputeShader* s = new CCoreDX11ComputeShader( this );
  s->SetCode( CString( Code ), CString( EntryFunction ), CString( ShaderVersion ) );

  if ( !s->CompileAndCreate( Err ) )
  {
    SAFEDELETE( s );
    return NULL;
  }

  return s;
}

CCoreVertexShader* CCoreDX11Device::CreateVertexShader()
{
  return new CCoreDX11VertexShader( this );
}

CCorePixelShader* CCoreDX11Device::CreatePixelShader()
{
  return new CCoreDX11PixelShader( this );
}

CCoreGeometryShader* CCoreDX11Device::CreateGeometryShader()
{
  return new CCoreDX11GeometryShader( this );
}

CCoreHullShader* CCoreDX11Device::CreateHullShader()
{
  return new CCoreDX11HullShader( this );
}

CCoreDomainShader* CCoreDX11Device::CreateDomainShader()
{
  return new CCoreDX11DomainShader( this );
}

CCoreComputeShader* CCoreDX11Device::CreateComputeShader()
{
  return new CCoreDX11ComputeShader( this );
}

//////////////////////////////////////////////////////////////////////////
// renderstate

TBOOL CCoreDX11Device::ApplyRenderState( const CORESAMPLER Sampler, const CORERENDERSTATE RenderState, const CORERENDERSTATEVALUE Value )
{
  switch ( RenderState )
  {
  case CORERS_BLENDSTATE:
  {
    if ( !Value.BlendState )
    {
      DeviceContext->OMSetBlendState( NULL, NULL, 0xffffffff );
      CurrentBlendState = NULL;
      return true;
    }
    return Value.BlendState->Apply();
  }
  break;
  case CORERS_RASTERIZERSTATE:
  {
    if ( !Value.RasterizerState )
    {
      DeviceContext->RSSetState( NULL );
      CurrentRasterizerState = NULL;
      return true;
    }
    return Value.RasterizerState->Apply();
  }
  break;
  case CORERS_DEPTHSTENCILSTATE:
  {
    if ( !Value.DepthStencilState )
    {
      DeviceContext->OMSetDepthStencilState( NULL, 0 );
      CurrentDepthStencilState = NULL;
      return true;
    }
    return Value.DepthStencilState->Apply();
  }
  break;
  case CORERS_SAMPLERSTATE:
  {
    if ( !Value.SamplerState ) return false;
    return Value.SamplerState->Apply( Sampler );
  }
  break;
  case CORERS_TEXTURE:
  {
    if ( !Value.Texture )
    {
      ID3D11ShaderResourceView* null[ 1 ];
      null[ 0 ] = NULL;

      if ( Sampler >= CORESMP_PS0 && Sampler <= CORESMP_PS15 )
        DeviceContext->PSSetShaderResources( Sampler, 1, null );
      if ( Sampler >= CORESMP_VS0 && Sampler <= CORESMP_VS3 )
        DeviceContext->VSSetShaderResources( Sampler - CORESMP_VS0, 1, null );
      if ( Sampler >= CORESMP_GS0 && Sampler <= CORESMP_GS3 )
        DeviceContext->GSSetShaderResources( Sampler - CORESMP_GS0, 1, null );
      return true;
    }
    return ApplyTextureToSampler( Sampler, Value.Texture );
  }
  break;
  case CORERS_VERTEXFORMAT:
  {
    if ( !Value.VertexFormat )
    {
      CurrentVertexFormatSize = 0;
      DeviceContext->IASetInputLayout( NULL );
      return true;
    }

    CurrentVertexFormatSize = Value.VertexFormat->GetSize();
    return ApplyVertexFormat( Value.VertexFormat );
  }
  break;
  case CORERS_INDEXBUFFER:
  {
    if ( !Value.IndexBuffer )
    {
      DeviceContext->IASetIndexBuffer( NULL, DXGI_FORMAT_R16_UINT, 0 );
      return true;
    }
    return ApplyIndexBuffer( Value.IndexBuffer );
  }
  break;
  case CORERS_VERTEXSHADER:
  {
    if ( !Value.VertexShader )
    {
      DeviceContext->VSSetShader( NULL, NULL, 0 );
      return true;
    }
    return ApplyVertexShader( Value.VertexShader );
  }
  break;
  case CORERS_GEOMETRYSHADER:
  {
    if ( !Value.GeometryShader )
    {
      DeviceContext->GSSetShader( NULL, NULL, 0 );
      return true;
    }
    return ApplyGeometryShader( Value.GeometryShader );
  }
  break;
  case CORERS_HULLSHADER:
  {
    if ( !Value.GeometryShader )
    {
      DeviceContext->HSSetShader( NULL, NULL, 0 );
      return true;
    }
    return ApplyHullShader( Value.HullShader );
  }
  break;
  case CORERS_DOMAINSHADER:
  {
    if ( !Value.DomainShader )
    {
      DeviceContext->DSSetShader( NULL, NULL, 0 );
      return true;
    }
    return ApplyDomainShader( Value.DomainShader );
  }
  break;
  case CORERS_COMPUTESHADER:
  {
    if ( !Value.ComputeShader )
    {
      DeviceContext->DSSetShader( NULL, NULL, 0 );
      return true;
    }
    return ApplyComputeShader( Value.ComputeShader );
  }
  break;
  case CORERS_PIXELSHADER:
  {
    if ( !Value.PixelShader )
    {
      DeviceContext->PSSetShader( NULL, NULL, 0 );
      return true;
    }
    return ApplyPixelShader( Value.PixelShader );
  }
  break;
  default: return true;
  }
}

TBOOL CCoreDX11Device::SetNoVertexBuffer()
{
  DeviceContext->IASetVertexBuffers( 0, 1, NULL, 0, NULL );
  return true;
}

TBOOL CCoreDX11Device::CommitRenderStates()
{
  return true;
}

//////////////////////////////////////////////////////////////////////////
// display functions

TBOOL CCoreDX11Device::BeginScene()
{
  return true;
}

TBOOL CCoreDX11Device::EndScene()
{
  return true;
}

TBOOL CCoreDX11Device::Clear( const TBOOL clearPixels, const TBOOL clearDepth, const CColor& Color, const TF32 Depth, const TS32 Stencil )
{
  TS32 Flags = 0;

  float col[ 4 ] = { Color[ 0 ] / 255.0f, Color[ 1 ] / 255.0f, Color[ 2 ] / 255.0f, Color[ 3 ] / 255.0f };

  if ( clearPixels )
    DeviceContext->ClearRenderTargetView( BackBufferView, col );

  if ( clearDepth )
    DeviceContext->ClearDepthStencilView( DepthBufferView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, Depth, Stencil );

  //if (clearPixels) Flags|=D3DCLEAR_TARGET;
  //if (clearDepth) Flags|=D3DCLEAR_ZBUFFER;
  //return Device->Clear(0, NULL, Flags , Color, Depth, Stencil)==D3D_OK;
  return true;
}

TBOOL CCoreDX11Device::Flip( TBOOL Vsync )
{
  //RECT r;
  //r.top=r.left=0; 
  //r.right=D3DPP.BackBufferWidth; 
  //r.bottom=D3DPP.BackBufferHeight;

  //if (D3DPP.Windowed)
  //	return Device->Present(&r,NULL,NULL,NULL)==D3D_OK;
  //else
  //	return Device->Present(NULL,NULL,NULL,NULL)==D3D_OK;


  //VSYNC: SwapChain->Present(1,0);
  //NON VSYNC: SwapChain->Present(0,0);

  HRESULT res;

  if ( Vsync ) res = SwapChain->Present( 1, 0 );
  else res = SwapChain->Present( 0, 0 );

  //if (res!=S_OK)
  //{
  //	_com_error err(res);
  //	LOG(LOG_ERROR,_T("[core] Present failed (%s)"),err.ErrorMessage());
  //}
  return res == S_OK;
}

TBOOL CCoreDX11Device::DrawIndexedTriangles( TS32 Count, TS32 NumVertices )
{
  DeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
  if ( !ApplyRequestedRenderState() ) return false;
  DeviceContext->DrawIndexed( Count * 3, 0, 0 );
  return true;
}

TBOOL CCoreDX11Device::DrawLines( TS32 Count )
{
  DeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
  if ( !ApplyRequestedRenderState() ) return false;
  DeviceContext->Draw( Count * 2, 0 );
  return true;
}

TBOOL CCoreDX11Device::DrawIndexedLines( TS32 Count, TS32 NumVertices )
{
  DeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
  if ( !ApplyRequestedRenderState() ) return false;
  DeviceContext->DrawIndexed( Count * 2, 0, 0 );
  return true;
}

TBOOL CCoreDX11Device::DrawTriangles( TS32 Count )
{
  DeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
  if ( !ApplyRequestedRenderState() ) return false;
  DeviceContext->Draw( Count * 3, 0 );
  return true;
}

TBOOL CCoreDX11Device::SetViewport( CRect Viewport )
{
  D3D11_VIEWPORT viewport;
  memset( &viewport, 0, sizeof( D3D11_VIEWPORT ) );

  viewport.TopLeftX = (TF32)Viewport.x1;
  viewport.TopLeftY = (TF32)Viewport.y1;
  viewport.Width = max( 0, (TF32)Viewport.Width() );
  viewport.Height = max( 0, (TF32)Viewport.Height() );
  viewport.MinDepth = 0;
  viewport.MaxDepth = 1;
  DeviceContext->RSSetViewports( 1, &viewport );

  return true;
}

void CCoreDX11Device::SetShaderConstants( TS32 Slot, TS32 Count, CCoreConstantBuffer** Buffers )
{
  void* buffers[ 16 ];

  if ( Buffers )
  {
    for ( TS32 x = 0; x < Count; x++ )
      buffers[ x ] = Buffers[ x ] ? Buffers[ x ]->GetBufferPointer() : NULL;
  }
  else
  {
    memset( buffers, 0, 16 * sizeof( void* ) );
  }

  DeviceContext->VSSetConstantBuffers( Slot, Count, (ID3D11Buffer**)buffers );
  DeviceContext->GSSetConstantBuffers( Slot, Count, (ID3D11Buffer**)buffers );
  DeviceContext->PSSetConstantBuffers( Slot, Count, (ID3D11Buffer**)buffers );
}

CCoreConstantBuffer* CCoreDX11Device::CreateConstantBuffer()
{
  return new CCoreDX11ConstantBuffer( this );
}

CCoreBlendState* CCoreDX11Device::CreateBlendState()
{
  return new CCoreDX11BlendState( this );
}

CCoreDepthStencilState* CCoreDX11Device::CreateDepthStencilState()
{
  return new CCoreDX11DepthStencilState( this );
}

CCoreRasterizerState* CCoreDX11Device::CreateRasterizerState()
{
  return new CCoreDX11RasterizerState( this );
}

CCoreSamplerState* CCoreDX11Device::CreateSamplerState()
{
  return new CCoreDX11SamplerState( this );
}

void CCoreDX11Device::SetCurrentDepthStencilState( ID3D11DepthStencilState* bs )
{
  CurrentDepthStencilState = bs;
}

ID3D11DepthStencilState* CCoreDX11Device::GetCurrentDepthStencilState()
{
  return CurrentDepthStencilState;
}

void CCoreDX11Device::SetCurrentRasterizerState( ID3D11RasterizerState* bs )
{
  CurrentRasterizerState = bs;
}

ID3D11RasterizerState* CCoreDX11Device::GetCurrentRasterizerState()
{
  return CurrentRasterizerState;
}

void CCoreDX11Device::SetCurrentBlendState( ID3D11BlendState* bs )
{
  CurrentBlendState = bs;
}

ID3D11BlendState* CCoreDX11Device::GetCurrentBlendState()
{
  return CurrentBlendState;
}

TBOOL CCoreDX11Device::SetRenderTarget( CCoreTexture2D* RT )
{
  if ( !RT )
  {
    //DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews( 1, &BackBufferView, DepthBufferView, 0, 0, NULL, NULL );
    DeviceContext->OMSetRenderTargets( 1, &BackBufferView, DepthBufferView );
    return true;
  }

  return false;
}

void CCoreDX11Device::ForceStateReset()
{
  CurrentVertexBuffer = NULL;
  CurrentRenderState.Flush();
  CurrentBlendState = NULL;
  CurrentDepthStencilState = NULL;
  CurrentRasterizerState = NULL;
}

CCoreTexture2D* CCoreDX11Device::CopyTexture( CCoreTexture2D* Texture )
{
  return Texture->Copy();
}

void CCoreDX11Device::TakeScreenShot( CString Filename )
{
  ID3D11Texture2D* bb;

  HRESULT res = SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (LPVOID*)&bb );
  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_ERROR, _T( "[core] DirectX11 Swapchain buffer acquisition failed (%s)" ), err.ErrorMessage() );
    return;
  }

  //D3DX11SaveTextureToFile(DeviceContext, bb, D3DX11_IFF_PNG, Filename.GetPointer());

  //CStreamWriterMemory Writer;
  //HRESULT res = SaveDDSTexture(DeviceContext, bb, Writer);

//	CCoreTexture2D *t=new

  CCoreDX11Texture2D* dummy = new CCoreDX11Texture2D( this );
  dummy->SetTextureHandle( bb );

  dummy->ExportToImage( Filename, true, CORE_PNG, false );

  dummy->SetTextureHandle( NULL );
  dummy->SetView( NULL );
  SAFEDELETE( dummy );


  bb->Release();

  LOG_NFO( "[core] Screenshot %s saved", Filename.GetPointer() );
}

#ifdef ENABLE_PIX_API
#define DONT_SAVE_VSGLOG_TO_TEMP
#include "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\include\vsgcapture.h"
#endif

void CCoreDX11Device::InitializeDebugAPI()
{
#ifdef ENABLE_PIX_API
  //InitVsPix();
#endif
}

void CCoreDX11Device::CaptureCurrentFrame()
{
#ifdef ENABLE_PIX_API
  g_pVsgDbg->CaptureCurrentFrame();
#endif
}

void CCoreDX11Device::BeginOcclusionQuery()
{
  if ( OcclusionQuery )
    DeviceContext->Begin( OcclusionQuery );
}

TBOOL CCoreDX11Device::EndOcclusionQuery()
{
  if ( OcclusionQuery )
  {
    DeviceContext->End( OcclusionQuery );

    UINT64 queryData; // This data type is different depending on the query type
    while ( S_OK != DeviceContext->GetData( OcclusionQuery, &queryData, sizeof( UINT64 ), 0 ) )
    {
    }

    return queryData > 0;
  }

  return 0;
}

void CCoreDX11Device::WaitRetrace()
{
  if ( swapChainRetraceObject )
    WaitForSingleObjectEx(swapChainRetraceObject, 1000, true);
}

ID3D11Texture2D* CCoreDX11Device::GetBackBuffer()
{
  ID3D11Texture2D* bb;

  HRESULT res = SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (LPVOID*)&bb );
  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_ERROR, _T( "[core] DirectX11 Swapchain buffer acquisition failed (%s)" ), err.ErrorMessage() );
    return NULL;
  }

  return bb;
}

#else
NoEmptyFile();
#endif