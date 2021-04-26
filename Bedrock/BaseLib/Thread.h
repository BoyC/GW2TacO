#pragma once
#include <Thread>

class CThread
{
public:

  virtual void Start() {}
  virtual void Run( void *This ) = 0;

  CThread()
  {
    //std::thread 
  }

  virtual ~CThread()
  {

  }
};