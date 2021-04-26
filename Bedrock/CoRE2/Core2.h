#pragma once
#include "Core2_Config.h"

//include base library

#include "../BaseLib/BaseLib.h"

//CoRE2 includes follow

#ifdef CORE_API_DX9

#include "DX9Device.h"
typedef CCoreDX9Device CCore;
//#define UVTRANSLATION(uv,res) ((uv+0.5f)/(TF32)(res))
//#define SCREENSPACETRANSLATION(a,b,c,d) CVector2((TF32)(a),(TF32)(b))

#endif

#ifdef CORE_API_DX11

#include "DX11Device.h"
typedef CCoreDX11Device CCore;
//#define UVTRANSLATION(uv,res) ((uv+0.5f)/(TF32)(res))
//#define SCREENSPACETRANSLATION(a,b,c,d) CVector4((a+0.5f)/(TF32)(c)*2.0f-1,(-b-0.5f)/(TF32)(d)*2.0f+1,0,1.0f)
//#define SCREENSPACETRANSLATION(a,b,c,d) CVector2((a)/(TF32)(c)*2.0f-1,(-b)/(TF32)(d)*2.0f+1)
//#define SCREENSPACETRANSLATION(a,b,c,d) CVector2((TF32)(a),(TF32)(b))

#endif

#define UVTRANSLATION(uv,res) ((uv+UVOffset)/(TF32)(res))


#include "Resource.h"
#include "Texture.h"
#include "Scene.h"

