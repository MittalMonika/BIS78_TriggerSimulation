/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: IdentifiableCacheBase.cxx 791541 2017-01-09 10:43:53Z smh $
/**
 * @file IdentifiableCacheBase.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2016
 * @brief 
 */

// Not in ids, null --- default state.
// In ids, INVALID -- conversion/add failed.
// In ids, valid --- Have payload
// In ids, null --- Conversion in progress.
// Not in ids, INVALID --- intention to add soon

#include "EventContainers/IdentifiableCacheBase.h"


using namespace std;
namespace EventContainers {

static const void* INVALID = reinterpret_cast<const void*>(IdentifiableCacheBase::INVALIDflag);
static const void* ABORTED = reinterpret_cast<const void*>(IdentifiableCacheBase::ABORTEDflag);

IdentifiableCacheBase::IdentifiableCacheBase (IdentifierHash maxHash,
                                              const IMaker* maker)
  : m_vec(maxHash),
    m_owns(maxHash, false),
    m_maker (maker)
{
   for(auto &h : m_vec) h.store(nullptr, std::memory_order_relaxed); //Ensure initialized to null - I'm not sure if this is implicit
}


IdentifiableCacheBase::~IdentifiableCacheBase()
{
  if (!m_ids.empty())
    std::abort();
}

int IdentifiableCacheBase::tryLock(IdentifierHash hash, IDC_Lock &lock, std::vector<waitlistPair> &wait){
   const void *ptr1 = m_vec[hash]; //atomic load
   if(ptr1==nullptr){
      size_t slot = hash % s_lockBucketSize;
      lock_t lockhdl(m_mapMutexes[slot]);
      if(m_vec[hash].compare_exchange_strong(ptr1, INVALID)){//atomic swap
         //First call

         auto &mutexpair = m_HoldingMutexes[slot][hash]; //make new pair in unmoving memory
         
         lock.LockOn(&m_vec[hash], &mutexpair);
         return 0;
      }
   }

   if(ptr1 == INVALID){
      size_t slot = hash % s_lockBucketSize;
      lock_t lockhdl(m_mapMutexes[slot]);
      //Second call while not finished
      wait.emplace_back(m_HoldingMutexes[slot].find(hash));
      return 1;
   }
   if(ptr1 == ABORTED) return 3;
   return 2;   //Already completed
}


void IdentifiableCacheBase::clear (deleter_f* deleter)
{
  lock_t lock (m_mutex);
  for (IdentifierHash hash : m_ids) {
    if (m_owns[hash])
      deleter (m_vec[hash]);
    m_owns[hash] = false;
  }
  m_ids.clear();
  for(auto& h : m_vec) h.store(nullptr, std::memory_order_relaxed); //This should not be being called in a thread contensious situation
}


//Does not lock or clear atomics to allow faster destruction
void IdentifiableCacheBase::cleanUp (deleter_f* deleter)
{
  for (IdentifierHash hash : m_ids) {
    if (m_owns[hash])
      deleter (m_vec[hash]);
  }
  m_ids.clear();
}

int IdentifiableCacheBase::itemAborted (IdentifierHash hash) const{
   const void* p = m_vec[hash].load();
   return (p == ABORTED);
}


int IdentifiableCacheBase::itemInProgress (IdentifierHash hash) const{
   const void* p = m_vec[hash].load();
   return (p == INVALID);
}


const void* IdentifiableCacheBase::find (IdentifierHash hash) const
{
  if (hash >= m_vec.size()) return nullptr;
  const void* p = m_vec[hash].load();
  if (p >= ABORTED)
    return nullptr;
  return p;
}



const void* IdentifiableCacheBase::findWait (IdentifierHash hash) const
{
  if (hash >= m_vec.size()) return nullptr;
  const void* p = m_vec[hash];
  if(p<ABORTED) return p;
  size_t slot = hash % s_lockBucketSize;
  std::unique_lock<std::mutex> lockmap(m_mapMutexes[slot]);

  mutexPair &mutpair = m_HoldingMutexes[slot][hash]; //make new pair in unmoving memory
  lockmap.unlock();
  while(m_vec[hash]==INVALID){
     std::unique_lock<decltype(mutpair.mutex)> lk(mutpair.mutex);//mutex associated with condition
     while(itemInProgress(hash)){
        mutpair.condition.wait(lk);
     }
  }
  p = m_vec[hash];
  if(p>=ABORTED) return nullptr;
  return p;
}

const void* IdentifiableCacheBase::get (IdentifierHash hash)
{
  // If it's there already, return directly without locking.
  if (hash >= m_vec.size()) return nullptr;
  if (m_vec[hash]) return find (hash);

  std::unique_lock<mutex_t> lock (m_mutex);
  {
    // Check again.
    if (m_vec[hash]) return find (hash);
    idset_t::iterator it = m_ids.find (hash);

    if (it != m_ids.end()) {
      // It's in the set.  Someone's running the conversion already.
      // Wait for the condition, then check again.
      while (!m_vec[hash])
        m_cond.wait (lock);
      return find (hash);
    }

    // We're going to try to do a conversion.
    // Enter the id in the set.
    m_ids.insert (hash);

    // If there's no maker, mark it as invalid.
    if (!m_maker) {
      m_vec[hash] = INVALID;
      return nullptr;
    }

    // Let go of the lock.
  }

  // Make the payload.
  const void* p = nullptr;
  try {
    p = m_maker->typelessMake (hash).release();
  }
  catch (...) {
    // FIXME: Can this be done with RAII?
    m_vec[hash] = INVALID;
    m_cond.notify_all();
    throw;
  }

  // Stick it in the vector; notify other threads.
  if (p) {
    m_vec[hash] = p;
    m_owns[hash] = true;
  }
  else{
    m_vec[hash] = INVALID;
  }
  m_cond.notify_all();
  return p;
}
#ifdef IdentifiableCacheBaseRemove
bool IdentifiableCacheBase::remove (IdentifierHash hash)
{
   if (hash >= m_vec.size()) return false;
   lock_t lock (m_mutex);
   idset_t::const_iterator it = m_ids.find (hash);
   if(it != m_ids.end()){
      m_vec[hash] = nullptr;
      m_owns[hash] = false;
      m_ids.erase(it);
   }
   return true;
}
#endif
size_t IdentifiableCacheBase::numberOfHashes() const
{
  lock_t lock (m_mutex);
  return m_ids.size();
}

std::vector<IdentifierHash> IdentifiableCacheBase::ids() const
{
  std::vector<IdentifierHash> ret;
  ret.reserve (m_vec.size());
  lock_t lock (m_mutex);
  for (IdentifierHash hash : m_ids) {
    const void* p = m_vec[hash].load();
    if (p && p < ABORTED)
      ret.push_back (hash);
  }
  return ret;
}


bool IdentifiableCacheBase::add (IdentifierHash hash, const void* p, bool owns)
{
  if (hash >= m_vec.size()) {
//     cout << "Out of bounds " << hash << " / " << m_vec.size() << endl;
     return false;
  }
  lock_t lock (m_mutex);
  idset_t::iterator it = m_ids.lower_bound (hash);
  if (it != m_ids.end() && *it == hash)
  {
//     cout << "iterator wrong " << endl;
     return false;
  }
  assert (m_vec[hash] == nullptr || m_vec[hash]==INVALID);//Can be invalid in the case of "tryfetch lock"
  m_vec[hash] = p ? p : INVALID;
  m_owns[hash] = p!=nullptr ? owns : false;
  m_ids.insert (it, hash);
  return true;
}


bool IdentifiableCacheBase::add (IdentifierHash hash,
                                 void_unique_ptr p)
{
  if (hash >= m_vec.size()) return false;
  lock_t lock (m_mutex);
  idset_t::iterator it = m_ids.lower_bound (hash);
  if (it != m_ids.end() && *it == hash)
    return false;
  assert (m_vec[hash] == nullptr || m_vec[hash]==INVALID);//Can be invalid in the case of "tryfetch lock"
  if (p) {
    m_vec[hash] = p.release();
    m_owns[hash] = true;
  }
  else {
    m_vec[hash] = INVALID;
  }
  m_ids.insert (it, hash);
  return true;
}



} // namespace EventContainers


