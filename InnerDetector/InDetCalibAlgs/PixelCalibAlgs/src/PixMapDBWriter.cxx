/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// PixelCalibAlgs
#include "PixelCalibAlgs/PixMapDBWriter.h"
#include "PixelCalibAlgs/PixelConvert.h"

// PixelConditions
#include "PixelConditionsServices/ISpecialPixelMapSvc.h"
#include "PixelConditionsData/SpecialPixelMap.h"

// geometry
#include "InDetIdentifier/PixelID.h"

// ROOT
#include "TFile.h"
#include "TKey.h"
#include "TDirectory.h"
#include "TH2.h"



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
  m_pixMapFileName("PixMap.root"),
  m_pixelID(0)
{
  declareProperty("SpecialPixelMapKey", m_specialPixelMapKey, "Key of the map to be merged with/compared to the new pixel map");
  declareProperty("DoValidate", m_doValidate, "Switch for validation mode");
  declareProperty("CalculateOccupancy", m_calculateOccupancy, "Switch for calculation of occupancy");
  declareProperty("NBCReadout", m_nBCReadout, "Number of bunch crossings read out per event");
  declareProperty("PixelPropertyName", m_pixelPropertyName, "Name of the special property of pixels, beginning of directory name in input root file");
  declareProperty("PixelStatus", m_pixelStatus, "Pixel status used for validation/writing");
  declareProperty("ListSpecialPixels", m_listSpecialPixels, "Switch for printout of complete special pixels list");
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

  std::map<std::string, std::map<unsigned int, TH2C*> > specialPixelHistograms;
  std::map<std::string, std::map<unsigned int, TH2D*> > hitMaps;

  // endcaps 

  std::vector<std::string> directions;
  directions.push_back("A");
  directions.push_back("C");
  
  int nmodules=0, npixels=0;

  for(std::vector<std::string>::const_iterator direction = directions.begin(); direction != directions.end(); ++direction){
  
    std::string specialPixelHistogramsDirName = m_pixelPropertyName + std::string("Maps_endcap") + (*direction);
    
    for(int k = 1; k <= 3; k ++){
      
      std::ostringstream component, specialPixelHistogramsPath;
      
      component << "Disk" << k << (*direction);
      specialPixelHistogramsPath << m_pixelPropertyName + "Maps_endcap" << (*direction) << "/Disk" << k;

      for(int j = 0; j < 48; j++){
	std::string name((static_cast<TKey*>
			  ((static_cast<TDirectory*>
			    (pixMapFile.Get(specialPixelHistogramsPath.str().c_str())))->
			   GetListOfKeys()->At(j)))->GetName());
	
	int i = PixelConvert::HashID(PixelConvert::OnlineIDfromDCSID(name));

	specialPixelHistograms[component.str()][i] = 
	  static_cast<TH2C*>
	  ((static_cast<TKey*>
	    ((static_cast<TDirectory*>
	      (pixMapFile.Get(specialPixelHistogramsPath.str().c_str())))
	     ->GetListOfKeys()->At(j)))->ReadObj());
      }

      if( m_calculateOccupancy ){
	
	std::ostringstream hitMapsPath;

	hitMapsPath << "hitMaps_endcap" << (*direction) << "/Disk" << k;
	
	for(int j = 0; j < 48; j++){
	  std::string name((static_cast<TKey*>
			    ((static_cast<TDirectory*>
			      (pixMapFile.Get(hitMapsPath.str().c_str())))->
			     GetListOfKeys()->At(j)))->GetName());
	  
	  int i = PixelConvert::HashID(PixelConvert::OnlineIDfromDCSID(name));

	  hitMaps[component.str()][i] = 
	    static_cast<TH2D*>
	    ((static_cast<TKey*>
	      ((static_cast<TDirectory*>
		(pixMapFile.Get(hitMapsPath.str().c_str())))
	       ->GetListOfKeys()->At(j)))->ReadObj());
	}
      }
    }
  }

  
  // barrel
  
  std::vector<int> staves;
  staves.push_back(22);
  staves.push_back(38);
  staves.push_back(52);

  std::vector<std::string> layers;
  layers.push_back("B-layer");
  layers.push_back("Layer1");
  layers.push_back("Layer2");
  

  for(unsigned int k = 0; k < staves.size(); k++){
    
    std::ostringstream specialPixelHistogramsPath;
    
    specialPixelHistogramsPath << m_pixelPropertyName + "Maps_barrel/" << layers[k];
    
    for(int j = 0; j < (staves[k] * 13); j++){
      
      std::string name((static_cast<TKey*>
			((static_cast<TDirectory*>
			  (pixMapFile.Get(specialPixelHistogramsPath.str().c_str())))->
			 GetListOfKeys()->At(j)))->GetName());

      int i = PixelConvert::HashID(PixelConvert::OnlineIDfromDCSID(name));

      specialPixelHistograms[layers[k]][i] = 
	static_cast<TH2C*>
	((static_cast<TKey*>
	  ((static_cast<TDirectory*>
	    (pixMapFile.Get(specialPixelHistogramsPath.str().c_str())))
	   ->GetListOfKeys()->At(j)))->ReadObj());
    }

    if( m_calculateOccupancy ){
      
      std::ostringstream hitMapsPath;

      hitMapsPath << "hitMaps_barrel/" << layers[k];
      
      for(int j = 0; j < (staves[k] * 13); j++){
	
	std::string name((static_cast<TKey*>
			  ((static_cast<TDirectory*>
			    (pixMapFile.Get(hitMapsPath.str().c_str())))->
			   GetListOfKeys()->At(j)))->GetName());
	
	int i = PixelConvert::HashID(PixelConvert::OnlineIDfromDCSID(name));
	
	hitMaps[layers[k]][i] = 
	  static_cast<TH2D*>
	  ((static_cast<TKey*>
	    ((static_cast<TDirectory*>
	      (pixMapFile.Get(hitMapsPath.str().c_str())))
	     ->GetListOfKeys()->At(j)))->ReadObj());
      }
    }
  }
  
  auto spm = std::make_unique<DetectorSpecialPixelMap>();
  double nHitsBeforeMask = 0.;
  double nHitsAfterMask = 0.;

  spm->resize(m_pixelID->wafer_hash_max());
  
  for(unsigned int i = 0; i < m_pixelID->wafer_hash_max(); i++){
    (*spm)[i] = new ModuleSpecialPixelMap();
  }   

  for(std::map<std::string, std::map<unsigned int, TH2C*> >::const_iterator component = specialPixelHistograms.begin();
      component != specialPixelHistograms.end(); ++component){
    for(std::map<unsigned int, TH2C*>::const_iterator specialPixelHistogram = (*component).second.begin();
	specialPixelHistogram != (*component).second.end(); ++specialPixelHistogram){
      
      int nspecial = static_cast<int>( specialPixelHistogram->second->GetEntries() );
      if( !m_calculateOccupancy ){
	if ( nspecial == 0 ){
	  continue; // no noise hits
	}
      }

      TH2D* hitMap = 0;

      if( m_calculateOccupancy ){
	hitMap = hitMaps[component->first][specialPixelHistogram->first];

	if( hitMap->GetEntries() == 0 ){
	  continue;
	}
      }
      
      Identifier moduleID(specialPixelHistogram->first);
      IdentifierHash moduleHash = m_pixelID->wafer_hash(moduleID);
      
      int barrel     = m_pixelID->barrel_ec(moduleID);
      int module_phi = m_pixelID->phi_module(moduleID);

      unsigned int hashID = ( ((m_pixelID->barrel_ec(moduleID) + 2) / 2) << 25 ) +
	( m_pixelID->layer_disk(moduleID) << 23) +
	( m_pixelID->phi_module(moduleID) << 17) +
	( (m_pixelID->eta_module(moduleID) + 6) << 13);

      if ( nspecial != 0 && m_listSpecialPixels ){
	ATH_MSG_INFO( "Module " << PixelConvert::DCSID(PixelConvert::OnlineID(hashID))
		      << " has " << nspecial << " special pixels." );
      }
      
      nmodules++;
      npixels += nspecial;
      
      double nHitsAfterMaskThisModule = 0.;

      for(int pixel_eta = 0; pixel_eta <= m_pixelID->eta_index_max(moduleID); pixel_eta++){
	for(int pixel_phi = 0; pixel_phi <= m_pixelID->phi_index_max(moduleID); pixel_phi++){

	  bool special = false;

	  if( nspecial != 0 && specialPixelHistogram->second->GetBinContent(pixel_eta + 1, pixel_phi + 1) != 0 ){
	    special = true;
	  }

	  if( special ){

	    unsigned int pixelID = 
	      ModuleSpecialPixelMap::encodePixelID(barrel, module_phi, pixel_eta, pixel_phi);

	    (*spm)[moduleHash]->setPixelStatus(pixelID, m_pixelStatus);

	    if( m_listSpecialPixels ){
	      ATH_MSG_INFO( "  eta_index = " << pixel_eta << " phi_index = " << pixel_phi );
	    }
	  }

	  if( m_calculateOccupancy && !special ){

	    nHitsAfterMaskThisModule += hitMap->GetBinContent( pixel_eta + 1, pixel_phi + 1 );
	  } 
	}
      }

      if( m_calculateOccupancy ){
	nHitsBeforeMask += hitMap->GetEntries();
	nHitsAfterMask += nHitsAfterMaskThisModule;
      }
    }
  }
  
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
    
    int nMaskedPixels1 = 0;
    int nMaskedPixels2 = 0;
    
    int nCommonMaskedPixels = 0;
    
    int nModulesWithMaskedPixels1 = 0;
    int nModulesWithMaskedPixels2 = 0;
    
    int nCommonModulesWithMaskedPixels = 0;
    
    std::vector<std::pair<unsigned int, Identifier> > results;
    
    for(unsigned int moduleHash=0; moduleHash < m_pixelID->wafer_hash_max(); moduleHash++){
      
      int nMaskedPixelsThis1 = 0;
      int nMaskedPixelsThis2 = 0;
      
      int nCommonMaskedPixelsThis = 0;
      
      Identifier moduleID = m_pixelID->wafer_id(IdentifierHash(moduleHash));
      
      int barrel     = m_pixelID->barrel_ec(moduleID);
      int module_phi = m_pixelID->phi_module(moduleID);
      
      for(int pixel_eta = 0; pixel_eta <= m_pixelID->eta_index_max(moduleID); pixel_eta++){
	for(int pixel_phi = 0; pixel_phi <= m_pixelID->phi_index_max(moduleID); pixel_phi++){
	  
	  unsigned int pixelID = 
	    ModuleSpecialPixelMap::encodePixelID(barrel, module_phi, pixel_eta, pixel_phi);
	  
	  unsigned int pixelStatus1 = (*spm)[moduleHash]->pixelStatus(pixelID);
	  unsigned int pixelStatus2 = (*spmFromDB)[moduleHash]->pixelStatus(pixelID);
	  
	  if( (pixelStatus1 & m_pixelStatus) == m_pixelStatus ){
	    nMaskedPixels1++;
	    nMaskedPixelsThis1++;
	  }
	  
	  if( (pixelStatus2 & m_pixelStatus) == m_pixelStatus ){
	    nMaskedPixels2++;
	    nMaskedPixelsThis2++;
	  }
	  
	  if( (pixelStatus2 & m_pixelStatus) == m_pixelStatus &&
	    (pixelStatus1 & m_pixelStatus) == m_pixelStatus ){
	    nCommonMaskedPixels++;
	    nCommonMaskedPixelsThis++;
	  }
	}
      }
      
      if(nMaskedPixelsThis1 != 0) nModulesWithMaskedPixels1++;
      if(nMaskedPixelsThis2 != 0) nModulesWithMaskedPixels2++;
      if(nMaskedPixelsThis1 != 0 && nMaskedPixelsThis2 != 0) nCommonModulesWithMaskedPixels++;
      
      results.push_back(std::pair<unsigned int, Identifier>
			(std::abs(nMaskedPixelsThis1 - nMaskedPixelsThis2), moduleID));
    }
    
    std::sort(results.rbegin(), results.rend());
    
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
		  static_cast<double>(nModulesWithMaskedPixels2) * 100. << "%), +"
		  << nModulesWithMaskedPixels1 - nCommonModulesWithMaskedPixels
		  << " (+" << static_cast<double>(nModulesWithMaskedPixels1 - nCommonModulesWithMaskedPixels) /
		  static_cast<double>(nModulesWithMaskedPixels2) * 100. << "%), total difference " << std::showpos
		  << nModulesWithMaskedPixels1 - nModulesWithMaskedPixels2 << " ("
		  << static_cast<double>(nModulesWithMaskedPixels1 - nModulesWithMaskedPixels2) /
		  static_cast<double>(nModulesWithMaskedPixels2) * 100. << "%)" << std::noshowpos );
    
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
		  static_cast<double>(nMaskedPixels2) * 100. << "%), +"
		  << nMaskedPixels1 - nCommonMaskedPixels << " (+" 
		  << static_cast<double>(nMaskedPixels1 - nCommonMaskedPixels) /
		  static_cast<double>(nMaskedPixels2) * 100. << "%), total difference " << std::showpos 
		  << nMaskedPixels1 - nMaskedPixels2 << " ("
		  << static_cast<double>(nMaskedPixels1 - nMaskedPixels2) /
		  static_cast<double>(nMaskedPixels2) * 100. << "%)" << std::noshowpos );
    

    ATH_MSG_INFO("");
    ATH_MSG_INFO( "Modules with largest differences:" );
    ATH_MSG_INFO("");
    
    for(int i = 0; i < 20 ; i++){

      unsigned int hashID = ( ((m_pixelID->barrel_ec(results[i].second) + 2) / 2) << 25 ) +
	( m_pixelID->layer_disk(results[i].second) << 23) +
	( m_pixelID->phi_module(results[i].second) << 17) +
	( (m_pixelID->eta_module(results[i].second) + 6) << 13);

      ATH_MSG_INFO( results[i].first << " pixels differing on module   \t" << results[i].second 
		    << " \t" << PixelConvert::DCSID(PixelConvert::OnlineID(hashID)) );
    }

    ATH_MSG_INFO("");

    pixMapFile.Close();
    
    return StatusCode::SUCCESS;
  }
  
  if( m_calculateOccupancy ){

    double nPixels = 80363520.;
    double nModules = 1744.;

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
  }

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
