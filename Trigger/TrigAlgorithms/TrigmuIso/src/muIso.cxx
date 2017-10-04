/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// ********************************************************************
//
// NAME:     muIso.cxx
// PACKAGE:  Trigger/TrigAlgorithms/TrigmuIso
//
// AUTHOR:   S.Giagu
//
// PURPOSE:  LVL2 Muon Isolation FEX Algorithm: V6.0
// ********************************************************************
#include <iostream>
#include <math.h>

#include "TrigConfHLTData/HLTTriggerElement.h"

#include "TrigmuIso/muIso.h"
#include "TrigmuIso/muIsoUtil.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/StatusCode.h"

#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/DataHandle.h"

#include "TrigTimeAlgs/TrigTimer.h"
#include "AthenaKernel/Timeout.h"

#include "xAODEventInfo/EventInfo.h"

#include "xAODTrigMuon/L2IsoMuonAuxContainer.h"
#include "xAODTrigMuon/L2CombinedMuonContainer.h"
#include "xAODTrigMuon/L2CombinedMuonAuxContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "TrigT1Interfaces/RecMuonRoI.h"

#include "CLHEP/Units/SystemOfUnits.h"

class ISvcLocator;

muIso::muIso(const std::string & name, ISvcLocator* pSvcLocator):
   HLT::FexAlgo(name, pSvcLocator)
{

   // ID trk xAOD collection to use
   declareProperty("IDalgo", m_ID_algo_to_use = "InDetTrigTrackingxAODCnv_Muon_FTF");

   declareProperty("RID",                    m_RID = 0.4);
   //ID tracks quality parameters
   declareProperty("MinPtTRK",          m_PtMinTrk = 1.0);
   declareProperty("MaxAbsEtaTRK",     m_EtaMaxTrk = 2.5);
   declareProperty("MaxDzetaIDMuon",     m_DzetaMax = 15.0);

   // Isolation cuts used for monitoring
   declareProperty("MonitoringGlobalIsoCut_ID",  m_GlobIsoCut_ID  = 0.2);

   //Monitored quantities
   declareMonitoredVariable("ErrorFlagMI",        m_ErrorFlagMI);
   declareMonitoredVariable("PtFL",               m_ptFL);
   declareMonitoredVariable("Sumpt"             , m_Sumpt);
   declareMonitoredVariable("TEMuPt"            , m_MuPt);
   declareMonitoredVariable("TEMuEta"           , m_MuEta);
   declareMonitoredVariable("TEMuPhi"           , m_MuPhi);
   declareMonitoredVariable("NTRK"              , m_NTRK);
   declareMonitoredVariable("IDiso"             , m_IDiso);
   declareMonitoredVariable("IsIsolated"        , m_isIsolated);

}


muIso::~muIso()
{
}

HLT::ErrorCode muIso::hltInitialize()
{

  ATH_MSG_DEBUG( "on initialize()"  );

  ATH_MSG_INFO( "Initializing " << name() << " - package version " << PACKAGE_VERSION  );

   m_pStoreGate = store();
   // Timer Service
   m_pTimerService = 0;

   ATH_MSG_DEBUG( "Initialization completed successfully" );

   return HLT::OK;
}

HLT::ErrorCode muIso::hltFinalize()
{

   ATH_MSG_DEBUG( "in finalize()"  );

   return HLT::OK;
}

HLT::ErrorCode muIso::hltBeginRun()
{

   ATH_MSG_INFO( "At BeginRun of " << name() << " - package version "
                 << PACKAGE_VERSION  );

   return HLT::OK;
}

