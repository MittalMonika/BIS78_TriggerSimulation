/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDTPREPDATACONTAINER_TLP1_H
#define MDTPREPDATACONTAINER_TLP1_H

//-----------------------------------------------------------------------------
// MuonPrepRawData
//-----------------------------------------------------------------------------
#include "MuonEventTPCnv/MuonPrepRawData/MuonPRD_Container_p1.h"
#include "MuonEventTPCnv/MuonPrepRawData/MdtPrepData_p1.h"
#include "TrkEventTPCnv/TrkPrepRawData/PrepRawData_p1.h"
#include "TrkEventTPCnv/TrkEventPrimitives/HepSymMatrix_p1.h"
#include "TrkEventTPCnv/TrkEventPrimitives/LocalPosition_p1.h"


// Token
#include "AthenaPoolUtilities/TPCnvTokenList_p1.h"

namespace Muon
{
   class MdtPrepDataContainer_tlp1
   {
  public:
     MdtPrepDataContainer_tlp1() {}
     TPCnvTokenList_p1		                        m_tokenList;

// This is the basic info
     std::vector< Muon::MuonPRD_Container_p1>       m_pixDCCont;
     std::vector< Muon::MdtPrepData_p1 >            m_muonPrds;
// This are the base classes
     std::vector< Trk::PrepRawData_p1>              m_prepData;
// This is info held by the base class
     std::vector< Trk::LocalPosition_p1>            m_locPos;
     std::vector< Trk::HepSymMatrix_p1>             m_errorMat;
   };
}


#endif
