/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include "MMRDOVariables.h"
#include "AthenaKernel/errorcheck.h"

#include "MuonSimData/MuonSimDataCollection.h"

#include "MuonRDO/MM_RawDataContainer.h"

#include "MuonReadoutGeometry/MMReadoutElement.h"

#include "TTree.h"

using namespace Muon;

/** ---------- filling of variables */
/** ---------- to be called on each evt i.e. execute level of main alg */
StatusCode MMRDOVariables::fillVariables()
{
  ATH_MSG_DEBUG("do fillNSWMMRDOVariables()");

  // clear variables
  CHECK( this->clearVariables() );

  // get the rdo (a container corresponds to a multilayer of a module)
  const MM_RawDataContainer* rdo_container = nullptr;
  CHECK( m_evtStore->retrieve(rdo_container, m_ContainerName.c_str()) );

  if(rdo_container->size()==0) ATH_MSG_WARNING(" MM RDO Container empty ");

  // iteration on all containers, i.e. all multilayers of all modules
  for(auto it : *rdo_container) {
    // a digit collection is instanciated for each container, i.e. holds all digits of a multilayer
    const MM_RawDataCollection* coll = it;

    // loop on all digits inside a collection, i.e. multilayer
    for (unsigned int item=0; item<coll->size(); item++) {

      // get specific digit and identify it
      const MM_RawData* rdo = coll->at(item);
      Identifier Id = rdo->identify();

      std::string stName   = m_MmIdHelper->stationNameString(m_MmIdHelper->stationName(Id));
      int stationEta       = m_MmIdHelper->stationEta(Id);
      int stationPhi       = m_MmIdHelper->stationPhi(Id);
      int multiplet        = m_MmIdHelper->multilayer(Id);
      int gas_gap          = m_MmIdHelper->gasGap(Id);
      int channel          = m_MmIdHelper->channel(Id);

      ATH_MSG_DEBUG(     "MicroMegas RDO Offline id:  Station Name [" << stName << " ]"
                      << " Station Eta ["  << stationEta      << "]"
                      << " Station Phi ["  << stationPhi      << "]"
                      << " Multiplet  ["   << multiplet       << "]"
                      << " GasGap ["       << gas_gap         << "]"
                      << " ChNr ["         << channel         << "]" );

      // module details down to the level of channel which is closest to the Geant4 hit
      // to be stored in the ntuple
      m_NSWMM_rdo_stationName->push_back(stName);
      m_NSWMM_rdo_stationEta->push_back(stationEta);
      m_NSWMM_rdo_stationPhi->push_back(stationPhi);
      m_NSWMM_rdo_multiplet->push_back(multiplet);
      m_NSWMM_rdo_gas_gap->push_back(gas_gap);
      m_NSWMM_rdo_channel->push_back(channel);
      m_NSWMM_rdo_time->push_back(rdo->time());
      m_NSWMM_rdo_charge->push_back(rdo->charge());

      // get the readout element class where the RDO is recorded
      int isSmall = (stName[2] == 'S');
      const MuonGM::MMReadoutElement* rdoEl = m_detManager->getMMRElement_fromIdFields(isSmall, stationEta, stationPhi, multiplet );


      // rdo counter for the ntuple
      m_NSWMM_nrdo++;
    }
  }

  ATH_MSG_DEBUG("processed " << m_NSWMM_nrdo << " MicroMegas rdo");
  return StatusCode::SUCCESS;
}


/** ---------- clearing of variables */
/** ---------- to be called inside filling method before filling starts */
StatusCode MMRDOVariables::clearVariables()
{
  m_NSWMM_nrdo = 0;

  // information of the module down to the channel closest to the initial G4 hit
  // size of vector is m_NSWMM_rdo
  m_NSWMM_rdo_stationName->clear();
  m_NSWMM_rdo_stationEta->clear();
  m_NSWMM_rdo_stationPhi->clear();
  m_NSWMM_rdo_multiplet->clear();
  m_NSWMM_rdo_gas_gap->clear();
  m_NSWMM_rdo_channel->clear();

  return StatusCode::SUCCESS;
}


/** ---------- creating variables and associate them to branches */
/** ---------- to be called on initialization level of main alg */
StatusCode MMRDOVariables::initializeVariables()
{

  m_NSWMM_nrdo = 0;
  m_NSWMM_rdo_stationName = new std::vector<std::string>();
  m_NSWMM_rdo_stationEta  = new std::vector<int>();
  m_NSWMM_rdo_stationPhi  = new std::vector<int>();
  m_NSWMM_rdo_multiplet   = new std::vector<int>();
  m_NSWMM_rdo_gas_gap     = new std::vector<int>();
  m_NSWMM_rdo_channel     = new std::vector<int>();
  m_NSWMM_rdo_time        = new std::vector<int>();
  m_NSWMM_rdo_charge      = new std::vector<int>();


  if(m_tree) {
    m_tree->Branch("nrdo_MM",            &m_NSWMM_nrdo);
    m_tree->Branch("RDO_MM_stationName", &m_NSWMM_rdo_stationName);
    m_tree->Branch("RDO_MM_stationEta",  &m_NSWMM_rdo_stationEta);
    m_tree->Branch("RDO_MM_stationPhi",  &m_NSWMM_rdo_stationPhi);
    m_tree->Branch("RDO_MM_multiplet",   &m_NSWMM_rdo_multiplet);
    m_tree->Branch("RDO_MM_gas_gap",     &m_NSWMM_rdo_gas_gap);
    m_tree->Branch("RDO_MM_channel",     &m_NSWMM_rdo_channel);
  }

  return StatusCode::SUCCESS;
}


/** ---------- freeing resources and resetting pointers */
/** ---------- to be called on finalize level of main alg */
void MMRDOVariables::deleteVariables()
{
  delete m_NSWMM_rdo_stationName;
  delete m_NSWMM_rdo_stationEta;
  delete m_NSWMM_rdo_stationPhi;
  delete m_NSWMM_rdo_multiplet;
  delete m_NSWMM_rdo_gas_gap;
  delete m_NSWMM_rdo_channel;
  

  m_NSWMM_nrdo = 0;
  m_NSWMM_rdo_stationName = nullptr;
  m_NSWMM_rdo_stationEta = nullptr;
  m_NSWMM_rdo_stationPhi = nullptr;
  m_NSWMM_rdo_multiplet = nullptr;
  m_NSWMM_rdo_gas_gap = nullptr;
  m_NSWMM_rdo_channel = nullptr;

  return;
}
