/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TrigOutputHandling_BareDataBucket_h
#define TrigOutputHandling_BareDataBucket_h
#include "AthenaKernel/DataBucketBase.h"
#include "DataModelRoot/RootType.h"

/**
 * @class Allows to insert void* returned from serialisation into the store
 **/

class BareDataBucket: public DataBucketBase {
public:
  BareDataBucket() = delete;

  BareDataBucket( void * data, size_t sz, CLID clid, const RootType& type )
    : m_data(data), m_size(sz), m_clid(clid), m_type( type ){}

  virtual ~BareDataBucket() {
    if ( m_own )
      m_type.Destruct( m_data );
  }

  // DataObject overrides
  virtual const CLID& clID() const override {
    return m_clid;
  }
  
  // DataBuckedBase overrides

  virtual void* object() override { 
    return m_data; 
  }

  virtual const std::type_info& tinfo() const override { 
    return m_type.TypeInfo(); 
  };

  virtual void* cast (CLID clid,
                      SG::IRegisterTransient* ,
                      bool isConst = true) override {
    return ( m_clid == clid and isConst ) ? m_data : nullptr;
  }

  virtual void* cast (const std::type_info& tinfo,
                      SG::IRegisterTransient* ,
                      bool isConst = true) override {
    return ( tinfo == m_type.TypeInfo() and isConst ) ? m_data : nullptr;
  }

  virtual DataBucketBase* clone() const override {
    return nullptr;
  }
  
  virtual void relinquish() override { 
    m_own = false; 
  };
  
  virtual void lock() override { /*not lockable I think */ };

private:
  void* m_data = 0;
  size_t m_size = 0;
  CLID m_clid  = 0;
  RootType m_type;
  bool m_own = false;
};

#endif
