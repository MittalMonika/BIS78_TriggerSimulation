/*
 Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
 */

//// Developed by Johannes Josef Junggeburth (jojungge@cern.ch)
// Local include(s):
#include "TestIsolationCloseByCorrAthenaAlg.h"

// EDM include(s):
#include "xAODEgamma/PhotonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"

#include <IsolationSelection/IIsolationCloseByCorrectionTool.h>
#include <IsolationSelection/IIsolationSelectionTool.h>
#include <IsolationSelection/IsoVariableHelper.h>
#include <IsolationSelection/TestMacroHelpers.h>



namespace CP {
    static SG::AuxElement::Decorator<char> dec_PassQuality("quality");
    static SG::AuxElement::Decorator<char> dec_PassIsol("DefaultIso");

    TestIsolationCloseByCorrAthenaAlg::TestIsolationCloseByCorrAthenaAlg(const std::string& name, ISvcLocator* svcLoc) :
                AthAlgorithm(name, svcLoc),
                m_histSvc("THistSvc", name),
                m_isoCloseByCorrTool(""),
                m_isoSelectorTool(""),
                m_tree(nullptr),
                m_ele_helper(),
                m_muo_helper(),
                m_pho_helper() {
        declareProperty("IsoCloseByCorrTool", m_isoCloseByCorrTool);
        declareProperty("IsoSelectorTool", m_isoSelectorTool);
    }

    StatusCode TestIsolationCloseByCorrAthenaAlg::initialize() {
        ATH_CHECK(m_isoSelectorTool.retrieve());
        ATH_CHECK(m_isoCloseByCorrTool.retrieve());

        m_tree = new TTree("IsoCorrTest", "Test tree for the isolaiton correction tool");

        m_ele_helper = std::unique_ptr < IsoCorrectionTestHelper > (new IsoCorrectionTestHelper(m_tree, "Electrons",m_isoSelectorTool->getElectronWPs()));
        m_muo_helper = std::unique_ptr < IsoCorrectionTestHelper > (new IsoCorrectionTestHelper(m_tree, "Muons",m_isoSelectorTool->getMuonWPs()));
        m_pho_helper = std::unique_ptr < IsoCorrectionTestHelper > (new IsoCorrectionTestHelper(m_tree, "Photons",m_isoSelectorTool->getPhotonWPs()));

        ATH_CHECK(m_histSvc->regTree("/ISOCORRECTION/IsoCorrTest", m_tree));

        return StatusCode::SUCCESS;
    }

    StatusCode TestIsolationCloseByCorrAthenaAlg::execute() {
        xAOD::ElectronContainer* Electrons = nullptr;
        xAOD::MuonContainer* Muons = nullptr;
        xAOD::PhotonContainer* Photons = nullptr;

        //Create the links to the shallow copy objects
        ATH_CHECK(CreateContainerLinks("Electrons", Electrons));
        for (const auto ielec : *Electrons) {
            //Store if the electron passes the isolation
            dec_PassIsol(*ielec) = m_isoSelectorTool->accept(*ielec);
            //Quality criteria only baseline kinematic selection
            dec_PassQuality(*ielec) = ielec->pt() > 10.e3 && fabs(ielec->eta()) < 2.47;
        }

        ATH_CHECK(CreateContainerLinks("Photons", Photons));
        for (const auto iphot : *Photons) {
            //Store if the photon passes the isolation (only needed for later comparisons)
            dec_PassIsol(*iphot) = m_isoSelectorTool->accept(*iphot);
            //Quality criteria only baseline kinematic selection
            dec_PassQuality(*iphot) = iphot->pt() > 25.e3 && fabs(iphot->eta()) < 2.35;
        }

        ATH_CHECK(CreateContainerLinks("Muons", Muons));
        for (const auto imuon : *Muons) {
            //Store if the muon passes the isolation
            dec_PassIsol(*imuon) = m_isoSelectorTool->accept(*imuon);
            //Quality criteria only baseline kinematic selection
            dec_PassQuality(*imuon) = imuon->pt() > 5.e3 && fabs(imuon->eta()) < 2.7;
        }
        //Okay everything is defined for the preselection of the algorithm. lets  pass the things  towards the IsoCorrectionTool

        if (m_isoCloseByCorrTool->getCloseByIsoCorrection(Electrons, Muons, Photons).code() == CorrectionCode::Error) {
            ATH_MSG_ERROR("Something weird happened with the tool");
            return StatusCode::FAILURE;
        }
        // The isoCorrectionTool has now corrected everything using close-by objects satisfiyng the dec_PassQuality criteria
        // The name of the decorator is set via the 'SelectionDecorator' property of the tool
        // Optionally one can also define that the tool shall only objects surviving the overlap removal without  changing the initial decorator
        // Use therefore the 'PassOverlapDecorator' property to define the decorators name
        // If you define  the 'BackupPrefix' property then the original values are stored before correction <Prefix>_<IsolationCone>
        // The final result  whether the object  passes the isolation criteria now can be stored in the 'IsolationSelectionDecorator' e.g. 'CorrectedIso'

        //Store everything in the final ntuples
        ATH_CHECK(m_ele_helper->Fill(Electrons));
        ATH_CHECK(m_muo_helper->Fill(Muons));
        ATH_CHECK(m_pho_helper->Fill(Photons));
        m_tree->Fill();
        return StatusCode::SUCCESS;
    }
}