HLT::ErrorCode muIso::hltExecute(const HLT::TriggerElement* inputTE, HLT::TriggerElement* outputTE)
{

   ATH_MSG_DEBUG( "in execute()"  );

   //Initalize Monitored variables
   m_ErrorFlagMI = 0;
   m_Sumpt       = -9999.;
   m_MuPt        = -9999.;
   m_MuEta       = -9999.;
   m_MuPhi       = -9999.;
   m_NTRK        = -9999;
   m_ptFL        = -9999.;
   m_IDiso       = -9999.;
   m_isIsolated  = -1.0;

   // Retrieve store
   m_pStoreGate = store();

   const xAOD::EventInfo* pEvent;
   StatusCode sc = m_pStoreGate->retrieve(pEvent);
   if (sc.isFailure()) {
      m_ErrorFlagMI = 1;
      ATH_MSG_ERROR( "Could not find xAOD::EventInfo object"  );
      return HLT::ErrorCode(HLT::Action::ABORT_CHAIN, HLT::Reason::NAV_ERROR);
   }

   auto current_run_id = pEvent->runNumber();
   auto current_event_id = pEvent->eventNumber();
   auto current_bcg_id = pEvent->bcid();

   ATH_MSG_DEBUG( " ---> Run Number       : " << current_run_id  );
   ATH_MSG_DEBUG( " ---> Event Number     : " << std::hex << current_event_id << std::dec  );
   ATH_MSG_DEBUG( " ---> Bunch Crossing ID: " << std::hex << current_bcg_id << std::dec  );

   ATH_MSG_DEBUG( "Configured to fex ID:   "  );
   ATH_MSG_DEBUG( "R ID:                   " << m_RID            );
   ATH_MSG_DEBUG( "PtMin ID:               " << m_PtMinTrk       );
   ATH_MSG_DEBUG( "AbsEtaMax ID:           " << m_EtaMaxTrk      );
   ATH_MSG_DEBUG( "AbsDeltaZeta Max ID:    " << m_DzetaMax       );

   // Some debug output:
   ATH_MSG_DEBUG( "inputTE->ID():  " << inputTE->getId()  );
   ATH_MSG_DEBUG( "outputTE->ID(): " << outputTE->getId()  );

   // Start Trigger Element Processing


   // Get input for seeding
   xAOD::L2CombinedMuonContainer* muonColl = 0;
   const xAOD::L2CombinedMuonContainer* const_muonColl(0);
   const xAOD::L2CombinedMuon* muonCB(0);

   HLT::ErrorCode status = getFeature(outputTE, const_muonColl);
   muonColl = const_cast<xAOD::L2CombinedMuonContainer*>(const_muonColl);
   if (status != HLT::OK || ! muonColl) {
      ATH_MSG_ERROR( " L2CombinedMuonContainer not found --> ABORT"  );
      return HLT::ErrorCode(HLT::Action::ABORT_CHAIN, HLT::Reason::MISSING_FEATURE);
   }

   xAOD::L2CombinedMuonContainer::iterator muonCB_it(muonColl->begin());
   muonCB = *muonCB_it;

   // Create Iso muon
   xAOD::L2IsoMuonContainer *muonISColl = new xAOD::L2IsoMuonContainer();
   xAOD::L2IsoMuonAuxContainer caux;
   muonISColl->setStore(&caux);

   xAOD::L2IsoMuon* muonIS = new xAOD::L2IsoMuon();
   muonIS->makePrivateStore();
   muonIS->setPt(0.0);

   if (muonCB->pt() == 0.) {
      m_ErrorFlagMI = 1;
      ATH_MSG_DEBUG( " L2CombinedMuon pt = 0 --> stop processing RoI"  );
      muonIS->setErrorFlag(m_ErrorFlagMI);
      muonISColl->push_back(muonIS);
      return muIsoSeed(outputTE, muonISColl);
   }

   muonIS->setPt(muonCB->pt());
   muonIS->setEta(muonCB->eta());
   muonIS->setPhi(muonCB->phi());
   muonIS->setCharge(muonCB->charge());
   muonIS->setSumEt01(0.0);
   muonIS->setSumEt02(0.0);
   muonIS->setSumEt03(0.0);
   muonIS->setSumEt04(0.0);

   double pt       = muonCB->pt();
   double eta      = muonCB->eta();
   double phi      = muonCB->phi();
   double zeta     = muonCB->idTrack()->z0();

   ATH_MSG_DEBUG( "Input L2CombinedMuon pt (GeV) = " << pt
                  << " / eta = "                    << eta
                  << " / phi = "                    << phi
                  << " / z = "                      << zeta );

   // ID tracks Decoding
   std::string algoId = m_ID_algo_to_use;
   const xAOD::TrackParticleContainer* idTrackParticles = 0;
   status = getFeature(outputTE, idTrackParticles, algoId);

   if (status != HLT::OK) {
      ATH_MSG_DEBUG( " Failed to get " << algoId << " xAOD::TrackParticleContainer --> ABORT"  );
      m_ErrorFlagMI = 2;
      return HLT::ErrorCode(HLT::Action::ABORT_CHAIN, HLT::Reason::MISSING_FEATURE);
   }
   if (!idTrackParticles) {
      ATH_MSG_DEBUG( "Pointer to xAOD::TrackParticleContainer[" << algoId << "] = 0"  );
      m_ErrorFlagMI = 2;
      muonIS->setErrorFlag(m_ErrorFlagMI);
      muonIS->setSumPt01(0.0);
      muonIS->setSumPt02(0.0);
      muonIS->setSumPt03(0.0);
      muonIS->setSumPt04(0.0);
      muonISColl->push_back(muonIS);
      return muIsoSeed(outputTE, muonISColl);
   }

   ATH_MSG_DEBUG( " Got xAOD::TrackParticleContainer with size: " << idTrackParticles->size()  );

   //ID based isolation
   float  sumpt01 = 0.0;
   float  sumpt02 = 0.0;
   float  sumpt03 = 0.0;
   float  sumpt04 = 0.0;
   int    ntrk    = 0;

   for (xAOD::TrackParticleContainer::const_iterator trkit = idTrackParticles->begin(); trkit != idTrackParticles->end(); ++trkit) {


      //Select tracks
      double phi_id   = (*trkit)->phi();
      double eta_id   = (*trkit)->eta();
      double q_id     = ((*trkit)->qOverP() > 0 ? 1.0 : -1.0);
      double pt_id    = (*trkit)->pt() * q_id;
      double z_id     = (*trkit)->z0();

      ATH_MSG_DEBUG( "Found track: "
                     << "  with pt (GeV) = " << pt_id / CLHEP::GeV
                     << ", eta =" << eta_id
                     << ", phi =" << phi_id );

      if ((fabs(pt_id) / CLHEP::GeV) < m_PtMinTrk)       continue;
      if (fabs(eta_id)               > m_EtaMaxTrk)      continue;

      double dzeta    = z_id - zeta;
      if (fabs(dzeta) > m_DzetaMax)       continue;

      ATH_MSG_DEBUG( "Track selected "  );

      ATH_MSG_DEBUG( "Found track: "
                     << m_ID_algo_to_use
                     << "  with pt = " << pt_id
                     << ", eta = " << eta_id
                     << ", phi = " << phi_id
                     << ", Zid = " << z_id
                     << ", DZeta = " << dzeta );

      //see if is in cone
      double deta = fabs(eta_id - eta);
      double dphi = fabs(phi_id - phi);
      if (dphi > M_PI) dphi = 2.* M_PI - dphi;

      double DR = sqrt(deta * deta + dphi * dphi);
      if (DR <= m_RID) {
         ntrk++;
         if (DR <= 0.1) sumpt01 += fabs(pt_id);
         if (DR <= 0.2) sumpt02 += fabs(pt_id);
         if (DR <= 0.3) sumpt03 += fabs(pt_id);
         if (DR <= 0.4) sumpt04 += fabs(pt_id);
      }
   }


   sumpt01 -= muonCB->idTrack()->pt();
   sumpt02 -= muonCB->idTrack()->pt();
   sumpt03 -= muonCB->idTrack()->pt();
   sumpt04 -= muonCB->idTrack()->pt();

   ATH_MSG_DEBUG( " REGTEST ID ALGO: ntrk / pt / sumpt02 / iso"
                  << " / " << ntrk
                  << " / " << pt
                  << " / " << sumpt02
                  << " / " << sumpt02 / pt );

   // updated monitored variables
   m_NTRK     = ntrk;
   m_Sumpt    = sumpt02 / CLHEP::GeV; //in GeV
   m_IDiso    = sumpt02 / pt;
   m_ErrorFlagMI = 0;
   m_MuPt        = pt;
   m_MuEta       = eta;
   m_MuPhi       = phi;
   if (m_IDiso > m_GlobIsoCut_ID) m_isIsolated = 0.0;
   if (m_isIsolated == 1) m_ptFL = -999999.;


   // Fill feature
   muonIS->setSumPt01(sumpt01);
   muonIS->setSumPt02(sumpt01);
   muonIS->setSumPt03(sumpt01);
   muonIS->setSumPt04(sumpt01);

   // Record results in SG to seed next algorithm
   muonISColl->push_back(muonIS);
   return muIsoSeed(outputTE, muonISColl);
}


HLT::ErrorCode muIso::muIsoSeed(HLT::TriggerElement* outputTE, xAOD::L2IsoMuonContainer* muon_cont)
{
   //seeding the next Algorithm
   std::string muonISCollKey = "MuonL2ISInfo";
   HLT::ErrorCode status = attachFeature(outputTE, muon_cont, muonISCollKey);
   if (status != HLT::OK) {
      outputTE->setActiveState(false);
      delete muon_cont;
      ATH_MSG_ERROR( " Record of xAOD::L2IsoMuonContainer in TriggerElement failed"
                     );
      return status;
   } else {
     ATH_MSG_DEBUG( " xAOD::L2IsoMuonContainer attached to the TriggerElement" );
   }
   outputTE->setActiveState(true);
   return HLT::OK;
}
