/*
 Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
 */

#include <MuonEfficiencyCorrections/KinematicSystHandler.h>
namespace CP {

    float IKinematicSystHandler::Eta(const xAOD::Muon &mu) const {
        return mu.eta();
    }
    float IKinematicSystHandler::Pt(const xAOD::Muon &mu) const {
        return mu.pt();
    }
    float IKinematicSystHandler::PtGeV(const xAOD::Muon &mu) const {
        return mu.pt() / 1.e3;
    }
    float IKinematicSystHandler::AbsEta(const xAOD::Muon &mu) const {
        return fabs(mu.eta());
    }
    
    IKinematicSystHandler::KinVariable IKinematicSystHandler::GetMuonVariableToUse(const std::string &name) {
        if (name == "pt") return &IKinematicSystHandler::Pt;
        if (name == "ptGeV") return &IKinematicSystHandler::PtGeV;
        if (name == "eta") return &IKinematicSystHandler::Eta;
        if (name == "AbsEta") return &IKinematicSystHandler::AbsEta;
        return nullptr;
    }

            
    PtKinematicSystHandler::PtKinematicSystHandler(std::unique_ptr<HistHandler> pt_flatnesss, std::unique_ptr<HistHandler> energy_loss):
             m_flatness(),
             m_loss(),
             m_SystWeight(0){
        m_flatness.swap(pt_flatnesss);
        m_loss.swap(energy_loss);
    }
   
    CorrectionCode PtKinematicSystHandler::GetKineDependent(const xAOD::Muon& mu, float& eff) const {
        int bin_flat(-1), bin_loss(-1);
        float syst = 0;
        CorrectionCode cc_flat = m_flatness->FindBin(mu, bin_flat);
        CorrectionCode cc_eloss = mu.pt() > 200.e3 ? m_loss->FindBin(mu, bin_loss) : cc_flat;
      
        float eloss_syst = bin_loss < 1 ? 1.e6 : std::fabs( m_loss->GetBinContent(bin_loss) * mu.pt()/1.0e6);
        /// We exceed the limits of the histogram
        if (cc_flat != CorrectionCode::Ok){
            /// The eloss is going to take over now
            if (cc_eloss == CorrectionCode::Ok){ 
                syst = eloss_syst;
                eff *= 1 + m_SystWeight * std::fabs(syst);
                return cc_eloss;
             } else return cc_flat; 
        } else {        
            // The eloss -systematic is valid and smaller than the error from the flatness        
            float abs_error = std::fabs( m_flatness->GetBinError(bin_flat));
            if (cc_eloss == CorrectionCode::Ok && mu.pt() > 200.e3 && (eloss_syst < abs_error || abs_error == 0 || mu.pt() > 500.e3)){
                syst = eloss_syst;
            // The flatness of the scale-factor is still more precise than the eloss. Assign this as an extra syst
            } else {            
                syst = m_flatness->GetBinContent(bin_flat); 
            }
        }       
        eff *= 1 + m_SystWeight * std::fabs(syst);
        return cc_flat;
    }
            
    void PtKinematicSystHandler::SetSystematicWeight(float syst_weight) {
        m_SystWeight = syst_weight;
    }
            
    bool PtKinematicSystHandler::initialize(){
        return m_flatness.get() != nullptr && m_loss.get() != nullptr;
    }
   
    
    PrimodialPtSystematic::PrimodialPtSystematic(std::unique_ptr<HistHandler> Handler) :
                    m_Handler(),
                    m_SystWeight(0) {
        m_Handler.swap(Handler);

    }
    CorrectionCode PrimodialPtSystematic::GetKineDependent(const xAOD::Muon &mu, float& Eff) const {
        // Account for catastrophic energy loss for  very high
        // pt's
        if (mu.pt() <= 200.e3) return CorrectionCode::Ok;

        int binsys = -1;
        CorrectionCode cc = m_Handler->FindBin(mu, binsys);
        if (cc != CorrectionCode::Ok) {
            return cc;
        }
        Eff *= (1. + m_SystWeight * fabs(m_Handler->GetBinContent(binsys)) * mu.pt() / 1.0e6);
        return CorrectionCode::Ok;
    }
    void PrimodialPtSystematic::SetSystematicWeight(float SystWeight) {
        m_SystWeight = SystWeight;
    }
    bool PrimodialPtSystematic::initialize() {
        return m_Handler.get() != nullptr;
    }
    //###############################################################
    //                  BadMuonVetoSystHandler
    //###############################################################
    std::string BadMuonVetoSystHandler::GetNextProperty(std::string &sstr) {
        //Elimnate the beginning underscores
        if (sstr.find("_") == 0) {
            sstr.substr(1, sstr.size());
        }
        size_t pos = sstr.find("_");
        std::string Property = pos != std::string::npos ? sstr.substr(0, pos) : sstr;
        if (pos != std::string::npos) {
            sstr = sstr.substr(Property.size() + 1, sstr.size());
        }
        return Property;
    }

