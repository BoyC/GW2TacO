#include "OverlayApplication.h"

COverlayApp::COverlayApp()
{
}

TBOOL COverlayApp::Initialize( const CCoreWindowParameters& WindowParams )
{
  if ( !CWBApplication::Initialize( WindowParams ) ) return false;

  CCoreBlendState* GuiBlendState = Device->CreateBlendState();
  if ( !GuiBlendState )
  {
    LOG( LOG_ERROR, _T( "[gui] Error creating UI Blend State" ) );
    return false;
  }

  GuiBlendState->SetBlendEnable( 0, true );
  GuiBlendState->SetSrcBlend( 0, COREBLEND_SRCALPHA );
  GuiBlendState->SetDestBlend( 0, COREBLEND_INVSRCALPHA );
  GuiBlendState->SetSrcBlendAlpha( 0, COREBLEND_ONE );
  GuiBlendState->SetDestBlendAlpha( 0, COREBLEND_INVSRCALPHA );

  DrawAPI->SetUIBlendState( GuiBlendState );

  holePunchBlendState = DrawAPI->GetDevice()->CreateBlendState();
  holePunchBlendState->SetBlendEnable( 0, true );
  holePunchBlendState->SetIndependentBlend( true );
  holePunchBlendState->SetSrcBlend( 0, COREBLEND_ZERO );
  holePunchBlendState->SetDestBlend( 0, COREBLEND_ZERO );
  holePunchBlendState->SetSrcBlendAlpha( 0, COREBLEND_ZERO );
  holePunchBlendState->SetDestBlendAlpha( 0, COREBLEND_ZERO );

  //CCoreSamplerState *GuiSampler = Device->CreateSamplerState();
  //GuiSampler->SetFilter(COREFILTER_COMPARISON_MIN_MAG_MIP_POINT);
  //DrawAPI->SetUISamplerState(GuiSampler);

  //LPCSTR shader =
  //	"Texture2D GuiTexture:register(t0);"
  //	"SamplerState Sampler:register(s0);"
  //	"cbuffer resdata : register(b0)"
  //	"{							   "
  //	"		float4 resolution;	   "
  //	"}"
  //	"struct VSIN { float4 Position : POSITIONT; float2 UV : TEXCOORD0; float4 Color : COLOR0; };"
  //	"struct VSOUT { float4 Position : SV_POSITION; float2 UV : TEXCOORD0; float4 Color : COLOR0; };"
  //	"VSOUT vsmain(VSIN x) { VSOUT k; k.Position=float4(x.Position.x/resolution.x*2-1,-x.Position.y/resolution.y*2+1,0,1); k.UV=x.UV; k.Color=x.Color; return k; }"
  //	"float4 psmain(VSOUT x) : SV_TARGET0 { float4 col=x.Color*GuiTexture.Sample(Sampler,x.UV); return float4(col.xyz*col.a,col.a); }";

  //auto PxShader = Device->CreatePixelShader(shader, strlen(shader), "psmain", "ps_4_0");
  //DrawAPI->SetPixelShader(PxShader);

  return true;
}


COverlayApp::~COverlayApp()
{
  SAFEDELETE( holePunchBlendState );
}

void COverlayApp::TakeScreenshot()
{

}

TBOOL COverlayApp::DeviceOK()
{
  if ( !Device ) return false;
  return Device->DeviceOk();
}

void FetchMarkerPackOnline( CString& ourl );


LRESULT COverlayApp::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  switch ( uMsg )
  {
  case WM_COPYDATA:
    PCOPYDATASTRUCT pcpy = (PCOPYDATASTRUCT)lParam;
    if ( pcpy )
    {
      CString incoming( (TS8*)( pcpy->lpData ), pcpy->cbData );
      FetchMarkerPackOnline( incoming );
    }
    break;
  }

  if ( uMsg > 0x60ff )
  {
    int z = 0;
  }

  //switch (uMsg)
  //{
  //	case WM_WINDOWPOSCHANGED:
 //   {
 //     HWND gw2Window = FindWindow( NULL, "Guild Wars 2" );
 //     if ( gw2Window )
 //     {
 //       HWND wnd = ::GetNextWindow( (HWND)GetHandle(), GW_HWNDNEXT );
 //       if ( wnd != gw2Window )
 //         ::SetWindowPos( gw2Window, (HWND)GetHandle(), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
 //       //::SetWindowPos( (HWND)GetHandle(), gw2Window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOSENDCHANGING );
 //     }
 //   }
  //}

  return CWBApplication::WindowProc( uMsg, wParam, lParam );
}