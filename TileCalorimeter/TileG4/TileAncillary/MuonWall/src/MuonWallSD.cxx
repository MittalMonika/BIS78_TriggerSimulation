/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

//************************************************************
//
// Class MuonWallSD.
// Sensitive detector for the muon wall
//
//************************************************************

#include "MuonWallSD.h"

#include "CaloIdentifier/TileTBID.h"
#include "StoreGate/StoreGateSvc.h"
#include "TileSimEvent/TileHitVector.h"

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Bootstrap.h"

#include "G4HCofThisEvent.hh"
#include "G4VPhysicalVolume.hh"
#include "G4Step.hh"
#include "G4VTouchable.hh"
#include "G4TouchableHistory.hh"

MuonWallSD::MuonWallSD(const std::string& name, const std::string& hitCollectionName, int verbose)
    : G4VSensitiveDetector(name),
    m_nhits(),
    m_hit(),
    m_HitColl(hitCollectionName)
{
  verboseLevel = std::max(verboseLevel, verbose);

  ISvcLocator* svcLocator = Gaudi::svcLocator();

  StoreGateSvc* detStore(nullptr);
  if (svcLocator->service("DetectorStore", detStore).isFailure()) {
    G4ExceptionDescription description;
    description << "Constructor: DetectorStoreSvc not found!";
    G4Exception("MuonWallSD", "NoDetStore", FatalException, description);
    abort();
  } else if (verboseLevel >= 5) {
    G4cout << "DetectorStoreSvc initialized" << G4endl;
  }

  if (detStore->retrieve(m_tileTBID).isFailure()) {
    G4ExceptionDescription description;
    description << "Constructor: No TileTBID helper!";
    G4Exception("MuonWallSD", "NoTileTBIDHelper", FatalException, description);
    abort();
  } else if (verboseLevel >= 5) {
    G4cout << "TileTBID helper retrieved" << G4endl;
  }

  int type=TileTBID::ADC_TYPE, module=TileTBID::BACK_WALL;
  for (int channel=0; channel<s_nCellMu; ++channel) {
    m_id[channel] = m_tileTBID->channel_id(type,module,channel);
  }

  module=TileTBID::CRACK_WALL;
  for (int channel=0; channel<s_nCellS; ++channel) {
    m_id[channel+s_nCellMu] = m_tileTBID->channel_id(type,module,channel);
  }
}

MuonWallSD::~MuonWallSD() {
}

void MuonWallSD::StartOfAthenaEvent() {
  if (verboseLevel >= 5) {
    G4cout << "Initializing SD" << G4endl;
  }

  memset(m_nhits, 0, sizeof(m_nhits));
}

void MuonWallSD::Initialize(G4HCofThisEvent* /* HCE */) {
  if (verboseLevel >= 5) {
    G4cout << "MuonWallSD::Initialize()" << G4endl;
  }

  if (!m_HitColl.isValid()) {
    m_HitColl = std::make_unique<TileHitVector>(m_HitColl.name());
  }
}

G4bool MuonWallSD::ProcessHits(G4Step* aStep, G4TouchableHistory* /* ROhist */) {
  if (verboseLevel >= 10) {
    G4cout << "MuonWallSD::ProcessHits" << G4endl;
  }

  const G4TouchableHistory* theTouchable = (G4TouchableHistory*)(aStep->GetPreStepPoint()->GetTouchable());
  const G4VPhysicalVolume* physVol = theTouchable->GetVolume();
  const G4LogicalVolume* logiVol = physVol->GetLogicalVolume();
  const G4String nameLogiVol = logiVol->GetName();
  const G4int nScinti = physVol->GetCopyNo();

  const G4double edep = aStep->GetTotalEnergyDeposit() * aStep->GetTrack()->GetWeight();
  G4double stepl = 0.;

  if (aStep->GetTrack()->GetDefinition()->GetPDGCharge() != 0.){ // FIXME not-equal check on double

    stepl = aStep->GetStepLength();
  }

  if ((edep == 0.) && (stepl == 0.)) { //FIXME equality check on double

    return false;
  }

  int ind;

  if(nameLogiVol.find("MuScintillatorLayer") !=G4String::npos) {
    // for muon wall, nScinti-1 is the correct indice.
    ind = nScinti-1;
  } else if(nameLogiVol.find("S1") !=G4String::npos) {
    ind = s_nCellMu + 0;
  } else if(nameLogiVol.find("S2") !=G4String::npos) {
    ind = s_nCellMu + 1;
  } else if(nameLogiVol.find("S3") !=G4String::npos) {
    ind = s_nCellMu + 2;
  } else {
    ind = s_nCellMu + 3;
  }

  if (verboseLevel >= 10) {
    G4cout << ((m_nhits[ind] > 0)?"Additional hit in ":"First hit in ")
           << ((ind<s_nCellMu)?"MuonWall ":"beam counter S")
           << ((ind<s_nCellMu)?nScinti:(ind-s_nCellMu+1))
           << " time=" << aStep->GetPostStepPoint()->GetGlobalTime()
           << " ene=" << edep << G4endl;
  }

  if ( m_nhits[ind] > 0 ) {
    m_hit[ind]->add(edep,0.0,0.0);
  } else {
    // First hit in a cell
    m_hit[ind] = new TileSimHit(m_id[ind],edep,0.0,0.0);
  }

  ++m_nhits[ind];

  return true;
}

void MuonWallSD::EndOfAthenaEvent() {
  for (int ind = 0; ind < s_nCell; ++ind) {
    int nhit = m_nhits[ind];
    if (nhit > 0) {
      if (verboseLevel >= 5) {
        G4cout << "Cell id=" << m_tileTBID->to_string(m_id[ind])
               << " nhit=" << nhit
               << " ene=" << m_hit[ind]->energy() << G4endl;
      }
      m_HitColl->Insert(TileHit(m_hit[ind]));
      delete m_hit[ind];
    } else if (verboseLevel >= 10) {
      G4cout << "Cell id=" << m_tileTBID->to_string(m_id[ind])
             << " nhit=0" << G4endl;
    }
  }

  if (verboseLevel >= 5) {
    G4cout << "Total number of hits is " << m_HitColl->size() << G4endl;
  }
  return ;
}
