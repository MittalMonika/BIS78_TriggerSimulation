/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUREC_TAUPROCESSORALG_H
#define TAUREC_TAUPROCESSORALG_H

#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "tauRecTools/ITauToolExecBase.h"
#include "tauRecTools/ITauToolBase.h"

#include "tauRecTools/TauEventData.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "xAODPFlow/PFOContainer.h"
#include "xAODPFlow/PFOAuxContainer.h"

#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"


/**
 * @brief       Main class for tau candidate processing.
 */

class ICaloCellMakerTool;

class TauProcessorAlg: public AthAlgorithm
{
    public:
        //-----------------------------------------------------------------
        // Contructor and destructor
        //-----------------------------------------------------------------
        TauProcessorAlg( const std::string &name, ISvcLocator *pSvcLocator );
        ~TauProcessorAlg();

        //-----------------------------------------------------------------
        // Gaudi algorithm hooks
        //-----------------------------------------------------------------
        virtual StatusCode initialize();
        virtual StatusCode execute();
        virtual StatusCode finalize();

    private:
       
	void setEmptyTauTrack( xAOD::TauJet* &tauJet,
			       xAOD::TauTrackContainer* &tauTrackCont);				 

	ToolHandleArray<ITauToolBase>  m_tools;

	double m_maxEta; //!< only build taus with eta_seed < m_maxeta
	double m_minPt;  //!< only build taus with pt_seed > m_minpt

	bool m_doCreateTauContainers;

        //ToolHandleArray<ITauToolExecBase>  m_tools;
	TauEventData m_data;

	/** @brief tool handles */
	ToolHandle<ICaloCellMakerTool> m_cellMakerTool;

	SG::ReadHandleKey<xAOD::JetContainer> m_jetInputContainer{this,"Key_jetInputContainer","AntiKt4LCTopoJets","input jet key"};
	SG::WriteHandleKey<xAOD::TauJetContainer> m_tauOutputContainer{this,"Key_tauOutputContainer","tmp_TauJets","output tau data key"};
	SG::WriteHandleKey<xAOD::TauTrackContainer> m_tauTrackOutputContainer{this,"Key_tauTrackOutputContainer","TauTracks","output tau tracks data key"};
	SG::WriteHandleKey<xAOD::CaloClusterContainer> m_tauShotClusOutputContainer{this,"Key_tauShotClusOutputContainer", "TauShotClusters", "tau shot clusters out key"};
	SG::WriteHandleKey<xAOD::PFOContainer> m_tauShotPFOOutputContainer{this,"Key_tauShotPFOOutputContainer", "TauShotParticleFlowObjects", "tau pfo out key"};
	SG::WriteHandleKey<CaloCellContainer> m_tauPi0CellOutputContainer{this,"Key_tauPi0CellOutputContainer","TauCommonPi0Cells","output calo cell key"};

	
};

#endif // TAUREC_TAUPROCESSORALG_H
