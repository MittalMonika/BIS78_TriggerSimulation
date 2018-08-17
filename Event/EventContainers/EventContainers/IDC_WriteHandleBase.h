/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/


#ifndef IDC_WRITEHANDLEBASE_H
#define IDC_WRITEHANDLEBASE_H
#include <atomic>
#include <condition_variable>
#include <mutex>

namespace EventContainers {

struct mutexPair{
   std::condition_variable condition;
   std::mutex mutex;
#ifndef NDEBUG
   std::atomic<int> counter;
#endif
   mutexPair() : condition(), mutex() {
#ifndef NDEBUG
      counter.store(0);
#endif
   }
};


class IDC_WriteHandleBase{
protected:
   std::atomic<const void*>*  m_atomic;
   mutexPair *m_mut;   
   IDC_WriteHandleBase();
public:

   void LockOn(std::atomic<const void*>* in, mutexPair *pair){
      m_atomic = in;
      m_mut    = pair;
   }
   void DropLock();
   void ReleaseLock();

   ~IDC_WriteHandleBase();
};

}
#endif

