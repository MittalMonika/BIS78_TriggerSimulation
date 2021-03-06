//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id:$
//
//---------------------------------------------------------------------------
// Author: Alberto Ribon
// Date:   October 2017
//
// New physics list QGSP_FTFP_BERT_ATL.
// This is a modified version of the FTFP_BERT_ATL physics list for ATLAS.
// The physics list QGSP_FTFP_BERT_ATL uses QGSP above 25 GeV, with 
// transition between FTFP and QGSP between 12 - 25 GeV.
//----------------------------------------------------------------------------
//
#ifndef TQGSP_FTFP_BERT_ATL_h
#define TQGSP_FTFP_BERT_ATL_h 1

#include <CLHEP/Units/SystemOfUnits.h>

#include "globals.hh"
#include "G4VModularPhysicsList.hh"
#include "CompileTimeConstraints.hh"

template<class T>
class TQGSP_FTFP_BERT_ATL: public T
{
public:
  TQGSP_FTFP_BERT_ATL(G4int ver = 1);
  virtual ~TQGSP_FTFP_BERT_ATL();
  
public:
  // SetCuts() 
  virtual void SetCuts();

private:
  enum {ok = CompileTimeConstraints::IsA<T, G4VModularPhysicsList>::ok };
};
#include "QGSP_FTFP_BERT_ATL.icc"
typedef TQGSP_FTFP_BERT_ATL<G4VModularPhysicsList> QGSP_FTFP_BERT_ATL;

#endif

