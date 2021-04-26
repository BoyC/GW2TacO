#pragma once
#include "Types.h"

#include "BaseConfig.h"
#include "StackTracker.h"
#include "Memory.h"

#define WIN32_LEAN_AND_MEAN

#include <WinSock2.h>
#include <Windows.h>
#include <comdef.h>

#include <tchar.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "SpecMath.h"

#include "Assert.h"
#include "Constants.h"
#include "CriticalSection.h"
#include "Color.h"
#include "Random.h"
#include "Timer.h"

#include "Array.h"
#include "Dictionary.h"
#include "RingBuffer.h"
#include "String.h"
#include "Logger.h"
#include "StreamReader.h"
#include "StreamWriter.h"
#include "Socket.h"
#include "Logger.h"
#include "FloatControl.h"

#include "Vector.h"
#include "Line.h"
#include "Plane.h"
#include "Sphere.h"
#include "Spline.h"
#include "Matrix.h"
#include "BBox.h"
#include "Quaternion.h"
#include "PRS.h"
#include "Rectangle.h"

#include "FileList.h"
#include "Profile.h"

#include "ImageDecompressor.h"
#include "Hash.h"

#include "Archive.h"
#include "EnumHelpers.h"

#include "Thread.h"
#include "JobHandler.h"

#include "CrashTracker.h"

#define NoEmptyFile()   namespace { char NoEmptyFileDummy##__LINE__; }