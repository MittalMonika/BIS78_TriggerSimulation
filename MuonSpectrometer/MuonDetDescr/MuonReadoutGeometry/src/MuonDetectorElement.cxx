/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 ----------------------------------------------------
 Copyright (C) 2004 by ATLAS Collaboration
 ***************************************************************************/

#include "MuonReadoutGeometry/MuonDetectorElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "GeoModelKernel/GeoPhysVol.h"

#include "GaudiKernel/MsgStream.h"

#include <cassert>

namespace MuonGM {

MuonDetectorElement::MuonDetectorElement(GeoVFullPhysVol* pv, 
                                         MuonDetectorManager* mgr,
                                         Identifier id,
                                         IdentifierHash idHash)
  : TrkDetElementBase(pv)
{
  m_muon_mgr = mgr;
  m_id = id;
  m_idhash = idHash;
  _nREinDetectorElement = 0;
}


MuonDetectorElement::~MuonDetectorElement()
{
    //    if (m_MsgStream) delete m_MsgStream;
    //  m_MsgStream=0;  
}
    
}
