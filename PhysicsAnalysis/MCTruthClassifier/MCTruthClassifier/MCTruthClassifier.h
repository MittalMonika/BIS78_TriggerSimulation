/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCTRUTHCLASSIFIER_MCTRUTHCLASSIFIER_H
#define MCTRUTHCLASSIFIER_MCTRUTHCLASSIFIER_H
/********************************************************************
NAME:     MCTruthClassifier.h 
PACKAGE:  atlasoff/PhysicsAnalysis/MCTruthClassifier
AUTHORS:  O. Fedin
CREATED:  Sep 2007
PURPOSE:            
Updated:  
********************************************************************/

// INCLUDE HEADER FILES:
#include "AsgTools/AsgTool.h"
#include "MCTruthClassifier/MCTruthClassifierDefs.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"

// EDM includes

#include "xAODTruth/TruthParticleContainerFwd.h"
#include "xAODTruth/TruthVertexFwd.h"


#ifndef XAOD_ANALYSIS
#include "GaudiKernel/ToolHandle.h"
#include "GeneratorObjects/xAODTruthParticleLink.h"
#include "ParticlesInConeTools/ITruthParticlesInConeTool.h"
namespace HepMC {
 class GenParticle;
}

namespace Trk {
  class IParticleCaloExtensionTool;
}
#endif


class MCTruthClassifier : virtual public IMCTruthClassifier , public asg::AsgTool {

  ASG_TOOL_CLASS(MCTruthClassifier, IMCTruthClassifier)
 public:

  // constructor 
  MCTruthClassifier(const std::string& type);
  // destructor 
  virtual ~MCTruthClassifier();

  // Gaudi algorithm hooks
  virtual StatusCode initialize() override;
  virtual StatusCode finalize()
#ifndef XAOD_STANDALONE
  override
#endif // not XAOD_STANDALONE
  ;

  //Old EDM  
#ifndef XAOD_ANALYSIS
  virtual
  std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>  particleTruthClassifier(const HepMC::GenParticle *, Info* info = nullptr) const override; 
  bool compareTruthParticles(const HepMC::GenParticle *genPart, const xAOD::TruthParticle *truthPart) const;
#endif
  virtual
  std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> particleTruthClassifier(const xAOD::TruthParticle *, Info* info = nullptr) const override;
  virtual
  std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> particleTruthClassifier(const xAOD::TrackParticle *, Info* info = nullptr) const override;
  virtual
  std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> particleTruthClassifier(const xAOD::Electron*, Info* info = nullptr) const override;
  virtual
  std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> particleTruthClassifier(const xAOD::Photon*, Info* info = nullptr ) const override;
  virtual
  std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> particleTruthClassifier(const xAOD::Muon*, Info* info = nullptr) const override;
  virtual
  std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> particleTruthClassifier(const xAOD::CaloCluster*, Info* info = nullptr) const override;
  virtual
  std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> particleTruthClassifier(const xAOD::Jet*, bool DR, Info* info = nullptr) const override;

  virtual
  const xAOD::TruthParticle* getGenPart(const xAOD::TrackParticle *,
                                        Info* info = nullptr) const override;
  const xAOD::TruthParticle* getMother(const xAOD::TruthParticle*) const;

  virtual
  std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
    checkOrigOfBkgElec(const xAOD::TruthParticle* thePart,
                       Info* info = nullptr) const override;

  virtual
  const xAOD::TruthParticle* isHadronFromB(const xAOD::TruthParticle *) const override;

 private:

