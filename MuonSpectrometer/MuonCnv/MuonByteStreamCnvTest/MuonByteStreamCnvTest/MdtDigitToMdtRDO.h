/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDTDIGITTOMDTRDO_H
#define MDTDIGITTOMDTRDO_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"

#include "MuonDigitContainer/MdtDigitContainer.h"
#include "MuonRDO/MdtCsmContainer.h"

#include "MuonCablingData/MuonMDT_CablingMap.h"
#include "StoreGate/ReadCondHandleKey.h"

class MdtIdHelper;

/////////////////////////////////////////////////////////////////////////////

class MdtDigitToMdtRDO : public AthAlgorithm {

 public:

  MdtDigitToMdtRDO (const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;

 private:

  StatusCode fill_MDTdata() const;
  StatusCode fillTagInfo() const;

 protected:

  const MdtIdHelper*   m_mdtIdHelper;
  bool m_BMEpresent;
  SG::WriteHandleKey<MdtCsmContainer> m_csmContainerKey{this,"OutputObjectName","MDTCSM","WriteHandleKey for Output MdtCsmContainer"};
  SG::ReadHandleKey<MdtDigitContainer> m_digitContainerKey{this,"InputObjectName","MDT_DIGITS","ReadHandleKey for Input MdtDigitContainer"};
  SG::ReadCondHandleKey<MuonMDT_CablingMap> m_readKey{this, "ReadKey", "MuonMDT_CablingMap", "Key of MuonMDT_CablingMap"};

};

#endif
