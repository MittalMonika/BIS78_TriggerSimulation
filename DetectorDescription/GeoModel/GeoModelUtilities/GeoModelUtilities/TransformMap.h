/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELUTILITIES_TRANSFORMMAP_H
#define GEOMODELUTILITIES_TRANSFORMMAP_H

#include <unordered_map>

template<typename T, typename X> 
class TransformMap {
 public:
  TransformMap() {};
  ~TransformMap() {};

  bool setTransform(const T* obj, const X& xf);
  const X* getTransform(const T* obj) const;

 private:
  std::unordered_map<const T*, X> m_container;
};

template<typename T, typename X> 
bool TransformMap<T,X>::setTransform(const T* obj, const X& xf)
{
  auto result = m_container.emplace(obj, xf);
  return result.second;
}

template<typename T, typename X> 
const X* TransformMap<T,X>::getTransform(const T* obj) const
{
  auto it = m_container.find(obj);
  if(it!=m_container.end()) {
    return &it->second;
  }
  else {
    return nullptr;
  }
}

#endif 



