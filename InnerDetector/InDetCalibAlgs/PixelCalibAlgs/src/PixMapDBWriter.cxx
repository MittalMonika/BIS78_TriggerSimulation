/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// PixelCalibAlgs
#include "PixelCalibAlgs/PixMapDBWriter.h"
#include "PixelCalibAlgs/PixelConvert.h"

// PixelConditions
#include "PixelConditionsServices/ISpecialPixelMapSvc.h"
#include "PixelConditionsData/SpecialPixelMap.h"
//#include "PixelCoralClientUtils/SpecialPixelMap.hh" // kazuki

// geometry
#include "InDetIdentifier/PixelID.h"
#include "InDetReadoutGeometry/PixelDetectorManager.h" // kazuki
#include "InDetReadoutGeometry/SiDetectorElement.h" // kazuki
#include "InDetReadoutGeometry/PixelModuleDesign.h" // kazuki
#include "InDetReadoutGeometry/SiDetectorElementCollection.h" // kazuki
//#include "PixelGeoModel/IBLParameterSvc.h" // kazuki // this may not be necessary

// ROOT
#include "TFile.h"
#include "TKey.h"
#include "TDirectory.h"
#include "TH2.h"
#include "TString.h" // kazuki

// standard library
#include <vector> // kazuki
#include <map> // kazuki
#include <string> // kazuki
#include <algorithm> // kazuki
#include <fstream> // kazuki


PixMapDBWriter::PixMapDBWriter(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator),
  m_specialPixelMapSvc("SpecialPixelMapSvc", name),
  m_specialPixelMapKey(""),
  m_doValidate(false),
  m_calculateOccupancy(false),
  m_nBCReadout(1),
  m_pixelPropertyName("noise"),
  m_pixelStatus(32769), //default for noisy pixels
  m_listSpecialPixels(true),
  m_isIBL(true), // kazuki
  m_pixMapFileName("PixMap.root"),
  m_pixelID(0),
  m_pixman(0) // kazuki
{
  declareProperty("SpecialPixelMapKey", m_specialPixelMapKey, "Key of the map to be merged with/compared to the new pixel map");
  declareProperty("DoValidate", m_doValidate, "Switch for validation mode");
  declareProperty("CalculateOccupancy", m_calculateOccupancy, "Switch for calculation of occupancy");
  declareProperty("NBCReadout", m_nBCReadout, "Number of bunch crossings read out per event");
  declareProperty("PixelPropertyName", m_pixelPropertyName, "Name of the special property of pixels, beginning of directory name in input root file");
  declareProperty("PixelStatus", m_pixelStatus, "Pixel status used for validation/writing");
  declareProperty("ListSpecialPixels", m_listSpecialPixels, "Switch for printout of complete special pixels list");
  declareProperty("isIBL", m_isIBL, "If false IBL not considered"); // kazuki
  declareProperty("PixMapFileName", m_pixMapFileName, "File name of the ROOT file generated by PixMapBuilder");
}

PixMapDBWriter::~PixMapDBWriter(){}



StatusCode PixMapDBWriter::initialize(){

  ATH_MSG_INFO( "Initializing PixMapDBWriter" );

  StatusCode sc = m_specialPixelMapSvc.retrieve();
  if( !sc.isSuccess() ){
    ATH_MSG_FATAL( "Unable to retrieve SpecialPixelMapSvc" );
    return StatusCode::FAILURE;
  }

  sc = detStore()->retrieve( m_pixelID, "PixelID" );
  if( !sc.isSuccess() ){
    ATH_MSG_FATAL( "Unable to retrieve pixel ID helper" );
    return StatusCode::FAILURE;
  }

  // kazuki ==>
  sc = detStore()->retrieve( m_pixman, "Pixel" );
  if( !sc.isSuccess() ){
    ATH_MSG_FATAL( "Unable to retrieve pixel ID manager" );
    return StatusCode::FAILURE;
  }
  // <== //

  return StatusCode::SUCCESS;
}


StatusCode PixMapDBWriter::execute(){
  ATH_MSG_DEBUG( "Executing PixMapDBWriter" );

  return StatusCode::SUCCESS;
}


