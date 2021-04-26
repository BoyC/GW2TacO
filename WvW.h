#pragma once
#include "Bedrock/BaseLib/BaseLib.h"

class WvWObjective
{
public:
  CString id;
  CString type;
  CString mapType;
  int mapID;
  int objectiveID;
  CVector3 coord;
  CString marker;
  CString chatLink;

  CString name;
  CString nameToken;
};

void LoadWvWObjectives();
void UpdateWvWStatus();
