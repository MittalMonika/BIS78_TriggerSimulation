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
// $Id: HadronPhysicsQGSP_BERT.cc,v 1.5 2010-06-15 11:03:50 vnivanch Exp $
// GEANT4 tag $Name: geant4-09-04-patch-01 $
//
//---------------------------------------------------------------------------
//
// ClassName:   HadronPhysicsQGSP_BERT
//
// Author: 2002 J.P. Wellisch
//
// Modified:
// 23.11.2005 G.Folger: migration to non static particles
// 08.06.2006 V.Ivanchenko: remove stopping
// 20.06.2006 G.Folger: Bertini applies to Kaons, i.e. use SetMinEnergy instead of SetMinPionEnergy
// 25.04.2007 G.Folger: Add code for quasielastic
// 10.12.2007 G.Folger: Add projectilediffrative option for proton/neutron, off by default
//
//----------------------------------------------------------------------------
//
#include "HadronPhysicsQGSP_BERT_MuBias.hh"

#include "globals.hh"
#include "G4ios.hh"
#include <iomanip>   
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"

#include "G4MesonConstructor.hh"
#include "G4BaryonConstructor.hh"
#include "G4ShortLivedConstructor.hh"

#include "CLHEP/Units/PhysicalConstants.h"

HadronPhysicsQGSP_BERT_MuBias::HadronPhysicsQGSP_BERT_MuBias(G4int)
                    :  G4VPhysicsConstructor("hInelastic QGSP_BERT"),
    theNeutrons(0),
    theLEPNeutron(0),
    theQGSPNeutron(0),
    theBertiniNeutron(0),
    thePiK(0),
    theLEPPiK(0),
    theQGSPPiK(0),
    theBertiniPiK(0),
    thePro(0),
    theLEPPro(0),
    theQGSPPro(0), 
    theBertiniPro(0),
    theMiscLHEP(0),
    QuasiElastic(true),
    ProjectileDiffraction(false)
{
}

HadronPhysicsQGSP_BERT_MuBias::HadronPhysicsQGSP_BERT_MuBias(const G4String& name, G4bool quasiElastic)
                    :  G4VPhysicsConstructor(name),
    theNeutrons(0),
    theLEPNeutron(0),
    theQGSPNeutron(0),
    theBertiniNeutron(0),
    thePiK(0),
    theLEPPiK(0),
    theQGSPPiK(0),
    theBertiniPiK(0),
    thePro(0),
    theLEPPro(0),
    theQGSPPro(0), 
    theBertiniPro(0),
    theMiscLHEP(0),
    QuasiElastic(quasiElastic),
    ProjectileDiffraction(false)
{
}

void HadronPhysicsQGSP_BERT_MuBias::CreateModels()
{
  theNeutrons=new G4NeutronBuilder;
  theNeutrons->RegisterMe(theQGSPNeutron=new G4QGSPNeutronBuilder(QuasiElastic, ProjectileDiffraction));
  theNeutrons->RegisterMe(theLEPNeutron=new G4LEPNeutronBuilder);
  theLEPNeutron->SetMinInelasticEnergy(9.5*CLHEP::GeV);
  theLEPNeutron->SetMaxInelasticEnergy(25*CLHEP::GeV);  

  theNeutrons->RegisterMe(theBertiniNeutron=new G4BertiniNeutronBuilder);
  theBertiniNeutron->SetMinEnergy(0.0*CLHEP::GeV);
  theBertiniNeutron->SetMaxEnergy(9.9*CLHEP::GeV);

  thePro=new G4ProtonBuilder;
  thePro->RegisterMe(theQGSPPro=new G4QGSPProtonBuilder(QuasiElastic, ProjectileDiffraction));
  thePro->RegisterMe(theLEPPro=new G4LEPProtonBuilder);
  theLEPPro->SetMinEnergy(9.5*CLHEP::GeV);
  theLEPPro->SetMaxEnergy(25*CLHEP::GeV);

  thePro->RegisterMe(theBertiniPro=new G4BertiniProtonBuilder);
  theBertiniPro->SetMaxEnergy(9.9*CLHEP::GeV);
  
  thePiK=new G4PiKBuilder;
  thePiK->RegisterMe(theQGSPPiK=new G4QGSPPiKBuilder(QuasiElastic));
  thePiK->RegisterMe(theLEPPiK=new G4LEPPiKBuilder);
  theLEPPiK->SetMaxEnergy(25*CLHEP::GeV);
  theLEPPiK->SetMinEnergy(9.5*CLHEP::GeV);

  thePiK->RegisterMe(theBertiniPiK=new G4BertiniPiKBuilder);
  theBertiniPiK->SetMaxEnergy(9.9*CLHEP::GeV);
  
  theMiscLHEP=new G4MiscLHEPBuilder;
}

HadronPhysicsQGSP_BERT_MuBias::~HadronPhysicsQGSP_BERT_MuBias()
{
   delete theMiscLHEP;
   delete theQGSPNeutron;
   delete theLEPNeutron;
   delete theNeutrons;
   delete theBertiniNeutron;
   delete theQGSPPro;
   delete theLEPPro;
   delete thePro;
   delete theBertiniPro;
   delete theQGSPPiK;
   delete theLEPPiK;
   delete theBertiniPiK;
   delete thePiK;
}

void HadronPhysicsQGSP_BERT_MuBias::ConstructParticle()
{
  G4MesonConstructor pMesonConstructor;
  pMesonConstructor.ConstructParticle();

  G4BaryonConstructor pBaryonConstructor;
  pBaryonConstructor.ConstructParticle();

  G4ShortLivedConstructor pShortLivedConstructor;
  pShortLivedConstructor.ConstructParticle();  
}

#include "G4ProcessManager.hh"
void HadronPhysicsQGSP_BERT_MuBias::ConstructProcess()
{
  CreateModels();
  theNeutrons->Build();
  thePro->Build();
  thePiK->Build();
  theMiscLHEP->Build();
}

