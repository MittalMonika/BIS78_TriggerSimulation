///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// METMaker.h
// Header file for class METMaker
// Author: T.J.Khoo<khoo@cern.ch>
///////////////////////////////////////////////////////////////////
#ifndef METUTILITIES_MET_METMAKER_H
#define METUTILITIES_MET_METMAKER_H 1

// STL includes
#include <string>

// FrameWork includes
#include "AsgTools/ToolHandle.h"
#include "AsgTools/AsgTool.h"

// METInterface includes
#include "METInterface/IMETMaker.h"

// EDM includes
#include "xAODJet/JetContainer.h"

// Forward declaration

namespace met {

  // typedefs
  typedef ElementLink<xAOD::IParticleContainer> obj_link_t;

  class METMaker
  : public asg::AsgTool,
  virtual public IMETMaker

  {
    // This macro defines the constructor with the interface declaration
    ASG_TOOL_CLASS(METMaker, IMETMaker)

    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
  public:

    // Copy constructor:

    /// Constructor with parameters:
    METMaker(const std::string& name);

    /// Destructor:
    virtual ~METMaker();

    // Athena algtool's Hooks
    StatusCode  initialize();
    StatusCode  finalize();

    StatusCode rebuildMET(const std::string& metKey,
                          xAOD::Type::ObjectType metType,
                          xAOD::MissingETContainer* metCont,
                          const xAOD::IParticleContainer* collection,
                          const xAOD::MissingETAssociationMap* map,
                          std::vector<const xAOD::IParticle*>& uniques,
                          MissingETBase::UsageHandler::Policy objScale=MissingETBase::UsageHandler::PhysicsObject);
    //
    StatusCode rebuildMET(const std::string& metKey,
                          xAOD::Type::ObjectType metType,
                          xAOD::MissingETContainer* metCont,
                          const xAOD::IParticleContainer* collection,
                          const xAOD::MissingETAssociationMap* map,
                          MissingETBase::UsageHandler::Policy objScale=MissingETBase::UsageHandler::PhysicsObject);
    //
    StatusCode rebuildMET(xAOD::MissingET* met,
                          const xAOD::IParticleContainer* collection,
                          const xAOD::MissingETAssociationMap* map,
                          MissingETBase::UsageHandler::Policy objScale=MissingETBase::UsageHandler::PhysicsObject);
    //
    StatusCode rebuildMET(xAOD::MissingET* met,
                          const xAOD::IParticleContainer* collection,
                          const xAOD::MissingETAssociationMap* map,
                          std::vector<const xAOD::IParticle*>& uniques,
                          MissingETBase::UsageHandler::Policy objScale=MissingETBase::UsageHandler::PhysicsObject);
    //
    StatusCode rebuildMET(xAOD::MissingET* met,
                          const xAOD::IParticleContainer* collection,
                          const xAOD::MissingETAssociationMap* map,
                          std::vector<const xAOD::IParticle*>& uniques,
                          MissingETBase::UsageHandler::Policy p,
                          bool removeOverlap,
                          MissingETBase::UsageHandler::Policy objScale=MissingETBase::UsageHandler::PhysicsObject);

    StatusCode rebuildJetMET(const std::string& metJetKey,
                             const std::string& softClusKey,
                             const std::string& softTrkKey,
                             xAOD::MissingETContainer* metCont,
                             const xAOD::JetContainer* jets,
                             const xAOD::MissingETContainer* metCoreCont,
                             const xAOD::MissingETAssociationMap* map,
                             bool doJetJVT);
    StatusCode rebuildJetMET(const std::string& metJetKey,
                             const std::string& softClusKey,
                             const std::string& softTrkKey,
                             xAOD::MissingETContainer* metCont,
                             const xAOD::JetContainer* jets,
                             const xAOD::MissingETContainer* metCoreCont,
                             const xAOD::MissingETAssociationMap* map,
                             bool doJetJVT,
                             std::vector<const xAOD::IParticle*>& uniques);
    //
    StatusCode rebuildJetMET(const std::string& metJetKey,
                             const std::string& metSoftKey,
                             xAOD::MissingETContainer* metCont,
                             const xAOD::JetContainer* jets,
                             const xAOD::MissingETContainer* metCoreCont,
                             const xAOD::MissingETAssociationMap* map,
                             bool doJetJVT);
    StatusCode rebuildJetMET(const std::string& metJetKey,
                             const std::string& metSoftKey,
                             xAOD::MissingETContainer* metCont,
                             const xAOD::JetContainer* jets,
                             const xAOD::MissingETContainer* metCoreCont,
                             const xAOD::MissingETAssociationMap* map,
                             bool doJetJVT,
                             std::vector<const xAOD::IParticle*>& uniques);
    //
    StatusCode rebuildJetMET(xAOD::MissingET* metJet,
                             const xAOD::JetContainer* jets,
                             const xAOD::MissingETAssociationMap* map,
                             xAOD::MissingET* metSoftClus,
                             const xAOD::MissingET* coreSoftClus,
                             xAOD::MissingET* metSoftTrk,
                             const xAOD::MissingET* coreSoftTrk,
                             bool doJetJVT,
                             std::vector<const xAOD::IParticle*>& uniques,
                             bool tracksForHardJets = 0);

    StatusCode rebuildTrackMET(const std::string& metJetKey,
                             const std::string& softTrkKey,
                             xAOD::MissingETContainer* metCont,
                             const xAOD::JetContainer* jets,
                             const xAOD::MissingETContainer* metCoreCont,
                             const xAOD::MissingETAssociationMap* map,
                             bool doJetJVT);
    StatusCode rebuildTrackMET(const std::string& metJetKey,
                             const std::string& softTrkKey,
                             xAOD::MissingETContainer* metCont,
                             const xAOD::JetContainer* jets,
                             const xAOD::MissingETContainer* metCoreCont,
                             const xAOD::MissingETAssociationMap* map,
                             bool doJetJVT,
                             std::vector<const xAOD::IParticle*>& uniques);
    StatusCode rebuildTrackMET(xAOD::MissingET* metJet,
                             const xAOD::JetContainer* jets,
                             const xAOD::MissingETAssociationMap* map,
                             xAOD::MissingET* metSoftTrk,
                             const xAOD::MissingET* coreSoftTrk,
                             bool doJetJVT,
                             std::vector<const xAOD::IParticle*>& uniques);

    StatusCode markInvisible(const xAOD::IParticleContainer* collection,
			     const xAOD::MissingETAssociationMap* map,
			     MissingETBase::UsageHandler::Policy p=MissingETBase::UsageHandler::TrackCluster);

    StatusCode buildMETSum(const std::string& totalName,
                           xAOD::MissingETContainer* metCont,
                           MissingETBase::Types::bitmask_t softTermsSource=0);

    ///////////////////////////////////////////////////////////////////
    // Const methods:
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    // Non-const methods:
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    // Private data:
    ///////////////////////////////////////////////////////////////////
  private:

    StatusCode fillMET(xAOD::MissingET *& met,
		       xAOD::MissingETContainer * metCont,
		       const std::string& metKey,
		       const MissingETBase::Types::bitmask_t metSource);

    // std::string m_pvcoll;

    bool m_jetCorrectPhi;

    double m_jetMinPt;
    double m_jetMinAbsJvt;
    double m_jetMinEfrac;
    double m_jetMinWeightedPt;
    std::string m_jetConstitScaleMom;
    std::string m_jetJvtMomentName;

    bool m_doPFlow;
    bool m_doSoftTruth;
    bool m_doConstJet;

    bool m_muEloss;
    bool m_muIsolEloss;

    // Set up accessors to original object links in case of corrected copy containers
    SG::AuxElement::ConstAccessor<obj_link_t>  m_objLinkAcc;

    SG::AuxElement::Decorator<char>  m_jetUsedDec;
    /// Default constructor:
    METMaker();

  };

} //> end namespace met
#endif //> !METUTILITIES_MET_METMAKER_H
