/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LArNoiseBursts_H
#define LArNoiseBursts_H

#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/ObjectVector.h"
#include "GaudiKernel/AlgTool.h"

#include "CLHEP/Units/SystemOfUnits.h"
#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/ITHistSvc.h"
#include "AnalysisTools/AnalysisTools.h"

//LAr services:
#include "Identifier/Range.h" 
#include "Identifier/IdentifierHash.h"
//#include "Identifier/HWIdentifier.h"
//#include "LArElecCalib/ILArPedestal.h"
#include "LArRecConditions/ILArBadChannelMasker.h"
#include "CaloInterface/ICaloNoiseTool.h"
#include "CaloInterface/ICalorimeterNoiseTool.h"
#include "TrigAnalysisInterfaces/IBunchCrossingTool.h"
#include "LArTools/LArCablingService.h"
#include "LArTools/LArHVCablingTool.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArElectrodeID.h"
//#include "CaloDetDescr/CaloDetDescrManager.h"
#include "LArCondUtils/LArHVToolDB.h"
#include "LArElecCalib/ILArHVTool.h"

// Trigger
#include "TrigDecisionTool/TrigDecisionTool.h"

// Electrons
#include "egammaEvent/ElectronContainer.h"

//CBNT_UTILS
//#include "CBNT_Utils/CBNT_AthenaAwareBase.h"
//#include "AthenaBaseComps/AthAlgorithm.h"

//STL:
#include <string>
#include <bitset>


class LArOnlineID;
class LArElectrodeID;
class HWIdentifier;
class LArOnlineIDStrHelper;
class LArCablingService;
class LArEM_ID;
class LArHEC_ID;
class LArFCAL_ID;
class CaloIdManager;
class CaloDetDescrManager;

class TileTBID;
class TgcIdHelper;

class TTree;

class LArHVLineID;

class LArNoiseBursts : public Algorithm  {

 public:

   LArNoiseBursts(const std::string& name, ISvcLocator* pSvcLocator);
   ~LArNoiseBursts();

   //virtual StatusCode initializeBeforeEventLoop();
   virtual StatusCode initialize();
   virtual StatusCode finalize();
   virtual StatusCode execute();
   virtual StatusCode clear();

 private:

   /** methods called by execute() */
   StatusCode doTrigger();
   StatusCode doEventProperties();
   StatusCode doMBTSTimming();    
   StatusCode doLArNoiseBursts();  
   StatusCode doPhysicsObjects();

   //functions
   int GetPartitionLayerIndex(const Identifier& id);
   std::vector<int>* GetHVLines(const Identifier& id);
     

 private:

   StoreGateSvc* m_detectorStore;
   ITHistSvc * m_thistSvc;
   StoreGateSvc* m_storeGate;
    
   TTree* m_tree;

   /*Tools*/
   ToolHandle<LArCablingService> m_LArCablingService;
   ToolHandle<LArHVCablingTool> m_LArHVCablingTool;
   ToolHandle<ICaloNoiseTool> m_calo_noise_tool;
   ToolHandle<Trig::IBunchCrossingTool> m_bc_tool;

   ToolHandle< Trig::TrigDecisionTool > m_trigDec;

   /*services*/
   const LArOnlineID* m_LArOnlineIDHelper;
   const LArHVLineID* m_LArHVLineIDHelper;
   const LArElectrodeID* m_LArElectrodeIDHelper;
   const LArEM_ID* m_LArEM_IDHelper;
   const LArFCAL_ID* m_LArFCAL_IDHelper;
   const LArHEC_ID*  m_LArHEC_IDHelper;
   const CaloIdManager*  m_caloIdMgr;
   const CaloDetDescrManager* m_calodetdescrmgr;

   // Electrons
   std::string m_elecContainerName;

   /*event cuts */
   double m_sigmacut;
   double m_MBTSThreshold;  
   bool m_CosmicCaloStream;
   int m_frontbunches;
   bool m_keepOnlyCellID;

