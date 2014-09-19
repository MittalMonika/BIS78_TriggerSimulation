/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "G4AtlasUtilities/VertexRangeChecker.h"
#include "G4PrimaryVertex.hh"
#include "G4VSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"

using namespace std;

// All the GenVertex objects from the Event Generation step will be
// converted into G4PrimaryVertex objects. This method will process
// each of them in turn.
// See https://twiki.cern.ch/twiki/bin/viewauth/Atlas/AtlasG4EventFilters for more details.

bool VertexRangeChecker::EditVertex(G4PrimaryVertex* v) {
  if (!onSwitch) return true;
  G4ThreeVector p = v->GetPosition();
  if (verboseLevel > 0) {
    cout << " VertexRangeChecker::EditVertex: vertex at " << p << endl;
  }

  if (Z>0 && fabs(p.z()) > Z ){
    if (verboseLevel > 0) {
      cout << "  vertex at " << p << " discarded because outside the valid range " << endl;
    }
    return false;
  }
  if (Rmax>0){
    const double rho2 = p.x()*p.x() + p.y()*p.y();
    if (rho2 < Rmin*Rmin || rho2 > Rmax*Rmax){
      if (verboseLevel > 0) {
        cout << "  vertex at " << p << " discarded because outside the valid range " << endl;
      }
      return false;
    }
  }

  if (Z<0 || Rmax<0){
    if (m_sWorld==0){ // First time initialization
      G4LogicalVolumeStore * lvs = G4LogicalVolumeStore::GetInstance();
      for (unsigned int i=0;i<lvs->size();++i){
        if ( ! (lvs->at(i)) ) continue;
        if ( lvs->at(i)->GetName()=="Atlas::Atlas" ){
          m_sWorld=lvs->at(i)->GetSolid();
          break;
        }
      }
      if (m_sWorld==0){
        std::cout << "World volume not found!  Bailing out!" << std::endl;
        throw;
      }
    }

    if ( m_sWorld->Inside( p ) == kOutside ){
      if (Z>0 || Rmax>0){
        cout << "You are living in limbo.  Manually set Z or R coordinate, but point reported outside of the mother volume." << endl;
        cout << "  Will ditch the point because I'm not sure what else to do,  For very unusual applications, this may cause" << endl;
        cout << "  problems in your simulation.  Please report this error message to atlas-simulation-team@cern.ch" << endl;
        cout << "  You can \"fix\" the problem by setting both the Z and R world ranges at the same time." << endl;
      }
      if (verboseLevel > 0) {
        cout << "  vertex at " << p << " discarded because outside the world volume : " << m_sWorld->DistanceToIn( p ) << " " << m_sWorld->DistanceToOut( p )  << endl;
      }
      return false;
    }
  }

  if (verboseLevel > 0) {
    cout << "  vertex at " << p << " accepted." << endl;
  }
  return true;
}