    BadMuonVetoSystHandler::BadMuonVetoSystHandler(TDirectory* InDir) :
                    m_SystPolynomials(),
                    m_FirstVar(nullptr),
                    m_SecondVar(nullptr),
                    m_SystWeight(0.) {
        if (!InDir) {
            Error("BadMuonSysVetoHandler()", "No TDirectory is given");
            return;
        }
        TIter next(InDir->GetListOfKeys());
        while (TObject* obj = next()) {
            std::string ObjName = obj->GetName();
            float lowRange(-1.e20), highRange(1.e20);
            if (ObjName.find("TF1") == std::string::npos) {
                continue;
            }
            TF1* TF = nullptr;
            InDir->GetObject(ObjName.c_str(), TF);
            if (!TF) {
                continue;
            }
            //Elimnate the TF1_
            GetNextProperty(ObjName);
            m_FirstVar = GetMuonVariableToUse(GetNextProperty(ObjName));
            m_SecondVar = GetMuonVariableToUse(GetNextProperty(ObjName));
            std::string LowRange_str = GetNextProperty(ObjName);
            std::string HighRange_str = GetNextProperty(ObjName);
            if (!LowRange_str.empty()) {
                lowRange = atof(LowRange_str.c_str()) / pow(10, LowRange_str.size() -1);
            }
            if (!HighRange_str.empty()) {
                highRange = atof(HighRange_str.c_str()) / pow(10, LowRange_str.size() -1);
            }
            m_SystPolynomials.insert(std::pair<Ranges, std::unique_ptr<TF1>>(Ranges(lowRange, highRange), std::unique_ptr<TF1>(TF)));
        }

    }
    CorrectionCode BadMuonVetoSystHandler::GetKineDependent(const xAOD::Muon &mu, float& Eff) const {
        TF1* Poly = nullptr;
        if (m_SystWeight == 0) {
            return CorrectionCode::Ok;
        }
        // we know that Eff=(1+relative sys error), since SF==1
        float RelHighPtSys = 0.;
        if (mu.pt() >= 100.e3) {
            CorrectionCode cc = FindAppropiatePolynomial(mu, Poly);
            if (cc != CorrectionCode::Ok) {
                return cc;
            }
//            RelHighPtSys = fabs(1 - Poly->Eval((this->*m_FirstVar)(mu)));
            RelHighPtSys = Poly->Eval((this->*m_FirstVar)(mu));

        } else {
            //Apply flat 0.5% systematic
            RelHighPtSys = 0.005;
        }

        Eff = 1 + m_SystWeight * sqrt((Eff - 1) * (Eff - 1) + RelHighPtSys * RelHighPtSys);

        return CorrectionCode::Ok;
    }
    void BadMuonVetoSystHandler::SetSystematicWeight(float SystWeight) {
        m_SystWeight = SystWeight;
    }

    bool BadMuonVetoSystHandler::initialize() {
        if (m_SystPolynomials.empty()) {
            Error("BadMuonVetoSystHandler::initialize()", "No polynomials");
            return false;
        }
        if (!m_FirstVar || !m_SecondVar) {
            return false;
        }
        return true;
    }
    BadMuonVetoSystHandler::~BadMuonVetoSystHandler() {
    }

    CP::CorrectionCode BadMuonVetoSystHandler::FindAppropiatePolynomial(const xAOD::Muon& mu, TF1* &Poly) const {
        if (!m_SecondVar) {
            Error("BadMuonVetoSystHandler()", "Something went wrong with the initialization");
            return CorrectionCode::Error;
        }
        for (const auto& BinnedPoly : m_SystPolynomials) {
            if (BinnedPoly.first.first <= (this->*m_SecondVar)(mu) && (this->*m_SecondVar)(mu) < BinnedPoly.first.second) {
                Poly = BinnedPoly.second.get();
                return CorrectionCode::Ok;
            }
        }
        return CP::CorrectionCode::OutOfValidityRange;
    }

} /* namespace CP */