   /*declaration of branches*/
   int m_nb_sat;
   int m_lowqfactor;
   int m_medqfactor;
   int m_hiqfactor;
   long n_noisycell;
   int  m_nt_larcellsize;
   int  m_nt_cellsize;
   int  m_nt_npv;
   int  m_nt_npvtracks;
   //int  m_nt_cellpartition;
   //int  m_nt_runnumber;
   int  m_nt_evtId;
   int  m_nt_evtTime;
   int  m_nt_evtTime_ns;
   int  m_nt_lb;
   int  m_nt_bcid;
   int  m_nt_ntracks;
   int  m_nt_isbcidFilled;
   int  m_nt_isbcidInTrain;
   std::vector<int> m_nt_isBunchesInFront;
   int  m_nt_bunchtype;
   float m_nt_bunchtime;
   unsigned int m_nt_atlasready;
   unsigned int m_nt_stablebeams;
   std::vector<std::string> m_nt_streamTagName;
   std::vector<std::string> m_nt_streamTagType;
   short  m_nt_larnoisyro;
   short  m_nt_larnoisyro_satOne;
   short  m_nt_larnoisyro_satTwo;

   //event info veto variables
   short  m_nt_veto_mbts;
   //short  m_nt_veto_indet;
   short  m_nt_veto_bcm;
   short  m_nt_veto_lucid;
   short  m_nt_veto_pixel;
   short  m_nt_veto_sct;
   short  m_nt_veto_mbtstdHalo;
   short  m_nt_veto_mbtstdCol;
   short  m_nt_veto_lartdHalo;
   short  m_nt_veto_lartdCol;
   short  m_nt_veto_csctdHalo;
   short  m_nt_veto_csctdCol;
   short  m_nt_veto_bcmtHalo;
   short  m_nt_veto_bcmtCol;
   short  m_nt_veto_muontCol;
   short  m_nt_veto_muontCosmic;

   // LAr event bit info
   bool m_nt_larflag_badFEBs;
   bool m_nt_larflag_mediumSaturatedDQ;
   bool m_nt_larflag_tightSaturatedDQ;
   bool m_nt_larflag_noiseBurstVeto;
   bool m_nt_larflag_dataCorrupted;
   bool m_nt_larflag_dataCorruptedVeto;

   // NoiseBurst trigger
   bool m_nt_L1_J75;
   bool m_nt_L1_J10_EMPTY;
   bool m_nt_L1_J30_FIRSTEMPTY;
   bool m_nt_L1_J30_EMPTY;
   bool m_nt_L1_XE40;
   bool m_nt_L1_XE50;
   bool m_nt_L1_XE50_BGRP7;
   bool m_nt_L1_XE70;

   bool m_nt_EF_j165_u0uchad_LArNoiseBurst;
   bool m_nt_EF_j30_u0uchad_empty_LArNoiseBurst;
   bool m_nt_EF_j55_u0uchad_firstempty_LArNoiseBurst;
   bool m_nt_EF_j55_u0uchad_empty_LArNoiseBurst;
   bool m_nt_EF_xe45_LArNoiseBurst;
   bool m_nt_EF_xe55_LArNoiseBurst;
   bool m_nt_EF_xe60_LArNoiseBurst;
   bool m_nt_EF_j55_u0uchad_firstempty_LArNoiseBurstT;
   bool m_nt_EF_j100_u0uchad_LArNoiseBurstT;
   bool m_nt_EF_j165_u0uchad_LArNoiseBurstT;
   bool m_nt_EF_j130_u0uchad_LArNoiseBurstT;
   bool m_nt_EF_j35_u0uchad_empty_LArNoiseBurst;
   bool m_nt_EF_j35_u0uchad_firstempty_LArNoiseBurst;
   bool m_nt_EF_j80_u0uchad_LArNoiseBurstT;
   
   //Pixel variables
   int m_nt_nPixelhits;
   int m_nt_nPixelclusters;

