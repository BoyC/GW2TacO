#pragma once

class CJobHandler;

class CJobThread : public CThread
{

  CJobHandler *JobHandler;

public:

  CJobThread();
  virtual ~CJobThread();

};

class CJob
{

public:

  CJob();
  virtual ~CJob();

};

class CJobHandler
{

  CArray<CJobThread*> Threads;
  CArray<CJob*> Queue;

public:

  CJobHandler();
  virtual ~CJobHandler();

};