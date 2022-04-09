#pragma once
#include "Bedrock/baselib/baselib.h"

struct MumbleContext
{
  unsigned char serverAddress[ 28 ]; // contains sockaddr_in or sockaddr_in6
  uint32_t mapId;
  uint32_t mapType;
  uint32_t shardId;
  uint32_t instance;
  uint32_t buildId;
  // Additional data beyond the 48 bytes Mumble uses for identification
  uint32_t uiState; // Bitmask: Bit 1 = IsMapOpen, Bit 2 = IsCompassTopRight, Bit 3 = DoesCompassHaveRotationEnabled, Bit 4 = Game has focus, Bit 5 = Is in Competitive game mode, Bit 6 = Textbox has focus, Bit 7 = Is in Combat
  uint16_t compassWidth; // pixels
  uint16_t compassHeight; // pixels
  float compassRotation; // radians
  float playerX; // continentCoords
  float playerY; // continentCoords
  float mapCenterX; // continentCoords
  float mapCenterY; // continentCoords
  float mapScale;
  uint32_t processId;
  uint8_t mountIndex;
};

struct CompassData
{
  int compassWidth; // pixels
  int compassHeight; // pixels
  float compassRotation; // guessing... radians? :-P
  float playerX; // continentCoords
  float playerY; // continentCoords
  float mapCenterX; // continentCoords
  float mapCenterY; // continentCoords
  float mapScale; // not even sure TBH :-P};
  CMatrix4x4 BuildTransformationMatrix( const CRect& miniRect, bool ignoreRotation );
};

struct LinkedMem
{
#ifdef WIN32
  UINT32	uiVersion;
  DWORD	uiTick;
#else
  uint32_t uiVersion;
  uint32_t uiTick;
#endif
  float	fAvatarPosition[ 3 ];
  float	fAvatarFront[ 3 ];
  float	fAvatarTop[ 3 ];
  wchar_t	name[ 256 ];
  float	fCameraPosition[ 3 ];
  float	fCameraFront[ 3 ];
  float	fCameraTop[ 3 ];
  wchar_t	identity[ 256 ];
#ifdef WIN32
  UINT32	context_len;
#else
  uint32_t context_len;
#endif
  unsigned char context[ 256 ];
  wchar_t description[ 2048 ];
};

#define AVGCAMCOUNTER 6

class CMumbleLink
{
  LinkedMem* lm = NULL;
  CVector4 camchardist[ AVGCAMCOUNTER ];

public:
  LinkedMem lastData;
  LinkedMem prevData;

  int tick;
  double interpolation;

  CVector3 charPosition;
  CVector3 charEye;
  CVector3 camPosition;
  CVector3 camDir;
  CVector3 camUp;
  TF32 fov;
  TS32 mapID = 0;
  TS32 worldID = 0;
  TS32 mapType = 0;
  TS32 mapInstance = 0;
  TS32 charIDHash = 0;

  TS32 lastMapChangeTime = 0;
  bool isMapOpen; // bit 1: IsMapOpen, bit2: IsCompassTopRight, bit3: DoesCompassHaveRotationEnabled
  bool isMinimapTopRight;
  bool isMinimapRotating;
  bool gameHasFocus;
  bool isPvp;
  bool textboxHasFocus;
  bool isInCombat;

  CompassData miniMap;
  CompassData bigMap;

  TS32 uiSize = 1;

  CString charName;

  CVector4 averagedCharPosition;

  bool Update();
  TBOOL IsValid();

  CRingBuffer<TS32>* frameTimes;
  TF32 GetFrameRate();

  TS32 lastFrameTime = 0;
  CMumbleLink();
  virtual ~CMumbleLink();

  TU64 lastTickTime = 0;
  TU64 lastTickLength = 0;

  TBOOL charPosChanged = false;
  TBOOL charEyeChanged = false;
  TBOOL camPosChanged = false;
  TBOOL camDirChanged = false;
  TBOOL camUpChanged = false;

  CString mumblePath = L"MumbleLink";
  TU32 lastGW2ProcessID = 0;
};

extern CMumbleLink mumbleLink;
CRect GetMinimapRectangle();
void ChangeUIScale( int size );