   //SCT variables
   int m_nt_nSCThits;
   int m_nt_nSCTclusters;
   int m_nt_nSCTspacepoints;
   //TRT variables
   int m_nt_nTRThits;

   //TGC variables
   int m_nt_nTGChits_a;
   int m_nt_nTGChits_c;

   //LAr Time variables
   float m_nt_ECTimeDiff;
   float m_nt_ECTimeAvg;
   int  m_nt_nCellA;
   int  m_nt_nCellC;

   //MBTS Time variables   
   float  m_nt_mbtstimediff; 
   int    m_nt_nmbtscellA;
   int    m_nt_nmbtscellC;
   float  m_nt_mbtstimeavrg;

   //Quantites for noisy cells
   std::vector<float> m_nt_energycell;
   std::vector<float> m_nt_qfactorcell;
   std::vector<float> m_nt_phicell;
   std::vector<float> m_nt_etacell;
   std::vector<float> m_nt_signifcell;
   float m_nt_noisycellpercent;
   std::vector<short> m_nt_ft_noisy;
   std::vector<short> m_nt_slot_noisy;
   std::vector<short> m_nt_channel_noisy;
   std::vector<short>    m_nt_cellpartlayerindex;
   std::vector< unsigned int > m_nt_cellIdentifier;
   std::vector<float> m_nt_noisycellpart;
   std::vector<int> m_nt_nominalhv;
   std::vector<int> m_nt_maximalhv;
   std::vector<int> m_nt_noisycellHVphi;
   std::vector<int> m_nt_noisycellHVeta;
   std::vector<std::vector<short> > m_nt_samples;
   std::vector<int> m_nt_gain;
   std::vector<int> m_nt_isbadcell;

   /*
   std::vector<int> m_nt_isbarrel;
   std::vector<int> m_nt_isendcap;
   std::vector<int> m_nt_isfcal;
   std::vector<int> m_nt_ishec;
   */
   
   std::vector<int>  m_nt_partition;
   std::vector<int>  m_nt_layer;
   
   //Quanties for the saturated cells
   std::vector<int> m_nt_isbadcell_sat;
   std::vector<int>  m_nt_barrelec_sat;
   std::vector<int>  m_nt_posneg_sat;
   std::vector<short>  m_nt_ft_sat;
   std::vector<short>  m_nt_slot_sat;
   std::vector<short>  m_nt_channel_sat;
   std::vector<int>  m_nt_partition_sat;
   std::vector<float> m_nt_energy_sat;
   std::vector<float> m_nt_phicell_sat;
   std::vector<float> m_nt_etacell_sat;
   std::vector<int> m_nt_layer_sat;
   std::vector<unsigned int> m_nt_cellIdentifier_sat;

   ///ZDC quantities
   int   m_nt_nzdcAside;
   int   m_nt_nzdcCside;
   float m_nt_zdcAsideenergy;
   float m_nt_zdcCsideenergy;
   float m_nt_zdctimediff;

   //LUCID quantities
   int m_nt_lucidA;
   int m_nt_lucidAnBX;
   int m_nt_lucidApBX;

   int m_nt_lucidC;
   int m_nt_lucidCnBX;
   int m_nt_lucidCpBX;

   int m_nt_lucidchannels;
   int m_nt_lucidchannelsnBX;
   int m_nt_lucidchannelspBX;

   //electron variables
   int   m_nt_el_n;
   std::vector<float> m_nt_el_E;
   std::vector<float> m_nt_el_Et;
   std::vector<float> m_nt_el_eta;
   std::vector<float> m_nt_el_phi;
   std::vector<float> m_nt_el_cl_E;
   std::vector<float> m_nt_el_cl_Et;
   std::vector<float> m_nt_el_cl_eta;
   std::vector<float> m_nt_el_cl_phi;
   std::vector<int> m_nt_el_author;
   std::vector<int> m_nt_el_loosePP;
   std::vector<int> m_nt_el_mediumPP;
   std::vector<int> m_nt_el_tightPP;
};

#endif // LArNoiseBursts_H

