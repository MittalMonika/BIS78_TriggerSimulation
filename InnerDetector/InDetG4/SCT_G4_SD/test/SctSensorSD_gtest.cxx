/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "src/SctSensorSD.h"

#include "gtest/gtest.h"

#include "TestTools/initGaudi.h"

#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4TouchableHistory.hh"

#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4DynamicParticle.hh"
#include "G4ThreeVector.hh"
#include "G4Box.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4VPhysicalVolume.hh"

#include "G4AtlasTools/DerivedG4PhysicalVolume.h"
#include "G4AtlasTools/DerivedG4SensitiveDetectorTestSetting.h"
#include "MCTruth/TrackHelper.h"

//set environment
class GaudiEnvironment : public ::testing::Environment {
  protected:
  virtual void SetUp() override {
    Athena_test::initGaudi("SCT_G4_SD/optionForTest.txt", m_svcLoc);
  }
  ISvcLocator* m_svcLoc = nullptr;
};
class SctSensorSDtest : public ::testing::Test {	
  protected:
    virtual void SetUp() override {
    }

    virtual void TearDown() override {
    }
};
//end of environment setting

TEST_F( SctSensorSDtest, Initialize ) {
  G4HCofThisEvent hce;
  SctSensorSD sd1("name1", "name1" );
  sd1.Initialize( &hce );
  ASSERT_TRUE(sd1.m_HitColl.isValid()); //check if initialization of m_HitColl is successful
}

TEST_F( SctSensorSDtest, ProcessHits ) 
{
  G4Step sp;
  G4TouchableHistory th; 
  G4HCofThisEvent hce;  

  G4double totalenergydeposit = 3.0;
  G4String physicalname = "physicalName";
  G4String logicalname = "BBBBBBBBBTubeGas";
  G4int copyno = 1000;
  G4ThreeVector preStepPos = G4ThreeVector(0,0,1);
  G4ThreeVector postStepPos = G4ThreeVector(0,0,2);
  G4double globaltime0 = 0.5;
  G4double kineticenergy0 = 1.5;
  G4double velocity0 = 99.93100;
  G4double globaltime = 5.0;
  G4double kineticenergy = 0.5;
  G4double globaltime1 = 0.5;
  G4double kineticenergy1 = 0.5;
  G4double velocity1 = 99.93100;
  G4double steplength = 1.0;
  G4double charge = 1.0;
  G4int encoding = 22;
  G4int antiencoding = 22;
  G4String astring = "Cerenkov";
  G4ProcessType atype = (G4ProcessType)0;
  G4String nop1 = "opticalphoton";
  G4String nop2 = "opticalphoton";
  G4String nop3 = "photon";
  DerivedG4SensitiveDetectorTestSetting(sp, totalenergydeposit, physicalname, logicalname, copyno, preStepPos, postStepPos, globaltime0, kineticenergy0, velocity0, globaltime, kineticenergy, globaltime1, kineticenergy1, velocity1, steplength, charge, encoding, antiencoding, astring, atype, nop1, nop2, nop3);

  SctSensorSD sd2("name2", "name2");
  sd2.Initialize( &hce );
  sd2.ProcessHits(&sp, &th );

  HepGeom::Point3D<double> P1,P2;
  P1[SiHit::xEta] = 1;
  P1[SiHit::xPhi] = 0;
  P1[SiHit::xDep] = 0;
  P2[SiHit::xEta] = 2;
  P2[SiHit::xPhi] = 0;
  P2[SiHit::xDep] = 0;

  int barcode = 0;
  HepMcParticleLink plink(barcode);

  SiHitCollection* a = sd2.m_HitColl.ptr();
  ASSERT_EQ(a->begin()->localStartPosition(), P1); //test if the localStartPosition value of the Hit generated by this member function is right, the same below
  ASSERT_EQ(a->begin()->localEndPosition(), P2);
  ASSERT_EQ(a->begin()->energyLoss(), 3.0);
  ASSERT_EQ(a->begin()->meanTime(), 0.5);
  ASSERT_EQ(a->begin()->particleLink(), plink);
  ASSERT_EQ(a->begin()->identify(),4101273733);

  ASSERT_EQ(a->size(), 1); //test the current size of the Hit container
}

