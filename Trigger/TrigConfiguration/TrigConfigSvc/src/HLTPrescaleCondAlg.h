/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/ 

#ifndef TRIGCONFIGSVC__HLTPRESCALECONDALG
#define TRIGCONFIGSVC__HLTPRESCALECONDALG

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "TrigConfData/HLTPrescalesSet.h"

#include "AthenaPoolUtilities/AthenaAttributeList.h"

namespace TrigConf {

   /**
    * @brief Condition algorithm to provide the L1 trigger menu
    *
    * The algorithm will provide the menu configuration through a conditions handle using \c TrigConf::L1Menu to present the data
    *
    * It is able to load the configuration in the following 3 ways
    * * File-based menu description: for most test environments: for test jobs with no database involved
    * * DB-based menu description where the DB keys are provided as algorithm job-properties: for reprocessing grid jobs with new menus
    * * DB-based menu description where the DB keys are taken from COOL: for offline reconstruction jobs
    */
   class HLTPrescaleCondAlg : public AthReentrantAlgorithm {
   public:
      HLTPrescaleCondAlg(const std::string& name, ISvcLocator* pSvcLocator);
      virtual ~HLTPrescaleCondAlg() = default;

      virtual StatusCode initialize() override;
      virtual StatusCode execute(const EventContext& ctx) const override;

   private:

      // input key to the HLT Prescale Key folder
      SG::ReadCondHandleKey<AthenaAttributeList> m_pskFolderInputKey{ this, "PSKFolder", "/TRIGGER/HLT/PrescaleKey", "SG Key of AthenaAttributeList containing hlt psk"};

      // output key to store the HLTPrescalesSet
      SG::WriteCondHandleKey<TrigConf::HLTPrescalesSet> m_hltPrescaleSetOutputKey{ this, "HLTPrescales", "HLTPrescales", "HLT prescales"};

      // properties
      Gaudi::Property< std::string > m_configSource { this, "Source", "FILE", "Configuration source, can be 'FILE', 'DB', or 'COOL'" };
      Gaudi::Property< std::string > m_dbConnection { this, "TriggerDB", "TRIGGERDB", "DB connection alias" };
      Gaudi::Property< unsigned int > m_psk { this, "HLTPsk", 0, "HLT prescale key" };
      Gaudi::Property< std::string > m_filename { this, "Filename", "", "HLT prescale json file" };

   };

}
#endif
