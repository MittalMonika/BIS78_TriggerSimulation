/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// RALHec
// Sep 5 JFB

// Fetch parameters from a NOVA database structure.

#include "LArGeoRAL/RALHec.h"
#include "LArGeoCode/VDetectorParameters.h"

// Athena Service classes
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "StoreGate/StoreGateSvc.h"

#include "CLHEP/Units/PhysicalConstants.h"

#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"

#include "GeoModelInterfaces/IGeoModelSvc.h"

#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdexcept>

class LArGeo::RALHec::Clockwork {

public:

  Clockwork() {}
  ~Clockwork() {}
  
  IRDBRecordset_ptr hadronicEndcap;
  IRDBRecordset_ptr hecGridValues;
  IRDBRecordset_ptr hecLongitudinalBlock;
};


LArGeo::RALHec::RALHec():
  c(new Clockwork())
{
  // First, fetch the Athena services.
  ISvcLocator* svcLocator = Gaudi::svcLocator(); // from Bootstrap.h


  StatusCode sc;
  IRDBAccessSvc *pAccessSvc;
  sc=svcLocator->service("RDBAccessSvc",pAccessSvc);
  if (sc != StatusCode::SUCCESS) {
    throw std::runtime_error ("Cannot locate RDBAccessSvc!!");
  }


  IGeoModelSvc *geoModel;
  sc = svcLocator->service ("GeoModelSvc",geoModel);
  if (sc != StatusCode::SUCCESS) {
    throw std::runtime_error ("Cannot locate GeoModelSvc!!");
  }
  
  std::string AtlasVersion = geoModel->atlasVersion();
  std::string LArVersion = geoModel->LAr_VersionOverride();

  std::string detectorKey  = LArVersion.empty() ? AtlasVersion : LArVersion;
  std::string detectorNode = LArVersion.empty() ? "ATLAS" : "LAr";



  c->hadronicEndcap             = pAccessSvc->getRecordsetPtr("HadronicEndcap",detectorKey, detectorNode); 
  c->hecGridValues              = pAccessSvc->getRecordsetPtr("HecGridValues",detectorKey, detectorNode); 
  c->hecLongitudinalBlock       = pAccessSvc->getRecordsetPtr("HecLongitudinalBlock",detectorKey, detectorNode); 

}


LArGeo::RALHec::~RALHec()
{
  delete c;
}


double LArGeo::RALHec::GetValue(const std::string& a_name,
                                const int a0,
                                const int /*a1*/,
                                const int /*a2*/,
                                const int /*a3*/,
                                const int /*a4*/ ) 
{
  
  std::ostringstream A0STR;
  A0STR << "_" << a0;
  const std::string A0 = A0STR.str();


  if ( a_name == "LArHECmoduleNumber"  ) return (*c->hadronicEndcap)[0]->getInt("NSCT");
  if ( a_name == "LArHECzStart"        ) return (*c->hadronicEndcap)[0]->getDouble("ZSTART")*CLHEP::cm;
  if ( a_name == "LArHECgapSize"       ) return (*c->hadronicEndcap)[0]->getDouble("LARG")*CLHEP::cm;
  if ( a_name == "LArHECbetweenWheel"  ) return (*c->hadronicEndcap)[0]->getDouble("GAPWHL")*CLHEP::cm;
  if ( a_name == "LArHECcopperPad"     ) return (*c->hadronicEndcap)[0]->getDouble("COPPER")*CLHEP::cm;
  if ( a_name == "LArHECplateWidth0"   ) return (*c->hadronicEndcap)[0]->getDouble("PLATE_0")*CLHEP::cm;
  if ( a_name == "LArHECplateWidth1"   ) return (*c->hadronicEndcap)[0]->getDouble("PLATE_1")*CLHEP::cm;
  if ( a_name == "LArHECkaptonPosition") return (*c->hadronicEndcap)[0]->getDouble("KPTPOS"+A0)*CLHEP::cm;
  if ( a_name == "LArHECkaptonWidth"   ) return (*c->hadronicEndcap)[0]->getDouble("KPTWID"+A0)*CLHEP::cm;
  if ( a_name == "LArHECrodPosY"       ) return (*c->hadronicEndcap)[0]->getDouble("RODPOSR"+A0)*CLHEP::cm;
  if ( a_name == "LArHECrodPosX"       ) return (*c->hadronicEndcap)[0]->getDouble("RODPOSX"+A0)*CLHEP::cm;
  if ( a_name == "LArHECrodDim"        ) return (*c->hadronicEndcap)[0]->getDouble("RODDIM"+A0)*CLHEP::cm;
  if ( a_name == "LArHECspcDim"        ) return (*c->hadronicEndcap)[0]->getDouble("SPCDIM"+A0)*CLHEP::cm;

  if ( a_name == "LArHECsliceIndex"    ) return (*c->hecLongitudinalBlock)[a0]->getDouble("IBLC");
  if ( a_name == "LArHECmoduleRinner1" ) return (*c->hecLongitudinalBlock)[0]->getDouble("BLRMN")*CLHEP::cm;
  if ( a_name == "LArHECmoduleRinner2" ) return (*c->hecLongitudinalBlock)[1]->getDouble("BLRMN")*CLHEP::cm;
  if ( a_name == "LArHECmoduleRouter"  ) return (*c->hecLongitudinalBlock)[0]->getDouble("BLRMX")*CLHEP::cm;
  if ( a_name == "LArHECdepthZ"        ) return (*c->hecLongitudinalBlock)[a0]->getDouble("BLDPTH")*CLHEP::cm;
  if ( a_name == "LArHECfirstAbsorber" ) return (*c->hecLongitudinalBlock)[a0]->getDouble("PLATE0")*CLHEP::cm;
  if ( a_name == "LArHECgapNumber"     ) return (*c->hecLongitudinalBlock)[a0]->getDouble("BLMOD");

  // We didn't find a match.  
  std::string errMessage = "RALHec::GetValue: could not find a match for the key '" + a_name;
  std::cerr << errMessage << std::endl;
  throw std::runtime_error (errMessage.c_str());

  // This is a dummy statement to prevent a compiler warning. 
  return 0.;
}