StatusCode PixMapDBWriter::finalize(){
  ATH_MSG_INFO( "Finalizing PixMapDBWriter" );

  ATH_MSG_DEBUG( "Reading map of special pixels from file" );

  TFile pixMapFile(m_pixMapFileName.c_str(), "READ");
  //std::map<std::string, std::map<unsigned int, TH2C*> > specialPixelHistograms;
  //std::map<std::string, std::map<unsigned int, TH2D*> > hitMaps;
  //std::map<std::string, std::map<std::string, TH2C*> > specialPixelHistograms; // kazuki
  std::map<std::string, TH2C*> specialPixelHistograms; // kazuki
  std::map<std::string, TH2D*> hitMaps; // kazuki
  //std::map<std::string, std::map<std::string, TH2D*> > hitMaps; // kazuki
  std::map<int, std::map<int, int> > ModulePixelidStatus; // kazuki

  // endcaps

  std::vector<std::string> directions;
  directions.push_back("A");
  directions.push_back("C");

  int nmodules=0, npixels=0;

  for(std::vector<std::string>::const_iterator direction = directions.begin(); direction != directions.end(); ++direction){

    std::string specialPixelHistogramsDirName = m_pixelPropertyName + std::string("Maps_endcap") + (*direction);

    //for(int k = 1; k <= 3; k ++)
    for(int layer = 1; layer <= 3; layer++)
    {

      std::ostringstream component, specialPixelHistogramsPath;

      component << "Disk" << layer << (*direction);
      specialPixelHistogramsPath << m_pixelPropertyName + "Maps_endcap" << (*direction) << "/Disk" << layer;

      //for(int j = 0; j < 48; j++)
      for(int index = 0; index < 48; index++) // loop in TDirectory
      {
        // get DCS ID
        std::string name((static_cast<TKey*>
              ((static_cast<TDirectory*>
                (pixMapFile.Get(specialPixelHistogramsPath.str().c_str())))->
               GetListOfKeys()->At(index)))->GetName());

        //int i = PixelConvert::HashID(PixelConvert::OnlineIDfromDCSID(name)); // kazuki comment out

        //specialPixelHistograms[component.str()][i] =
        //specialPixelHistograms[component.str()][name] = /////////////////////////////////////////////////////////////////
        specialPixelHistograms[name] = /////////////////////////////////////////////////////////////////
          static_cast<TH2C*>
          ((static_cast<TKey*>
            ((static_cast<TDirectory*>
              (pixMapFile.Get(specialPixelHistogramsPath.str().c_str())))
             ->GetListOfKeys()->At(index)))->ReadObj());
        //std::cout << "DEBUG: " << specialPixelHistograms[component.str()][index]->GetName() << std::endl;
        //std::cout << "DEBUG: " << specialPixelHistograms[name]->GetName() << std::endl;
      }

      if( m_calculateOccupancy ){

        std::ostringstream hitMapsPath;

        hitMapsPath << "hitMaps_endcap" << (*direction) << "/Disk" << layer;

        //for(int j = 0; j < 48; j++)
        for(int index = 0; index < 48; index++) // loop in TDirectory
        {
          std::string name((static_cast<TKey*>
                ((static_cast<TDirectory*>
                  (pixMapFile.Get(hitMapsPath.str().c_str())))->
                 GetListOfKeys()->At(index)))->GetName());

          //int i = PixelConvert::HashID(PixelConvert::OnlineIDfromDCSID(name)); // kazuki comment out

          //hitMaps[component.str()][i] =
          //hitMaps[component.str()][name] =
          hitMaps[name] =
            static_cast<TH2D*>
            ((static_cast<TKey*>
              ((static_cast<TDirectory*>
                (pixMapFile.Get(hitMapsPath.str().c_str())))
               ->GetListOfKeys()->At(index)))->ReadObj());
        //std::cout << "DEBUG: " << specialPixelHistograms[component.str()][index]->GetName() << std::endl;
        }
      } // end if (m_calculateOccupancy)
    } // end loop on layer
  } // end loop on Endcap direction


  // barrel

  std::vector<int> staves;
  if(m_isIBL) staves.push_back(14); // --- IBL --- //
  staves.push_back(22);
  staves.push_back(38);
  staves.push_back(52);

  std::vector<std::string> layers;
  if(m_isIBL) layers.push_back("IBL"); // --- IBL --- //
  layers.push_back("B-layer");
  layers.push_back("Layer1");
  layers.push_back("Layer2");


  for(unsigned int layer = 0; layer < layers.size(); layer++)
  {
    //for(unsigned int k = 0; k < staves.size(); k++)
    //for(unsigned int stave = 0; stave < staves.size(); stave++)
    //{

      std::ostringstream specialPixelHistogramsPath;

      specialPixelHistogramsPath << m_pixelPropertyName + "Maps_barrel/" << layers[layer];

      int nModulesPerStave = 13;
      if (m_isIBL && layer == 0) nModulesPerStave = 20; // --- IBL --- //
      for(int module = 0; module < staves[layer] * nModulesPerStave; module++) // loop on modules
      {
        std::string name((static_cast<TKey*>
              ((static_cast<TDirectory*>
                (pixMapFile.Get(specialPixelHistogramsPath.str().c_str())))->
               GetListOfKeys()->At(module)))->GetName());

        //int i = PixelConvert::HashID(PixelConvert::OnlineIDfromDCSID(name));

        //specialPixelHistograms[ layers[layer] ][module] = ////////////////////////////
        //specialPixelHistograms[ layers[layer] ][i] =
        //specialPixelHistograms[ layers[layer] ][name] =
        specialPixelHistograms[name] =
          static_cast<TH2C*>
          ((static_cast<TKey*>
            ((static_cast<TDirectory*>
              (pixMapFile.Get(specialPixelHistogramsPath.str().c_str())))
             ->GetListOfKeys()->At(module)))->ReadObj());
        //std::cout << specialPixelHistograms[ layers[layer] ][module]->GetName()  << std::endl;
        //std::cout << specialPixelHistograms[module]->GetName()  << std::endl;
        //std::cout << "DEBUG: " << specialPixelHistograms[name]->GetName()  << std::endl;
      }

      if( m_calculateOccupancy ){

        std::ostringstream hitMapsPath;

        hitMapsPath << "hitMaps_barrel/" << layers[layer];

        //for(int j = 0; j < (staves[k] * 13); j++)
        for(int module = 0; module < staves[layer] * nModulesPerStave; module++) // loop on modules
        {

          std::string name((static_cast<TKey*>
                ((static_cast<TDirectory*>
                  (pixMapFile.Get(hitMapsPath.str().c_str())))->
                 GetListOfKeys()->At(module)))->GetName());

          //int i = PixelConvert::HashID(PixelConvert::OnlineIDfromDCSID(name));

          //hitMaps[ layers[layer] ][module] =
          //hitMaps[ layers[layer] ][name] =
          hitMaps[name] =
            static_cast<TH2D*>
            ((static_cast<TKey*>
              ((static_cast<TDirectory*>
                (pixMapFile.Get(hitMapsPath.str().c_str())))
               ->GetListOfKeys()->At(module)))->ReadObj());
        } // end loop on modules
      } // end if ( m_calculateOccupancy )
    //} // end loop on staves
  } // end loop on layers

  ////////////////////////////////////////////////////////////////////////////////////////////
  // temporary repairs of PixelConvert::DCSID
  ////////////////////////////////////////////////////////////////////////////////////////////

//  std::string testarea = std::getenv("TestArea");
//  ifstream ifs(testarea + "/InstallArea/share/PixelMapping_Run2.dat");
  char* tmppath = std::getenv("DATAPATH");
  if(tmppath == NULL){
      ATH_MSG_FATAL( "Unable to retrieve environmental DATAPATH" );
      return StatusCode::FAILURE; 
  }
  std::string cmtpath(tmppath); 
  std::vector<std::string> paths = splitter(cmtpath, ':');
  std::ifstream ifs;
  for (const auto& x : paths){
    if(is_file_exist((x + "/PixelMapping_Run2.dat").c_str())){
      if(m_isIBL){
        ifs.open(x + "/PixelMapping_Run2.dat");
      } else {
        ifs.open(x + "/PixelMapping_May08.dat");
      }
      int tmp_barrel_ec; int tmp_layer; int tmp_module_phi; int tmp_module_eta; std::string tmp_module_name;
      std::vector<int> tmp_position;
      tmp_position.resize(4);
      //int counter = 0; // debug
      while(ifs >> tmp_barrel_ec >> tmp_layer >> tmp_module_phi >> tmp_module_eta >> tmp_module_name) {
        tmp_position[0] = tmp_barrel_ec;
        tmp_position[1] = tmp_layer;
        tmp_position[2] = tmp_module_phi;
        tmp_position[3] = tmp_module_eta;
        m_pixelMapping.push_back(std::make_pair(tmp_module_name, tmp_position));
      }
      break;
    }
  }

  //std::map<int, int> phi2M_ECA;
  //phi2M_ECA[0] = 1;
  //phi2M_ECA[1] = 6;
  //phi2M_ECA[2] = 2;
  //phi2M_ECA[3] = 5;
  //phi2M_ECA[4] = 3;
  //phi2M_ECA[5] = 4;
  //std::map<int, TString> phi2moduleID_ECA;
  //for(int phi = 0; phi < 48; phi++){
  //  phi = phi + 6;
  //  if (phi >= 48) phi = phi % 6;
  //  phi2moduleID_ECA[phi] = Form("B%02d_S%d_M%d",(phi / 12) + 1, (phi % 12 < 6) ? 1 : 2, phi2M_ECA[phi % 6]);
  //}
  //std::map<int, int> phi2M_ECC;
  //phi2M_ECC[0] = 4;
  //phi2M_ECC[1] = 3;
  //phi2M_ECC[2] = 5;
  //phi2M_ECC[3] = 2;
  //phi2M_ECC[4] = 6;
  //phi2M_ECC[5] = 1;
  //std::map<int, TString> phi2moduleID_ECC;
  //for(int phi = 0; phi < 48; phi++){
  //  phi = phi + 6;
  //  if (phi >= 48) phi = phi % 6;
  //  phi2moduleID_ECC[phi] = Form("B%02d_S%d_M%d",(phi / 12) + 1, (phi % 12 < 6) ? 1 : 2, phi2M_ECC[phi % 6]);
  //}
  //std::map<int, TString> eta2moduleID_IBL;
  //eta2moduleID_IBL[9] = TString("A_M4_A8_2");
  //eta2moduleID_IBL[8] = TString("A_M4_A8_1");
  //eta2moduleID_IBL[7] = TString("A_M4_A7_2");
  //eta2moduleID_IBL[6] = TString("A_M4_A7_1");
  //eta2moduleID_IBL[5] = TString("A_M3_A6");
  //eta2moduleID_IBL[4] = TString("A_M3_A5");
  //eta2moduleID_IBL[3] = TString("A_M2_A4");
  //eta2moduleID_IBL[2] = TString("A_M2_A3");
  //eta2moduleID_IBL[1] = TString("A_M1_A2");
  //eta2moduleID_IBL[0] = TString("A_M1_A1");
  //eta2moduleID_IBL[-1] = TString("C_M1_C1");
  //eta2moduleID_IBL[-2] = TString("C_M1_C2");
  //eta2moduleID_IBL[-3] = TString("C_M2_C3");
  //eta2moduleID_IBL[-4] = TString("C_M2_C4");
  //eta2moduleID_IBL[-5] = TString("C_M3_C5");
  //eta2moduleID_IBL[-6] = TString("C_M3_C6");
  //eta2moduleID_IBL[-7] = TString("C_M4_C7_1");
  //eta2moduleID_IBL[-8] = TString("C_M4_C7_2");
  //eta2moduleID_IBL[-9] = TString("C_M4_C8_1");
  //eta2moduleID_IBL[-10] = TString("C_M4_C8_2");
  ////
  //std::map<int, TString> eta2moduleID_PixelBarrel;
  //eta2moduleID_PixelBarrel[6] = TString("_M6A");
  //eta2moduleID_PixelBarrel[5] = TString("_M5A");
  //eta2moduleID_PixelBarrel[4] = TString("_M4A");
  //eta2moduleID_PixelBarrel[3] = TString("_M3A");
  //eta2moduleID_PixelBarrel[2] = TString("_M2A");
  //eta2moduleID_PixelBarrel[1] = TString("_M1A");
  //eta2moduleID_PixelBarrel[0] = TString("_M0");
  //eta2moduleID_PixelBarrel[-1] = TString("_M1C");
  //eta2moduleID_PixelBarrel[-2] = TString("_M2C");
  //eta2moduleID_PixelBarrel[-3] = TString("_M3C");
  //eta2moduleID_PixelBarrel[-4] = TString("_M4C");
  //eta2moduleID_PixelBarrel[-5] = TString("_M5C");
  //eta2moduleID_PixelBarrel[-6] = TString("_M6C");
  ////
  //std::map<int, TString> phi2moduleID_BLayer;
  //for(int phi = 0; phi < 22; phi++){
  //  if(phi == 0) {
  //    phi2moduleID_BLayer[phi] = TString("B11_S2");
  //  } else {
  //    phi2moduleID_BLayer[phi] = Form("B%02d_S%d",(phi + 1) / 2, (phi % 2) ? 1 : 2);
  //  }
  //}
  ////
  //std::map<int, TString> phi2moduleID_Layer1;
  //std::map<int, TString> phi2moduleID_Layer2;
  //for(int phi = 0; phi < 52; phi++){
  //  phi2moduleID_Layer1[phi] = Form("B%02d_S%d", (phi / 2) + 1, (phi % 2) ? 2 : 1);
  //  if (phi == 51) phi2moduleID_Layer2[phi] = TString("B01_S1");
  //  else phi2moduleID_Layer2[phi] = Form("B%02d_S%d", (phi + 1) / 2 + 1, (phi % 2) ? 1 : 2);
  //}

  ////////////////////////////////////////////////////////////////////////////////////////////
  // temporary repairs of PixelConvert::DCSID
  ////////////////////////////////////////////////////////////////////////////////////////////

  //DetectorSpecialPixelMap* spm = new DetectorSpecialPixelMap();
  auto spm = std::make_unique<DetectorSpecialPixelMap>();
  double nHitsBeforeMask = 0.;
  double nHitsAfterMask = 0.;

  spm->resize(m_pixelID->wafer_hash_max());

  //for(unsigned int i = 0; i < m_pixelID->wafer_hash_max(); i++){
  //  (*spm)[i] = new ModuleSpecialPixelMap();
  //}

  // kazuki
  InDetDD::SiDetectorElementCollection::const_iterator iter, itermin, itermax;
  itermin = m_pixman->getDetectorElementBegin();
  itermax = m_pixman->getDetectorElementEnd();
  int niterators=0; // kazuki
  if(m_pixelID->wafer_hash_max() > 1744) m_isIBL = true; // #modules only Pixel is 1744
  //std::cout << "DEBUG: wafer_hash_max = " << m_pixelID->wafer_hash_max() << std::endl;

  for( iter = itermin; iter != itermax; ++iter) {
    niterators++; // kazuki
    const InDetDD::SiDetectorElement* element = *iter;
    if(element == 0) continue;
    Identifier ident = element->identify();
    if(!m_pixelID->is_pixel(ident)) continue;  // OK this Element is included
    const InDetDD::PixelModuleDesign* design = dynamic_cast<const InDetDD::PixelModuleDesign*>(&element->design());
    if(!design) continue;
    unsigned int mchips = design->numberOfCircuits();
    int mrow =design->rows();
    int barrel     = m_pixelID->barrel_ec (ident);
    int layer      = m_pixelID->layer_disk(ident); // kazuki
    int module_phi = m_pixelID->phi_module(ident);
    int module_eta = m_pixelID->eta_module(ident); // kazuki
    int moduleHash = m_pixelID->wafer_hash(ident);
    int chipType = 1; // FE-I4
    if( mchips==8 || abs(barrel)==2 || (barrel==0 && layer > 0) ){ // if FE-I3
      mchips *=2; // guess numberOfCircuits()
      mrow /=2; // number of chips in row
      chipType = 0; // FE-I3
    }
    (*spm)[moduleHash] = new ModuleSpecialPixelMap();
    unsigned int chipsPerModule = mchips * 10 + chipType;
    (*spm)[moduleHash]->setchipsPerModule(chipsPerModule);
    TString onlineID;
    if (m_isIBL) { // --- IBL --- //
      onlineID = TString( PixMapDBWriter::getDCSIDFromPosition(barrel,layer,module_phi,module_eta ) );
      //if ( nspecial != 0 && m_listSpecialPixels ){ }
      //if ( barrel == 0 ) { // barrel
      //  if (layer == 0) {
      //    onlineID = Form("LI_S%02d_",module_phi + 1) + eta2moduleID_IBL[module_eta];
      //  } else if (layer == 1) {
      //    onlineID = Form("L%d_",layer - 1) + phi2moduleID_BLayer[module_phi] + eta2moduleID_PixelBarrel[module_eta];
      //  } else if (layer == 2) {
      //    onlineID = Form("L%d_",layer - 1) + phi2moduleID_Layer1[module_phi] + eta2moduleID_PixelBarrel[module_eta];
      //  } else if (layer == 3) {
      //    onlineID = Form("L%d_",layer - 1) + phi2moduleID_Layer2[module_phi] + eta2moduleID_PixelBarrel[module_eta];
      //  }
      //} else if ( barrel == 2 ) { // ECA
      //  onlineID = Form("D%dA_",layer + 1) + phi2moduleID_ECA[module_phi];
      //} else if ( barrel == -2) { // ECC
      //  onlineID = Form("D%dC_",layer + 1) + phi2moduleID_ECC[module_phi];
      //}
    }
    if ( specialPixelHistograms.find(std::string(onlineID)) == specialPixelHistograms.end() ) {
      std::cout << "DEBUG: onlineID " << onlineID << " is not found in the specialPixelHistograms" << std::endl;
      continue;
    }
    TH2C* specialPixelHistogram = specialPixelHistograms[std::string(onlineID)];
    //int nspecial = static_cast<int>( specialPixelHistogram->second->GetEntries() ); // #noisy pixels
    int nspecial = static_cast<int>( specialPixelHistogram->GetEntries() ); // #noisy pixels
    if( !m_calculateOccupancy ){
      if ( nspecial == 0 ) continue; // no noise hits
    }
    TH2D* hitMap = 0;

    if( m_calculateOccupancy ){
      hitMap = hitMaps[std::string(onlineID)];
      if( hitMap->GetEntries() == 0 ) continue;
    }

    nmodules++; // nmodules is total # of modules which contain noisy pixels
    npixels += nspecial; // npixels is total # of noisy pixels...

    double nHitsAfterMaskThisModule = 0.;

    //for(int pixel_eta = 0; pixel_eta <= m_pixelID->eta_index_max(moduleID); pixel_eta++)
    for(int pixel_eta = 0; pixel_eta <= m_pixelID->eta_index_max(ident); pixel_eta++)
    {
      //for(int pixel_phi = 0; pixel_phi <= m_pixelID->phi_index_max(moduleID); pixel_phi++)
      for(int pixel_phi = 0; pixel_phi <= m_pixelID->phi_index_max(ident); pixel_phi++)
      {

        bool special = false;

        //if( nspecial != 0 && specialPixelHistogram->second->GetBinContent(pixel_eta + 1, pixel_phi + 1) != 0 )
        if( nspecial != 0 && specialPixelHistogram->GetBinContent(pixel_eta + 1, pixel_phi + 1) != 0 )
        {
          special = true;
        }

        if( special ){ // noisy/dead pixel

          unsigned int pixelID =
            ModuleSpecialPixelMap::encodePixelID(barrel, module_phi, pixel_eta, pixel_phi, chipsPerModule); // m_chipPermodule=mchip*10+chip type
            //ModuleSpecialPixelMap::encodePixelID(barrel, module_phi, pixel_eta, pixel_phi, mchips); // mchips := #chips on module
            //ModuleSpecialPixelMap::encodePixelID(barrel, module_phi, pixel_eta, pixel_phi);

          //std::cout << "DEBUG: moduleHash = " << moduleHash << ", Identifier = " << ident << std::endl;
          //std::cout << "DEBUG: onlineID = " << onlineID << ", pixelID = " << pixelID << ", m_pixelStatus = " << m_pixelStatus << std::endl;
          (*spm)[moduleHash]->setPixelStatus(pixelID, m_pixelStatus);
          ModulePixelidStatus[moduleHash][pixelID] = m_pixelStatus;

          if( m_listSpecialPixels ){
            if (!(pixel_phi%1000)) { // thin out
              ATH_MSG_INFO( "moduleHash = " << moduleHash << " barrel_ec = " << barrel << " module_phi = " << module_phi << " pixel_eta = " << pixel_eta << " pixel_phi = " << pixel_phi << " mchips = " << mchips << " pixelID = " << pixelID ); // too much...
            }
          }
        } // end if special

        if( m_calculateOccupancy && !special ){

          nHitsAfterMaskThisModule += hitMap->GetBinContent( pixel_eta + 1, pixel_phi + 1 );
        }
      } // end loop on pixel_phi
    } // end loop on pixel_eta

    if( m_calculateOccupancy ){
      nHitsBeforeMask += hitMap->GetEntries();
      nHitsAfterMask += nHitsAfterMaskThisModule;
    }
    //std::cout << "#iterators = " << niterators << std::endl;
  } // end loop on element

  ATH_MSG_INFO( "Total of " << npixels << " masked pixels on " << nmodules << " modules" );

  spm->markSpecialRegions();
  spm->setNeighbourFlags();


  if(m_doValidate){
    DetectorSpecialPixelMap* spmFromDB = 0;

    StatusCode sc = detStore()->retrieve(spmFromDB, m_specialPixelMapKey);

    if( !sc.isSuccess() ){
      ATH_MSG_FATAL( "Unable to retrieve reference special pixel map" );
      return StatusCode::FAILURE;
    }

    int nMaskedPixels1 = 0; // this
    int nMaskedPixels2 = 0; // reference

    int nCommonMaskedPixels = 0;

    int nModulesWithMaskedPixels1 = 0;
    int nModulesWithMaskedPixels2 = 0;

    int nCommonModulesWithMaskedPixels = 0;

    std::vector<std::pair<unsigned int, Identifier> > results;
    std::vector<std::pair<unsigned int, Identifier> > results_absnum;

    for( iter = itermin; iter != itermax; ++iter){
      const InDetDD::SiDetectorElement* element = *iter;
      if(element == 0) continue;
      Identifier ident = element->identify();
      if(!m_pixelID->is_pixel(ident)) continue;  // OK this Element is included
      const InDetDD::PixelModuleDesign* design = dynamic_cast<const InDetDD::PixelModuleDesign*>(&element->design());
      if(!design) continue;
      unsigned int mchips = design->numberOfCircuits();
      int mrow =design->rows();
      int barrel     = m_pixelID->barrel_ec (ident);
      int layer      = m_pixelID->layer_disk(ident); // kazuki
      int module_phi = m_pixelID->phi_module(ident);
      int module_eta = m_pixelID->eta_module(ident); // kazuki
      int moduleHash = m_pixelID->wafer_hash(ident);
      int chipType = 1; // FE-I4
      if( mchips==8 || abs(barrel)==2 || (barrel==0 && layer > 0) ){ // if FE-I3
        mchips *=2; // guess numberOfCircuits()
        mrow /=2; // number of chips in row
        chipType = 0; // FE-I3
      }
      unsigned int chipsPerModule = mchips * 10 + chipType;

      //for(unsigned int moduleHash = 0; moduleHash < m_pixelID->wafer_hash_max(); moduleHash++)

      int nMaskedPixelsThis1 = 0;
      int nMaskedPixelsThis2 = 0;

      int nCommonMaskedPixelsThis = 0;

      //int moduleHash = m_pixelID->wafer_hash(ident);
      //std::cout << "DEBUG2: moduleHash = " << moduleHash << std::endl;

      //Identifier moduleID = m_pixelID->wafer_id(IdentifierHash(moduleHash));

      //int barrel     = m_pixelID->barrel_ec(moduleID);
      //int module_phi = m_pixelID->phi_module(moduleID);

      //for(int pixel_eta = 0; pixel_eta <= m_pixelID->eta_index_max(moduleID); pixel_eta++)
      for(int pixel_eta = 0; pixel_eta <= m_pixelID->eta_index_max(ident); pixel_eta++)
      {
        //for(int pixel_phi = 0; pixel_phi <= m_pixelID->phi_index_max(moduleID); pixel_phi++)
        for(int pixel_phi = 0; pixel_phi <= m_pixelID->phi_index_max(ident); pixel_phi++)
        {

          unsigned int pixelID =
            ModuleSpecialPixelMap::encodePixelID(barrel, module_phi, pixel_eta, pixel_phi, chipsPerModule);
            //ModuleSpecialPixelMap::encodePixelID(barrel, module_phi, pixel_eta, pixel_phi, mchips);
            //ModuleSpecialPixelMap::encodePixelID(barrel, module_phi, pixel_eta, pixel_phi);


          //unsigned int pixelStatus1 = (*spm)[moduleHash]->pixelStatus(pixelID);
          //unsigned int pixelStatus2 = (*spmFromDB)[moduleHash]->pixelStatus(pixelID);
          //PixelCoralClientUtils::ModuleSpecialPixelMap* mspm1 = (*spm)[moduleHash];
          //PixelCoralClientUtils::ModuleSpecialPixelMap* mspm2 = (*spmFromDB)[moduleHash];
          //unsigned int pixelStatus1 = (*mspm1)[pixelID];
          //unsigned int pixelStatus2 = (*mspm2)[pixelID];
          unsigned int pixelStatus1 = ModulePixelidStatus[moduleHash][pixelID];
          unsigned int pixelStatus2 = (*spmFromDB)[moduleHash]->pixelStatus(pixelID);

          //if (!(moduleHash%100) && !(pixel_eta%10) && !(pixel_phi%100))
          if (pixelStatus1 != 0)
          {
            //std::cout << "DEBUG: moduleHash = " << moduleHash << ", Identifier = " << ident << std::endl;
            //std::cout << "DEBUG: onlineID = " << onlineID << std::endl;
            std::cout << "barrel, layer, module_phi, module_eta, pixel_eta, pixel_phi, mchips, pixelID\n"
                      << " = " << barrel << ", " << layer << ", " << module_phi << ", " << module_eta
                      << ", " << pixel_eta << ", " << pixel_phi << ", " << mchips << ", " << pixelID << std::endl;
            std::cout << "pixelStatus1, pixelStatus2 = " << pixelStatus1 << ", " << pixelStatus2 << std::endl;
          }

          if( (pixelStatus1 & m_pixelStatus) == m_pixelStatus ){
            nMaskedPixels1++;
            nMaskedPixelsThis1++;
          }

          if( (pixelStatus2 & m_pixelStatus) == m_pixelStatus ){
            nMaskedPixels2++;
            nMaskedPixelsThis2++;
          }

          if( (pixelStatus2 & m_pixelStatus) == m_pixelStatus &&
              (pixelStatus1 & m_pixelStatus) == m_pixelStatus ){ // both this and ref have the same status
            nCommonMaskedPixels++;
            nCommonMaskedPixelsThis++;
          }
        }
      }

      if(nMaskedPixelsThis1 != 0) nModulesWithMaskedPixels1++;
      if(nMaskedPixelsThis2 != 0) nModulesWithMaskedPixels2++;
      if(nMaskedPixelsThis1 != 0 && nMaskedPixelsThis2 != 0) nCommonModulesWithMaskedPixels++;

      results.push_back(std::pair<unsigned int, Identifier>
          (std::abs(nMaskedPixelsThis1 - nMaskedPixelsThis2), ident));
          //(std::abs(nMaskedPixelsThis1 - nMaskedPixelsThis2), moduleID));
      results_absnum.push_back(std::pair<unsigned int, Identifier>
          (std::abs(nMaskedPixelsThis1), ident));
    }

    std::sort(results.rbegin(), results.rend());
    std::sort(results_absnum.rbegin(), results_absnum.rend());

    ATH_MSG_INFO("");

    msg().precision(1);
    msg().setf(std::ios::fixed);

    ATH_MSG_INFO("");
    ATH_MSG_INFO( "Number of modules with masked pixels:" );
    ATH_MSG_INFO( "Common " << nCommonModulesWithMaskedPixels
        << ", New map " << nModulesWithMaskedPixels1 << " ("
        << static_cast<double>(nCommonModulesWithMaskedPixels) / static_cast<double>(nModulesWithMaskedPixels1 + 0.00001) * 100. //Add epsilon to avoid potential FPE
        << "% common), Reference map " << nModulesWithMaskedPixels2 << " ("
        << static_cast<double>(nCommonModulesWithMaskedPixels) / static_cast<double>(nModulesWithMaskedPixels2 + 0.00001) * 100.
        << "% common)" );

    ATH_MSG_INFO( "Changes: -" << nModulesWithMaskedPixels2 - nCommonModulesWithMaskedPixels
        << " (-" << static_cast<double>(nModulesWithMaskedPixels2 - nCommonModulesWithMaskedPixels) /
        static_cast<double>(nModulesWithMaskedPixels2 + 0.00001) * 100. << "%), +"
        << nModulesWithMaskedPixels1 - nCommonModulesWithMaskedPixels
        << " (+" << static_cast<double>(nModulesWithMaskedPixels1 - nCommonModulesWithMaskedPixels) /
        static_cast<double>(nModulesWithMaskedPixels2 + 0.00001) * 100. << "%), total difference " << std::showpos
        << nModulesWithMaskedPixels1 - nModulesWithMaskedPixels2 << " ("
        << static_cast<double>(nModulesWithMaskedPixels1 - nModulesWithMaskedPixels2) /
        static_cast<double>(nModulesWithMaskedPixels2 + 0.00001) * 100. << "%)" << std::noshowpos );

    ATH_MSG_INFO("");
    ATH_MSG_INFO( "Number of masked pixels (ganged pixels enter twice):" );
    ATH_MSG_INFO( "Common " << nCommonMaskedPixels
        << ", New map " << nMaskedPixels1 << " ("
        << static_cast<double>(nCommonMaskedPixels) / static_cast<double>(nMaskedPixels1 + 0.00001) * 100. //Add epsilon to avoid potential FPE
        << "% common), Reference map " << nMaskedPixels2 << " ("
        << static_cast<double>(nCommonMaskedPixels) / static_cast<double>(nMaskedPixels2 + 0.00001) * 100.
        << "% common)" );


    ATH_MSG_INFO( "Changes: -" << nMaskedPixels2 - nCommonMaskedPixels << " (-"
        << static_cast<double>(nMaskedPixels2 - nCommonMaskedPixels) /
        static_cast<double>(nMaskedPixels2 + 0.00001) * 100. << "%), +"
        << nMaskedPixels1 - nCommonMaskedPixels << " (+"
        << static_cast<double>(nMaskedPixels1 - nCommonMaskedPixels) /
        static_cast<double>(nMaskedPixels2 + 0.00001) * 100. << "%), total difference " << std::showpos
        << nMaskedPixels1 - nMaskedPixels2 << " ("
        << static_cast<double>(nMaskedPixels1 - nMaskedPixels2) /
        static_cast<double>(nMaskedPixels2 + 0.00001) * 100. << "%)" << std::noshowpos );


    ATH_MSG_INFO("");
    ATH_MSG_INFO( "Modules with largest differences:" );
    ATH_MSG_INFO("");

    for(int i = 0; i < 20 ; i++){

      if ( m_isIBL ) {
        int barrel     = m_pixelID->barrel_ec(results[i].second);
        int layer      = m_pixelID->layer_disk(results[i].second); // kazuki
        int module_phi = m_pixelID->phi_module(results[i].second);
        int module_eta = m_pixelID->eta_module(results[i].second); // kazuki
        TString tmp_onlineID;
        tmp_onlineID = TString( PixMapDBWriter::getDCSIDFromPosition(barrel,layer,module_phi,module_eta ) );
        //if ( barrel == 0 ) { // barrel
        //  if (layer == 0) {
        //    tmp_onlineID = Form("LI_S%02d_",module_phi + 1) + eta2moduleID_IBL[module_eta];
        //  } else if (layer == 1) {
        //    tmp_onlineID = Form("L%d_",layer - 1) + phi2moduleID_BLayer[module_phi] + eta2moduleID_PixelBarrel[module_eta];
        //  } else if (layer == 2) {
        //    tmp_onlineID = Form("L%d_",layer - 1) + phi2moduleID_Layer1[module_phi] + eta2moduleID_PixelBarrel[module_eta];
        //  } else if (layer == 3) {
        //    tmp_onlineID = Form("L%d_",layer - 1) + phi2moduleID_Layer2[module_phi] + eta2moduleID_PixelBarrel[module_eta];
        //  }
        //} else if ( barrel == 2 ) { // ECA
        //  tmp_onlineID = Form("D%dA_",layer + 1) + phi2moduleID_ECA[module_phi];
        //} else if ( barrel == -2) { // ECC
        //  tmp_onlineID = Form("D%dC_",layer + 1) + phi2moduleID_ECC[module_phi];
        //}
        ATH_MSG_INFO( results[i].first << " pixels differing on module   \t" << results[i].second
            << " \t" << tmp_onlineID );
      } else {
        unsigned int hashID = ( ((m_pixelID->barrel_ec(results[i].second) + 2) / 2) << 25 ) +
          ( m_pixelID->layer_disk(results[i].second) << 23) +
          ( m_pixelID->phi_module(results[i].second) << 17) +
          ( (m_pixelID->eta_module(results[i].second) + 6) << 13);

        ATH_MSG_INFO( results[i].first << " pixels differing on module   \t" << results[i].second
            << " \t" << PixelConvert::DCSID(PixelConvert::OnlineID(hashID)) );
      }
    }

    ATH_MSG_INFO("");

    ATH_MSG_INFO("");
    ATH_MSG_INFO( "Modules with the largest number of noisy pixels:" );
    ATH_MSG_INFO("");

    for(int i = 0; i < 20 ; i++){

      if ( m_isIBL ) {
        int barrel     = m_pixelID->barrel_ec(results_absnum[i].second);
        int layer      = m_pixelID->layer_disk(results_absnum[i].second); // kazuki
        int module_phi = m_pixelID->phi_module(results_absnum[i].second);
        int module_eta = m_pixelID->eta_module(results_absnum[i].second); // kazuki
        TString tmp_onlineID;
        tmp_onlineID = TString( PixMapDBWriter::getDCSIDFromPosition(barrel,layer,module_phi,module_eta ) );
        //if ( barrel == 0 ) { // barrel
        //  if (layer == 0) {
        //    tmp_onlineID = Form("LI_S%02d_",module_phi + 1) + eta2moduleID_IBL[module_eta];
        //  } else if (layer == 1) {
        //    tmp_onlineID = Form("L%d_",layer - 1) + phi2moduleID_BLayer[module_phi] + eta2moduleID_PixelBarrel[module_eta];
        //  } else if (layer == 2) {
        //    tmp_onlineID = Form("L%d_",layer - 1) + phi2moduleID_Layer1[module_phi] + eta2moduleID_PixelBarrel[module_eta];
        //  } else if (layer == 3) {
        //    tmp_onlineID = Form("L%d_",layer - 1) + phi2moduleID_Layer2[module_phi] + eta2moduleID_PixelBarrel[module_eta];
        //  }
        //} else if ( barrel == 2 ) { // ECA
        //  tmp_onlineID = Form("D%dA_",layer + 1) + phi2moduleID_ECA[module_phi];
        //} else if ( barrel == -2) { // ECC
        //  tmp_onlineID = Form("D%dC_",layer + 1) + phi2moduleID_ECC[module_phi];
        //}
        ATH_MSG_INFO( results_absnum[i].first << " noisy pixels on module   \t" << results_absnum[i].second
            << " \t" << tmp_onlineID );
      } else {
        unsigned int hashID = ( ((m_pixelID->barrel_ec(results_absnum[i].second) + 2) / 2) << 25 ) +
          ( m_pixelID->layer_disk(results_absnum[i].second) << 23) +
          ( m_pixelID->phi_module(results_absnum[i].second) << 17) +
          ( (m_pixelID->eta_module(results_absnum[i].second) + 6) << 13);

        ATH_MSG_INFO( results_absnum[i].first << " noisy pixels on module   \t" << results_absnum[i].second
            << " \t" << PixelConvert::DCSID(PixelConvert::OnlineID(hashID)) );
      }
    }

    ATH_MSG_INFO("");
    pixMapFile.Close();
    
    return StatusCode::SUCCESS;
  } // end if dovalidate

  if( m_calculateOccupancy ){

    double nPixels = 80363520.;
    double nModules = 1744.;
    if (m_isIBL) {
      nPixels = 2880. * 16. * 1744. // Pixel
        + 26880. * 112. + 53760. * 168.  // IBL 3D + Planar
        + 26880. * 24; // DBM
      nModules = 2048.;
    }

    TH1D* disabledModules = 0;
    TH1D* nEvents = 0;

    pixMapFile.GetObject( "DisabledModules", disabledModules );
    pixMapFile.GetObject( "NEvents", nEvents );

    double nDisabledModules = 0.;

    for( int i = 1; i < nModules + 1; i++ ){
      nDisabledModules += disabledModules->GetBinContent(i);
    }

    nDisabledModules /= nEvents->GetBinContent(1);

    double nPixelsOnDisabledModules = nDisabledModules * nPixels / nModules;

    ATH_MSG_INFO("");

    ATH_MSG_INFO("Occupancy calculated for " << m_nBCReadout << " bc readout and an average of "
        << nDisabledModules << " disabled modules." );
    ATH_MSG_INFO( "Occupancy before masking of special pixels "
        << nHitsBeforeMask / (nPixels - nPixelsOnDisabledModules) /
        (nEvents->GetBinContent(1) * m_nBCReadout) << " +- "
        << std::sqrt(nHitsBeforeMask) / (nPixels - nPixelsOnDisabledModules) /
        (nEvents->GetBinContent(1) * m_nBCReadout) );
    ATH_MSG_INFO( "Occupancy after masking of special pixels "
        << nHitsAfterMask / (nPixels - nPixelsOnDisabledModules) /
        (nEvents->GetBinContent(1) * m_nBCReadout) << " +- "
        << std::sqrt(nHitsAfterMask) / (nPixels - nPixelsOnDisabledModules) /
        (nEvents->GetBinContent(1) * m_nBCReadout) );

    ATH_MSG_INFO("");
  } // end if m_calculateOccupancy

  if( !m_specialPixelMapKey.empty() ){

    DetectorSpecialPixelMap* spmFromDB = 0;

    StatusCode sc = detStore()->retrieve(spmFromDB, m_specialPixelMapKey);

    if( !sc.isSuccess() ){
      ATH_MSG_FATAL( "Unable to retrieve special pixel map from database" );
      return StatusCode::FAILURE;
    }

    ATH_MSG_INFO( "Merging special pixel map with reference map at " << m_specialPixelMapKey );

    spm->merge(spmFromDB);
  }

  if( !m_doValidate ){
    StatusCode sc = m_specialPixelMapSvc->registerCondAttrListCollection(spm.get());

    if( !sc.isSuccess() ){
      ATH_MSG_FATAL( "Unable to construct and record CondAttrListCollection" );
      return StatusCode::FAILURE;
    }
  }

  pixMapFile.Close();

  return StatusCode::SUCCESS;
}

std::string PixMapDBWriter::getDCSIDFromPosition (int barrel_ec, int layer, int module_phi, int module_eta){
  for(unsigned int ii = 0; ii < m_pixelMapping.size(); ii++) {
    if (m_pixelMapping[ii].second.size() != 4) {
      std::cout << "getDCSIDFromPosition: Vector size is not 4!" << std::endl;
      return std::string("Error!");
    }
    if (m_pixelMapping[ii].second[0] != barrel_ec) continue;
    if (m_pixelMapping[ii].second[1] != layer) continue;
    if (m_pixelMapping[ii].second[2] != module_phi) continue;
    if (m_pixelMapping[ii].second[3] != module_eta) continue;
    return m_pixelMapping[ii].first;
  }
  std::cout << "Not found!" << std::endl;
  return std::string("Error!");
}

std::vector<int> PixMapDBWriter::getPositionFromDCSID (std::string DCSID){
  for(unsigned int ii = 0; ii < m_pixelMapping.size(); ii++) {
    if (m_pixelMapping[ii].first == DCSID)
    return m_pixelMapping[ii].second;
  }
  std::cout << "Not found!" << std::endl;
  std::vector<int> void_vec;
  return void_vec;
}
