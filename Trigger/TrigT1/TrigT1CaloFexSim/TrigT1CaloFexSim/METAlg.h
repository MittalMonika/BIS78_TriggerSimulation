/*
 *   Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloIdentifier/GTower_ID.h"
#include "CaloEvent/CaloCellContainer.h"
#include "xAODTrigL1Calo/JGTower.h"
#include "xAODTrigL1Calo/JGTowerContainer.h"
#include "xAODTrigL1Calo/JGTowerAuxContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "JetCalibTools/IJetCalibrationTool.h"
#include "JetInterface/IJetUpdateJvt.h"
#include "TrigAnalysisInterfaces/IBunchCrossingTool.h"
#include "Identifier/IdentifierHash.h"
#include "TH1.h"
#include "TH2.h"

class METAlg{

 public:
  struct MET{
    float phi;
    float et;

  };


  static StatusCode BuildMET(const xAOD::JGTowerContainer*towers,METAlg::MET* met,std::vector<float> noise);

};
