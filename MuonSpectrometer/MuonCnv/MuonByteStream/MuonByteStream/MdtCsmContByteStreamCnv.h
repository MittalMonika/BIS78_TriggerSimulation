/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONBYTESTREAM_MDTCSMCONTRAWEVENTCNV_H
#define MUONBYTESTREAM_MDTCSMCONTRAWEVENTCNV_H

#include <stdint.h>
#include <map>
#include "GaudiKernel/Converter.h"
//s#include "GaudiKernel/svcType_t.h"

#include "ByteStreamData/RawEvent.h" 
#include "ByteStreamCnvSvcBase/ByteStreamAddress.h" 
#include "ByteStreamCnvSvcBase/IByteStreamEventAccess.h" 
#include <string>
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonMDT_CnvTools/IMDT_RDOtoByteStreamTool.h"

//class DataObject;
class IAddressCreator;
class StoreGateSvc; 

// Abstract factory to create the converter
template <class TYPE> class CnvFactory;

// Externals 
//extern svcType_t ByteStream_StorageType;
extern long ByteStream_StorageType;

class MdtCsmContByteStreamCnv: public Converter {

 public:
  MdtCsmContByteStreamCnv(ISvcLocator* svcloc);

  typedef Muon::IMDT_RDOtoByteStreamTool  BYTESTREAMTOOL ; 

  virtual StatusCode initialize();
  virtual StatusCode createObj(IOpaqueAddress* /**pAddr*/, DataObject*& /**pObj*/){
    return StatusCode::FAILURE;
  }
  virtual StatusCode createRep(DataObject* pObj, IOpaqueAddress*& pAddr);

  /// Storage type and class ID
  virtual long repSvcType() const { return ByteStream_StorageType;}
  static  long storageType()     { return ByteStream_StorageType; }
  static const CLID& classID();

private: 

   ToolHandle<BYTESTREAMTOOL> m_tool ; 

   ServiceHandle<IByteStreamEventAccess>    m_byteStreamEventAccess; 

   ServiceHandle<StoreGateSvc>              m_storeGate;
};
#endif



