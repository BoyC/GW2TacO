#pragma once
#include "../BaseLib/BaseLib.h"

TBOOL DecompressPNG( const TU8 *IData, TS32 IDataSize, TU8 *&Image, TS32 &XRes, TS32 &YRes );
void ARGBtoABGR( TU8 *Image, TS32 XRes, TS32 YRes );
void ClearZeroAlpha( TU8 *Image, TS32 XRes, TS32 YRes );

TBOOL ExportPNG( TU8 *IData, TS32 XRes, TS32 YRes, TBOOL ClearAlpha, CString OutFile );
TBOOL ExportTga( TU8 *IData, TS32 XRes, TS32 YRes, TBOOL ClearAlpha, CString OutFile );
TBOOL ExportBmp( TU8 *Image, TS32 XRes, TS32 YRes, CString OutFile );
