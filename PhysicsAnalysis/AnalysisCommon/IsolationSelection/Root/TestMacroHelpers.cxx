/*
 Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
 */

#include <IsolationSelection/IsolationWP.h>
#include <IsolationSelection/TestMacroHelpers.h>
#include <IsolationSelection/IsoVariableHelper.h>

#include <xAODPrimitives/IsolationHelpers.h>
#include <xAODPrimitives/tools/getIsolationAccessor.h>
#include <xAODPrimitives/tools/getIsolationCorrectionAccessor.h>

namespace CP {

    std::string EraseWhiteSpaces(std::string str) {
        str.erase(std::remove(str.begin(), str.end(), '\t'), str.end());
        if (str.find(" ") == 0) return EraseWhiteSpaces(str.substr(1, str.size()));
        if (str.size() > 0 && str.find(" ") == str.size() - 1) return EraseWhiteSpaces(str.substr(0, str.size() - 1));
        return str;
    }
    //############################################################################
    //                      IsoCorrectionTestHelper
    //############################################################################
    IsoCorrectionTestHelper::IsoCorrectionTestHelper(TTree* outTree, const std::string& ContainerName, const std::vector<IsolationWP*> &WPs) :
                m_tree(outTree),
                m_init(true),
                m_pt(),
                m_eta(),
                m_phi(),
                m_e(),
                m_Q(),
                m_orig_passIso(),
                m_corr_passIso(),
                m_iso_branches(),
                m_acc_used_for_corr(SelectionAccessor(new CharAccessor("considerInCorrection"))),
                m_acc_passDefault(SelectionAccessor(new CharAccessor("defaultIso"))),
                m_acc_passCorrected(SelectionAccessor(new CharAccessor("correctedIsol"))) {

        if (!AddBranch(ContainerName + "_pt", m_pt)) m_init = false;
        if (!AddBranch(ContainerName + "_eta", m_eta)) m_init = false;
        if (!AddBranch(ContainerName + "_phi", m_phi)) m_init = false;
        if (!AddBranch(ContainerName + "_e", m_e)) m_init = false;
        if (!AddBranch(ContainerName + "_Q", m_Q)) m_init = false;

        //Retrieve the isolaiton accessors directly from the WP
        for (const auto& W : WPs) {
            for (auto& C : W->conditions()) {
                m_iso_branches.push_back(IsolationBranches(C->type(),"default"));                
            }
            //Assume only 1 WP
            break;
        }
        for (auto& branch : m_iso_branches) {
            if (!AddBranch(ContainerName + "_Orig_" + branch.Accessor->name(), branch.original_cones)) m_init = false;
            if (!AddBranch(ContainerName + "_Corr_" + branch.Accessor->name(), branch.corrected_cones)) m_init = false;
        }
        
        if (!AddBranch(ContainerName + "_OrigPassIso", m_orig_passIso)) m_init = false;
        if (!AddBranch(ContainerName + "_CorrPassIso", m_corr_passIso)) m_init = false;
    }
    StatusCode IsoCorrectionTestHelper::Fill(xAOD::IParticleContainer* Particles) {
        if (!Particles) {
            Error("IsoCorrectionTestHelper::Fill()", "No particles given");
            return StatusCode::FAILURE;
        }
        if (!m_init) {
            Error("IsoCorrectionTestHelper::Fill()", "Something went wrong during setup");
            return StatusCode::FAILURE;
        }
        //Clear the old vectors first
        m_pt.clear();
        m_eta.clear();
        m_phi.clear();
        m_e.clear();
        m_Q.clear();
        
        for (auto& branch: m_iso_branches){
            branch.original_cones.clear();
            branch.corrected_cones.clear();            
        }
      

        m_orig_passIso.clear();
        m_corr_passIso.clear();

        for (const auto object : *Particles) {
            if (!m_acc_used_for_corr->isAvailable(*object) || !m_acc_used_for_corr->operator()(*object)) continue;
            m_pt.push_back(object->pt());
            m_eta.push_back(object->eta());
            m_phi.push_back(object->phi());
            m_e.push_back(object->e());
            m_Q.push_back(Charge(object));
            for (auto& branch : m_iso_branches) {
                if (!FillIsolationBranches(object, branch.Accessor, branch.original_cones, branch.corrected_cones).isSuccess()) {
                   Error("IsoCorrectionTestHelper()", "Failed to fill isolation");
                   return StatusCode::FAILURE;
                }
            }                    
            if (!m_acc_passDefault->isAvailable(*object)) {
                Error("IsoCorrectionTestHelper()", "It has not been stored whether the particle passes the default isolation");
                return StatusCode::FAILURE;
            } else m_orig_passIso.push_back(m_acc_passDefault->operator()(*object));

            if (!m_acc_passCorrected->isAvailable(*object)) {
                Error("IsoCorrectionTestHelper()", "It has not been stored whether the particle passes the corrected isolation.");
                return StatusCode::FAILURE;
            } else m_corr_passIso.push_back(m_acc_passCorrected->operator()(*object));
        }
        return StatusCode::SUCCESS;
    }
    float IsoCorrectionTestHelper::Charge(const xAOD::IParticle* P) const {
        static FloatAccessor acc_charge("charge");
        if (!acc_charge.isAvailable(*P)) return 0;
        else return acc_charge(*P);
    }

    template<typename T> bool IsoCorrectionTestHelper::AddBranch(const std::string &Name, T &Element) {
        std::string bName = EraseWhiteSpaces(Name);
        if (m_tree->FindBranch(bName.c_str())) {
            Error("TreeHelper::AddBranch()", "The branch %s already exists in TTree %s", Name.c_str(), m_tree->GetName());
            return false;
        }
        if (m_tree->Branch(bName.c_str(), &Element) == nullptr) {
            Error("TreeHelper::AddBranch()", "Could not create the branch %s in TTree %s", Name.c_str(), m_tree->GetName());
            return false;
        }
        return true;
    }
    StatusCode IsoCorrectionTestHelper::FillIsolationBranches(const xAOD::IParticle* P, const IsoHelperPtr & Acc, std::vector<float> &Original, std::vector<float> &Corrected) {
        if (!Acc.get()) return StatusCode::SUCCESS;
        float IsoValue;
        if (Acc->getOrignalIsolation(P, IsoValue).code() != CorrectionCode::Ok) return StatusCode::FAILURE;
        Original.push_back(IsoValue);
        if (Acc->getIsolation(P, IsoValue).code() != CorrectionCode::Ok) return StatusCode::FAILURE;
        Corrected.push_back(IsoValue);
        return StatusCode::SUCCESS;
    }
    void IsoCorrectionTestHelper::DefaultIsolation(const std::string &DecorName){
        m_acc_passDefault= SelectionAccessor(new CharAccessor(DecorName));
    }
    void IsoCorrectionTestHelper::CorrectedIsolation(const std::string &DecorName){
       m_acc_passCorrected = SelectionAccessor(new CharAccessor(DecorName));
    }
    void IsoCorrectionTestHelper::BackupPreFix(const std::string &PreFix) {
        for (auto& branch : m_iso_branches){
            branch.Accessor = std::make_unique<IsoVariableHelper>(branch.Accessor->isotype(), PreFix);
        }
    }
            
         
}
