/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
#ifndef JPSIPLUSDSCASCADE_H
#define JPSIPLUSDSCASCADE_H
//*********************
// JpsiPlusDsCascade header file
//
// Eva Bouhova <e.bouhova@cern.ch>
// Adam Barton <abarton@cern.ch>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "HepPDT/ParticleDataTable.hh"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "JpsiUpsilonTools/PrimaryVertexRefitter.h"
#include <vector>


namespace Trk {
    class IVertexFitter;
    class TrkVKalVrtFitter;
    class IVertexCascadeFitter;
    class VxCascadeInfo;
    class V0Tools;
    class ParticleDataTable;
}

namespace DerivationFramework {
    class CascadeTools;
}
class IBeamCondSvc;

namespace DerivationFramework {

    static const InterfaceID IID_JpsiPlusDsCascade("JpsiPlusDsCascade", 1, 0);

    class JpsiPlusDsCascade : virtual public AthAlgTool, public IAugmentationTool
    {
      public:
        static const InterfaceID& interfaceID() { return IID_JpsiPlusDsCascade;}
        JpsiPlusDsCascade(const std::string& t, const std::string& n, const IInterface*  p);
        ~JpsiPlusDsCascade();
        virtual StatusCode initialize() override;
        StatusCode performSearch(std::vector<Trk::VxCascadeInfo*> *cascadeinfoContainer ) const;
        virtual StatusCode addBranches() const override;

      private:
        std::string m_vertexContainerKey;
        std::string m_vertexDxContainerKey;
        std::vector<std::string> m_cascadeOutputsKeys;

        std::string m_VxPrimaryCandidateName;   //!< Name of primary vertex container

        double m_jpsiMassLower;
        double m_jpsiMassUpper;
        double m_DxMassLower;
        double m_DxMassUpper;
        double m_MassLower;
        double m_MassUpper;

        const HepPDT::ParticleDataTable* m_particleDataTable;
        double m_mass_muon;
        double m_mass_pion;
        double m_mass_jpsi;
        double m_mass_kaon;
        double m_mass_Ds;
        double m_mass_Dp;
        double m_mass_Bc;
        int    m_Dx_pid;
        bool   m_constrDx;
        bool   m_constrJpsi;

        ServiceHandle<IBeamCondSvc>                      m_beamSpotSvc;
        ToolHandle < Trk::TrkVKalVrtFitter >             m_iVertexFitter;
        ToolHandle < Analysis::PrimaryVertexRefitter >   m_pvRefitter;
        ToolHandle < Trk::V0Tools >                      m_V0Tools;
        ToolHandle < DerivationFramework::CascadeTools > m_CascadeTools;

        bool        m_refitPV;
        std::string m_refPVContainerName;
        std::string m_hypoName;               //!< name of the mass hypothesis. E.g. Jpis, Upsi, etc. Will be used as a prefix for decorations
        //This parameter will allow us to optimize the number of PVs under consideration as the probability
        //of a useful primary vertex drops significantly the higher you go
        int         m_PV_max;
        int         m_DoVertexType;
        size_t      m_PV_minNTracks;
        std::vector<double> m_trkMasses;      //!< track mass hypotheses

        double getParticleMass(int particlecode) const;
    };
}

#endif
