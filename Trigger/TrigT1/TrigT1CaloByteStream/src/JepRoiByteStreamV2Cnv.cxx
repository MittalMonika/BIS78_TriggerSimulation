/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


#include <vector>

#include "ByteStreamCnvSvcBase/ByteStreamAddress.h"
#include "ByteStreamCnvSvcBase/IByteStreamEventAccess.h"

#include "ByteStreamData/RawEvent.h"
#include "ByteStreamData/ROBData.h"

#include "AthContainers/DataVector.h"

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/StatusCode.h"

#include "AthenaKernel/ClassID_traits.h"
#include "AthenaKernel/StorableConversions.h"

#include "TrigT1CaloEvent/JEPRoIBSCollectionV2.h"

#include "JepRoiByteStreamV2Cnv.h"
#include "JepRoiByteStreamV2Tool.h"

namespace LVL1BS {

JepRoiByteStreamV2Cnv::JepRoiByteStreamV2Cnv( ISvcLocator* svcloc )
    : Converter( storageType(), classID(), svcloc ),
      m_name("JepRoiByteStreamV2Cnv"),
      m_tool("LVL1BS::JepRoiByteStreamV2Tool/JepRoiByteStreamV2Tool"),
      m_ByteStreamEventAccess("ByteStreamCnvSvc", m_name),
      m_log(msgSvc(), m_name), m_debug(false)
{
}

JepRoiByteStreamV2Cnv::~JepRoiByteStreamV2Cnv()
{
}

// CLID

const CLID& JepRoiByteStreamV2Cnv::classID()
{
  return ClassID_traits<LVL1::JEPRoIBSCollectionV2>::ID();
}

long JepRoiByteStreamV2Cnv::storageType()
{
  return ByteStreamAddress::storageType();
}

//  Init method gets all necessary services etc.


StatusCode JepRoiByteStreamV2Cnv::initialize()
{
  m_debug = msgSvc()->outputLevel(m_name) <= MSG::DEBUG;
  m_log << MSG::DEBUG << "Initializing " << m_name << " - package version "
                      << PACKAGE_VERSION << endmsg;

  StatusCode sc = Converter::initialize();
  if ( sc.isFailure() )
    return sc;

  //Get ByteStreamCnvSvc
  sc = m_ByteStreamEventAccess.retrieve();
  if ( sc.isFailure() ) {
    m_log << MSG::ERROR << "Failed to retrieve service "
          << m_ByteStreamEventAccess << endmsg;
    return sc;
  } else {
    m_log << MSG::DEBUG << "Retrieved service "
          << m_ByteStreamEventAccess << endmsg;
  }

  // Retrieve Tool
  sc = m_tool.retrieve();
  if ( sc.isFailure() ) {
    m_log << MSG::ERROR << "Failed to retrieve tool " << m_tool << endmsg;
    return StatusCode::FAILURE;
  } else m_log << MSG::DEBUG << "Retrieved tool " << m_tool << endmsg;

  return StatusCode::SUCCESS;
}

// createRep should create the bytestream from RDOs.

StatusCode JepRoiByteStreamV2Cnv::createRep( DataObject* pObj,
                                             IOpaqueAddress*& pAddr )
{
  if (m_debug) m_log << MSG::DEBUG << "createRep() called" << endmsg;

  RawEventWrite* re = m_ByteStreamEventAccess->getRawEvent();

  LVL1::JEPRoIBSCollectionV2* jep = 0;
  if( !SG::fromStorable( pObj, jep ) ) {
    m_log << MSG::ERROR << " Cannot cast to JEPRoIBSCollectionV2" << endmsg;
    return StatusCode::FAILURE;
  }

  const std::string nm = pObj->registry()->name();

  ByteStreamAddress* addr = new ByteStreamAddress( classID(), nm, "" );

  pAddr = addr;

  // Convert to ByteStream
  return m_tool->convert( jep, re );
}

} // end namespace
