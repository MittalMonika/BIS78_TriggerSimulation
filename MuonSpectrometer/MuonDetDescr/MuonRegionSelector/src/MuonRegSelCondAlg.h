/** emacs: this is -*- c++ -*- **/
/**
 **   @file    MuonRegSelCondAlg.h        
 **                   
 **   @author  sutt
 **   @date    Tue  4 Feb 2020 15:25:00 CET
 **
 **   Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
 **/
 
#ifndef MuonRegSelCondAlg_h
#define MuonRegSelCondAlg_h

#include "GaudiKernel/ISvcLocator.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "GaudiKernel/ToolHandle.h"

#include "MuonCablingData/MuonMDT_CablingMap.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

#include "IRegionSelector/RegSelLUTCondData.h"

#include <string>


/////////////////////////////////////////////////////////////////////////////

class MuonRegSelCondAlg : public AthReentrantAlgorithm {

public:

  MuonRegSelCondAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual StatusCode  initialize() override;
  virtual StatusCode  execute (const EventContext& ctx) const override;

  virtual std::unique_ptr<RegSelSiLUT> createTable( const MuonMDT_CablingMap* cabling ) const = 0;   

 private:

  std::string m_managerName;
  bool        m_printTable;
 
  SG::ReadCondHandleKey<MuonMDT_CablingMap> m_cablingKey
    { this, "Cabling", "MuonMDT_CablingMap", "Key of output MDT cabling map" };

  /// Output conditions object
  SG::WriteCondHandleKey<RegSelLUTCondData> m_tableKey  
    { this, "RegSelLUT", "RegSelLUTCondData", "Region Selector lookup table" };



};

#endif // MuonRegSelCondAlg_h
