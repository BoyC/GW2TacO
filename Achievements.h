#pragma once
#include "Bedrock/BaseLib/BaseLib.h"
#include <thread>

struct Achievement
{
  bool done = false;
  CArray<TS32> bits;
};

class Achievements
{
  static bool beingFetched;

  static int lastFetchTime;
  static std::thread fetchThread;

public:
  static bool fetched;
  static CDictionary<TS32, Achievement> achievements;
  static LIGHTWEIGHT_CRITICALSECTION critSec;

  static void FetchAchievements();
  static void WaitForFetch();
};

