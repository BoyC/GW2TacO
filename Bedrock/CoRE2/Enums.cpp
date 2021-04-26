#include "BasePCH.h"
#include "Enums.h"

EnumNamePair ComparisonFunctionNames[]=
{
	{CORECMP_NEVER,			_T("Never")},
	{CORECMP_LESS,			_T("Less")},
	{CORECMP_EQUAL,			_T("Equal")},
	{CORECMP_LEQUAL,		_T("LessEqual")},
	{CORECMP_GREATER,		_T("Greater")},
	{CORECMP_NOTEQUAL,		_T("NotEqual")},
	{CORECMP_GEQUAL,		_T("GreaterEqual")},
	{CORECMP_ALWAYS,		_T("Always")},
	{-1,NULL}
};

EnumNamePair BlendFactorNames[]=
{
	{COREBLEND_ZERO,				_T("Zero")},
	{COREBLEND_ONE,					_T("One")},
	{COREBLEND_SRCCOL,				_T("SrcCol")},
	{COREBLEND_INVSRCCOL,			_T("InvSrcCol")},
	{COREBLEND_SRCALPHA,			_T("SrcAlpha")},
	{COREBLEND_INVSRCALPHA,			_T("InvSrcAlpha")},
	{COREBLEND_DSTALPHA,			_T("DstAlpha")},
	{COREBLEND_INVDSTALPHA,			_T("InvDstAlpha")},
	{COREBLEND_DSTCOLOR,			_T("DstCol")},
	{COREBLEND_INVDSTCOLOR,			_T("InvDstCol")},
	{COREBLEND_SRCALPHASATURATE,	_T("SrcAlphaSaturate")},
	{COREBLEND_BLENDFACTOR,			_T("BlendFactor")},
	{COREBLEND_INVBLENDFACTOR,		_T("InvBlendFactor")},
};


EnumNamePair BlendOpNames[]=
{
	{COREBLENDOP_ADD,		_T("Add")},
	{COREBLENDOP_SUB,		_T("Sub")},
	{COREBLENDOP_REVSUB,	_T("RevSub")},
	{COREBLENDOP_MIN,		_T("Min")},
	{COREBLENDOP_MAX,		_T("Max")},
	{-1,NULL}
};

EnumNamePair CullModeNames[]=
{
	{CORECULL_NONE,			_T("None")},
	{CORECULL_CW,			_T("CW")},
	{CORECULL_CCW,			_T("CCW")},
	{-1,NULL}
};

EnumNamePair FillModeNames[]=
{
	{COREFILL_SOLID,		_T("Solid")},
	{COREFILL_EDGES,		_T("Edges")},
	{COREFILL_POINTS,		_T("Points")},
	{-1,NULL}
};

EnumNamePair AddressModeNames[]=
{
	{CORETEXADDRESS_WRAP,		_T("Wrap")},
	{CORETEXADDRESS_MIRROR,		_T("Mirror")},
	{CORETEXADDRESS_CLAMP,		_T("Clamp")},
	{CORETEXADDRESS_BORDER,		_T("Border")},
	{CORETEXADDRESS_MIRRORONCE,	_T("MirrorOnce")},
	{-1,NULL}
};

EnumNamePair SamplerNames[]=
{
	{CORESMP_PS0,			_T("PS0")},
	{CORESMP_PS1,			_T("PS1")},
	{CORESMP_PS2,			_T("PS2")},
	{CORESMP_PS3,			_T("PS3")},
	{CORESMP_PS4,			_T("PS4")},
	{CORESMP_PS5,			_T("PS5")},
	{CORESMP_PS6,			_T("PS6")},
	{CORESMP_PS7,			_T("PS7")},
	{CORESMP_PS8,			_T("PS8")},
	{CORESMP_PS9,			_T("PS9")},
	{CORESMP_PS10,			_T("PS10")},
	{CORESMP_PS11,			_T("PS11")},
	{CORESMP_PS12,			_T("PS12")},
	{CORESMP_PS13,			_T("PS13")},
	{CORESMP_PS14,			_T("PS14")},
	{CORESMP_PS15,			_T("PS15")},
	{CORESMP_VS0,			_T("VS0")},
	{CORESMP_VS1,			_T("VS1")},
	{CORESMP_VS2,			_T("VS2")},
	{CORESMP_VS3,			_T("VS3")},
	{CORESMP_GS0,			_T("GS0")},
	{CORESMP_GS1,			_T("GS1")},
	{CORESMP_GS2,			_T("GS2")},
	{CORESMP_GS3,			_T("GS3")},
	{-1,NULL}
};

EnumNamePair FilterNames[]=
{
	{COREFILTER_MIN_MAG_MIP_POINT                           ,_T("Min_Mag_Mip_Point")},
	{COREFILTER_MIN_MAG_POINT_MIP_LINEAR                    ,_T("Min_Mag_Point_Mip_Linear")},
	{COREFILTER_MIN_POINT_MAG_LINEAR_MIP_POINT              ,_T("Min_Point_Mag_Linear_Mip_Point")},
	{COREFILTER_MIN_POINT_MAG_MIP_LINEAR                    ,_T("Min_Point_Mag_Mip_Linear")},
	{COREFILTER_MIN_LINEAR_MAG_MIP_POINT                    ,_T("Min_Linear_Mag_Mip_Point")},
	{COREFILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR             ,_T("Min_Linear_Mag_Point_Mip_Linear")},
	{COREFILTER_MIN_MAG_LINEAR_MIP_POINT                    ,_T("Min_Mag_Linear_Mip_Point")},
	{COREFILTER_MIN_MAG_MIP_LINEAR                          ,_T("Min_Mag_Mip_Linear")},
	{COREFILTER_ANISOTROPIC                                 ,_T("Anisotropic")},
	{COREFILTER_COMPARISON_MIN_MAG_MIP_POINT                ,_T("Comparison_Min_Mag_Mip_Point")},
	{COREFILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR         ,_T("Comparison_Min_Mag_Point_Mip_Linear")},
	{COREFILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT   ,_T("Comparison_Min_Point_Mag_Linear_Mip_Point")},
	{COREFILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR         ,_T("Comparison_Min_Point_Mag_Mip_Linear")},
	{COREFILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT         ,_T("Comparison_Min_Linear_Mag_Mip_Point")},
	{COREFILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR  ,_T("Comparison_Min_Linear_Mag_Point_Mip_Linear")},
	{COREFILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT         ,_T("Comparison_Min_Mag_Linear_Mip_Point")},
	{COREFILTER_COMPARISON_MIN_MAG_MIP_LINEAR               ,_T("Comparison_Min_Mag_Mip_Linear")},
	{COREFILTER_COMPARISON_ANISOTROPIC                      ,_T("Comparison_Anisotropic")},
	{-1,NULL}
};