   static float detEta(float x, float y ) {return fabs(x-y);}
   static float detPhi(float , float );
   static float rCone (float x, float y ){return sqrt(x*x + y*y);}
   //
   MCTruthPartClassifier::ParticleType    defTypeOfElectron(MCTruthPartClassifier::ParticleOrigin, bool isPrompt) const;
   MCTruthPartClassifier::ParticleOrigin  defOrigOfElectron(const xAOD::TruthParticleContainer* xTruthParticleContainer ,const xAOD::TruthParticle*,
                                                            bool& isPrompt,
                                                            Info* info) const;
   MCTruthPartClassifier::ParticleOutCome defOutComeOfElectron(const xAOD::TruthParticle*) const;
   //
   MCTruthPartClassifier::ParticleType    defTypeOfMuon(MCTruthPartClassifier::ParticleOrigin, bool isPrompt) const;
   MCTruthPartClassifier::ParticleOrigin  defOrigOfMuon(const xAOD::TruthParticleContainer* m_xTruthParticleContainer ,const xAOD::TruthParticle*, bool& isPrompt, Info* info) const;
   MCTruthPartClassifier::ParticleOutCome defOutComeOfMuon(const xAOD::TruthParticle*) const;
   //
   static MCTruthPartClassifier::ParticleType    defTypeOfTau(MCTruthPartClassifier::ParticleOrigin);
   MCTruthPartClassifier::ParticleOrigin  defOrigOfTau(const xAOD::TruthParticleContainer* m_xTruthParticleContainer ,const xAOD::TruthParticle*, int motherPDG, Info* info) const;
   MCTruthPartClassifier::ParticleOutCome defOutComeOfTau(const xAOD::TruthParticle*, Info* info) const;
   //
   MCTruthPartClassifier::ParticleType    defTypeOfPhoton(MCTruthPartClassifier::ParticleOrigin) const;
   MCTruthPartClassifier::ParticleOrigin  defOrigOfPhoton(const xAOD::TruthParticleContainer* m_xTruthParticleContainer ,const xAOD::TruthParticle*, bool& isPrompt, Info* info) const;
   MCTruthPartClassifier::ParticleOutCome defOutComeOfPhoton(const xAOD::TruthParticle*) const;
   //
   MCTruthPartClassifier::ParticleOrigin  defOrigOfNeutrino(const xAOD::TruthParticleContainer* m_xTruthParticleContainer ,const xAOD::TruthParticle*, bool& isPrompt, Info* info) const;
   //
   MCTruthPartClassifier::ParticleOrigin  defHadronType(long);
   static bool isHadron(const xAOD::TruthParticle*);
   static MCTruthPartClassifier::ParticleType    defTypeOfHadron(long);
   static MCTruthPartClassifier::ParticleOrigin  convHadronTypeToOrig(MCTruthPartClassifier::ParticleType pType,
                                                                      int motherPDG);
   //
   const xAOD::TruthVertex* findEndVert(const xAOD::TruthParticle*) const;
   static bool  isHardScatVrtx(const xAOD::TruthVertex* );
   //
   std::vector<const xAOD::TruthParticle*> findFinalStatePart(const xAOD::TruthVertex*) const;
   //
   static double partCharge(const xAOD::TruthParticle*);
#ifndef XAOD_ANALYSIS
   bool genPartToCalo(const xAOD::CaloCluster* , const xAOD::TruthParticle* , bool, double&, bool&, Cache * ) const;
   const xAOD::TruthParticle* egammaClusMatch(const xAOD::CaloCluster*, bool,
                                              Info* info) const;
#endif
   //
   void findAllJetMothers(const xAOD::TruthParticle*,std::set<const xAOD::TruthParticle*>&) const;
   void findParticleDaughters(const xAOD::TruthParticle*, std::set<const xAOD::TruthParticle*>&) const;
   double fracParticleInJet(const xAOD::TruthParticle*, const xAOD::Jet*, bool DR, bool nparts) const;
   void findJetConstituents(const xAOD::Jet*, std::set<const xAOD::TruthParticle*>& constituents, bool DR) const;
   static double deltaR(const xAOD::TruthParticle& v1, const xAOD::Jet & v2);
   MCTruthPartClassifier::ParticleOrigin defJetOrig(std::set<const xAOD::TruthParticle*>) const;
   //
  
   /** Return true if genParticle and truthParticle have the same pdgId, barcode and status **/
   static const xAOD::TruthParticle* barcode_to_particle(const xAOD::TruthParticleContainer*,int );
  
//------------------------------------------------------------------------
//      configurable data members
//------------------------------------------------------------------------

   
#ifndef XAOD_ANALYSIS
   ToolHandle< Trk::IParticleCaloExtensionTool >  m_caloExtensionTool;
   ToolHandle<xAOD::ITruthParticlesInConeTool> m_truthInConeTool;
   SG::ReadHandleKey<xAODTruthParticleLinkVector> m_truthLinkVecReadHandleKey{this,"xAODTruthLinkVector","xAODTruthLinks","ReadHandleKey for xAODTruthParticleLinkVector"};
   float m_FwdElectronTruthExtrEtaCut;
   float m_FwdElectronTruthExtrEtaWindowCut;
   float m_partExtrConeEta;
   float m_partExtrConePhi;
   bool  m_useCaching;
   float m_phtClasConePhi;
   float m_phtClasConeEta;
   float m_phtdRtoTrCut;
   float m_fwrdEledRtoTrCut; 
   bool  m_ROICone;
#endif

  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleContainerKey{this,"xAODTruthParticleContainerName","TruthParticles","ReadHandleKey for xAOD::TruthParticleContainer"};
   float m_deltaRMatchCut;
   float m_deltaPhiMatchCut;
   int   m_NumOfSiHitsCut;
   float m_pTChargePartCut;
   float m_pTNeutralPartCut;
   long  m_barcodeShift;
   long  m_barcodeG4Shift;
   float m_jetPartDRMatch;
   bool  m_inclG4part;
   bool  m_inclEgammaPhoton;
   bool  m_inclEgammaFwrdEle;
   bool  m_LQpatch;

};
#endif  // MCTRUTHCLASSIFIER_MCTRUTHCLASSIFIER_H 
