/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUREC_TAUPI0CREATEROI_H
#define	TAUREC_TAUPI0CREATEROI_H

#include <string>
#include <vector>

#include "GaudiKernel/ToolHandle.h"
#include "tauRecTools/TauRecToolBase.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "xAODTau/TauJet.h"

class ICaloCellMakerTool;

/**
 * @brief Create ROIs for the Pi0 finder.
 * 
 * @author Will Davey <will.davey@cern.ch> 
 * @author Benedict Winter <benedict.tobias.winter@cern.ch> 
 * @author Stephanie Yuen <stephanie.yuen@cern.ch>
 */

class TauPi0CreateROI : public TauRecToolBase {
public:
    TauPi0CreateROI(const std::string& name);
    ASG_TOOL_CLASS2(TauPi0CreateROI, TauRecToolBase, ITauToolBase);
    virtual ~TauPi0CreateROI();

    virtual StatusCode initialize();
    virtual StatusCode eventInitialize();
    virtual StatusCode execute(xAOD::TauJet& pTau);
    virtual StatusCode executeCaloClus(xAOD::TauJet&, xAOD::CaloClusterContainer&) { return StatusCode::SUCCESS; }
    virtual StatusCode eventFinalize();
    virtual StatusCode finalize();

    virtual void cleanup(xAOD::TauJet* ) { }
    virtual void print() const { }

private:

    /** @brief store cell in output container */ 
    void storeCell(const CaloCell* /* cell*/);

    /** @brief tool handles */
    ToolHandle<ICaloCellMakerTool> m_cellMakerTool;

    /** @brief calo cell navigation */
    const CaloDetDescrManager* m_calo_dd_man;
    const CaloCell_ID* m_calo_id;
  
    /** @brief output cell container*/ 
    CaloCellContainer *m_pPi0CellContainer;

    /** @brief hash map in order to keep track, which cells have been added to output cell container*/
    std::vector<CaloCell*> m_addedCellsMap;

    SG::ReadHandleKey<CaloCellContainer> m_caloCellInputContainer{this,"Key_caloCellInputContainer", "AllCalo", "input vertex container key"};
    SG::WriteHandleKey<CaloCellContainer> m_tauCaloOutputContainer{this,"Key_tauCaloOutputContainer","TauCommonPi0Cells","output calo cell key"};

};

#endif	/* TAUPI0CREATEROI_H */

