/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGEFMISSINGET_EFMISSINGETFROMHELPERMT_H
#define TRIGEFMISSINGET_EFMISSINGETFROMHELPERMT_H

/********************************************************************

NAME:     EFMissingETFromHelperMT.h
PACKAGE:  Trigger/TrigAlgorithms/TrigEFMissingET

AUTHORS:  Diego Casadei, Gabriel Gallardo
CREATED:  March 12, 2008

PURPOSE:  Updates TrigMissingET using TrigMissingETHelper info

 ********************************************************************/

#include "TrigEFMissingET/EFMissingETBaseTool.h"
//#include "TrigCaloEvent/TrigMissingET.h"
#include "TrigMissingEtEvent/TrigMissingET.h"
#include "TrigEFMissingET/IMissingETTool.h"


#include <vector>
//#include "TH2F.h"

/**
  $class EFMissingETFromHelper
  - Saves transient information in the persistent object,
  applying calibration only to global properties.
  - Fills component-level monitoring histograms
 **/

class EFMissingETFromHelperMT : public extends<EFMissingETBaseTool, IMissingETTool>
{
  public:

    EFMissingETFromHelperMT(const std::string& type,
                          const std::string& name,
                          const IInterface* parent);

    ~EFMissingETFromHelperMT();

    virtual StatusCode initialize();
    virtual StatusCode finalize();
    virtual StatusCode execute();

    virtual StatusCode execute(xAOD::TrigMissingET *met,
                               TrigEFMissingEtHelper *metHelper,
                               const xAOD::CaloClusterContainer *caloCluster,
                               const xAOD::JetContainer *jets,
                               const xAOD::TrackParticleContainer *track,
                               const xAOD::VertexContainer *vertex) const;
    virtual StatusCode update( xAOD::TrigMissingET *met,
           TrigEFMissingEtHelper *metHelper ) const override;
};

#endif // TRIGEFMISSINGET_EFMISSINGETFROMHELPERMT