TEST_F( SctSensorSDtest, indexMethod )
{
// decorate G4TouchableHistory object that will be used as the actual para of indexMethod
  G4NavigationHistory* navigationHistory = new G4NavigationHistory();
  G4String boxName = "name";
  G4Box* box = new G4Box(boxName, 1.0, 1.0, 1.0);
  G4NistManager* man = G4NistManager::Instance();
  G4Material* material = man->FindOrBuildMaterial("G4_AIR");
  G4String name = "logicalName";
  G4LogicalVolume fLogical(box, material, name);
  G4String name1 = "physicalName";
  G4VPhysicalVolume* pPhysical = NULL;
  G4MyPhysicalVolume physicalVolume(0, G4ThreeVector(0,0,0), name1, &fLogical, pPhysical);
  physicalVolume.SetCopyNo(1000);
  G4int nReplica = 2;
  navigationHistory->SetFirstEntry(&physicalVolume);
  navigationHistory->NewLevel(&physicalVolume, kNormal, nReplica);
  navigationHistory->NewLevel(&physicalVolume, kNormal, nReplica);
  navigationHistory->NewLevel(&physicalVolume, kNormal, nReplica);
  navigationHistory->NewLevel(&physicalVolume, kNormal, nReplica);
  navigationHistory->NewLevel(&physicalVolume, kNormal, nReplica);
  G4TouchableHistory th(*navigationHistory);
// end

  G4HCofThisEvent hce;

  double coord1z = 1;
  int brlEcap = 0;
  int layerDisk = 0;
  int etaMod = 0;
  int phiMod = 0;
  int side = 0;

  SctSensorSD sd3("name3", "name3");
  sd3.Initialize( &hce );
  sd3.indexMethod(&th, coord1z, brlEcap, layerDisk, etaMod, phiMod, side );

  ASSERT_EQ(brlEcap, 0); //test the value of brlEcap changed by the member function indexMethod, it should be 0 in this case. The same below
  ASSERT_EQ(layerDisk, 1000);
  ASSERT_EQ(etaMod, 1000);
  ASSERT_EQ(phiMod, 1000);
  ASSERT_EQ(side, 1000);

}

TEST_F( SctSensorSDtest, AddHit )
{
  HepGeom::Point3D<double> lP1,lP2;
  lP1[SiHit::xEta] = 1;
  lP1[SiHit::xPhi] = 0;
  lP1[SiHit::xDep] = 0;
  lP2[SiHit::xEta] = 2;
  lP2[SiHit::xPhi] = 0;
  lP2[SiHit::xDep] = 0;
  
  G4DynamicParticle* dynamicPar = new G4DynamicParticle();
  G4double aValueTime = 1;
  G4ThreeVector ValuePosition(1.0, 1.0, 1.0);
  G4Track track(dynamicPar, aValueTime, ValuePosition);
  TrackHelper trHelp(&track);
  
  int brlEcap = 0;
  int layerDisk = 0;
  int etaMod = 0;
  int phiMod = 0;
  int side = 0;
  SctSensorSD sd4("name4", "name4");
  G4HCofThisEvent hce;
  sd4.Initialize( &hce );
  sd4.AddHit(lP1, lP2, 2.0, 1.0, trHelp.GetParticleLink(), 1, brlEcap, layerDisk, etaMod, phiMod, side);

  HepGeom::Point3D<double> P1,P2;
  P1[SiHit::xEta] = 1;
  P1[SiHit::xPhi] = 0;
  P1[SiHit::xDep] = 0;
  P2[SiHit::xEta] = 2;
  P2[SiHit::xPhi] = 0;
  P2[SiHit::xDep] = 0;
  
  int barcode = 0;
  HepMcParticleLink plink(barcode);
  
  SiHitCollection* a = sd4.m_HitColl.ptr();
  ASSERT_EQ(a->begin()->localStartPosition(), P1); //test the localStartPosition value of the Hit generated by the member function AddHit, the same below
  ASSERT_EQ(a->begin()->localEndPosition(), P2);
  ASSERT_EQ(a->begin()->energyLoss(), 2.0);
  ASSERT_EQ(a->begin()->meanTime(), 1.0);
  ASSERT_EQ(a->begin()->particleLink(), plink);
  ASSERT_EQ(a->begin()->identify(), 10245);
  ASSERT_EQ(a->size(), 1); //test the current size of the Hit container

}

int main( int argc, char** argv ) {

  auto g=new GaudiEnvironment;
  ::testing::AddGlobalTestEnvironment(g);
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();

}
