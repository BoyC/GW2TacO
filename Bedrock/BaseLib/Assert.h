#pragma once
#include "BaseConfig.h"

#ifndef MEMORY_TRACKING
//memtracking disabled:

#ifdef _DEBUG
#include <stdlib.h>
#include <crtdbg.h>

#define BASEASSERT(v) \
	do { \
	if (!(v)) { \
	_CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, ASSERTMODULENAME, #v "\n"); \
	DebugBreak(); \
	} \
	} while (0)
#define BASEASSERTR(v,r) \
	do { \
	if (!(v)) { \
	_CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, ASSERTMODULENAME, #v "\n"); \
	DebugBreak(); return r;\
	} \
	} while (0)
#else
#define BASEASSERT(v) \
	do { (v); } while (0)
#define BASEASSERTR(v,r) \
	do { (v); } while (0)
#endif

#else
//memtracking enabled:

#ifdef _DEBUG
#define BASEASSERT(v) \
	do { \
	if (!(v)) { \
	DebugBreak(); \
	} \
	} while (0)
#define BASEASSERTR(v,r) \
	do { \
	if (!(v)) { \
	DebugBreak(); return r; \
	} \
	} while (0)
#else
#define BASEASSERT(v) \
	do { (v); } while (0)
#define BASEASSERTR(v,r) \
	do { (v); } while (0)
#endif


#endif