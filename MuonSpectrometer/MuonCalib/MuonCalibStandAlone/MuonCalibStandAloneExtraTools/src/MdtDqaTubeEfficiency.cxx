/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonCalibITools/IIdToFixedIdTool.h"
#include "MdtCalibInterfaces/IMdtSegmentFitter.h"

#include "MdtCalibUtils/GlobalTimeFitter.h"
#include "MuonCalibStandAloneExtraTools/HistogramManager.h"
#include "MuonCalibStandAloneExtraTools/PhiEtaUtils.h"
#include "MuonCalibEventBase/MuonCalibRawHitCollection.h"
#include "MuonCalibEventBase/MuonCalibRawMdtHit.h"
#include "MuonCalibEventBase/MuonCalibSegment.h"
#include "MdtCalibFitters/QuasianalyticLineReconstruction.h"
#include "MdtCalibFitters/DCSLFitter.h"
#include "MuonCalibEventBase/MuonCalibEvent.h"
#include "MuonCalibStandAloneExtraTools/MdtDqaTubeEfficiency.h"

#include "MuonCalibStandAloneBase/NtupleStationId.h"
#include "MuonCalibStandAloneBase/RegionSelectionSvc.h"
#include "MuonCalibStandAloneBase/MdtStationT0Container.h"

#include "MdtCalibData/IRtResolution.h"

#include "TFile.h"
#include "TH1.h"
#include "TNtuple.h"
#include "TString.h"
#include "TDirectory.h"

namespace MuonCalib {

//*****************************************************************************
// constructor
MdtDqaTubeEfficiency::MdtDqaTubeEfficiency(float nsigma, float chi2Cut,
                 bool defaultResol, float adcCut, bool GTFitON, 
                 bool useNewCalibConstants, bool useTimeCorrections) : 
  m_idHelper(nullptr),
  m_detMgr(nullptr),
  m_id_tool(nullptr),
  p_reg_sel_svc(nullptr),
  p_calib_input_svc(nullptr),
  m_histoManager(nullptr),
  m_qfitter(nullptr),
  m_nb_stations(-1)
{
  m_nsigma = nsigma;
  m_chi2Cut = chi2Cut;
  m_defaultResol = defaultResol; 
  m_adcCut = adcCut;
  m_GTFitON = GTFitON;
  m_useNewCalibConstants = useNewCalibConstants;
  m_useTimeCorrections = useTimeCorrections;
}
 
//:::::::::::::::::
//:: METHOD init ::
//:::::::::::::::::
StatusCode MdtDqaTubeEfficiency::initialize(const Muon::IMuonIdHelperSvc *idHelper, const MuonGM::MuonDetectorManager *detMgr, 
					    const MuonCalib::IIdToFixedIdTool *id_tool, RegionSelectionSvc *reg_sel_svc,
					    MdtCalibInputSvc *calib_input_svc, HistogramManager *histoManager) {
  m_idHelper = idHelper;
  m_detMgr = detMgr; 
  m_id_tool = id_tool;
  p_reg_sel_svc = reg_sel_svc;
  p_calib_input_svc = calib_input_svc;
  m_histoManager = histoManager;

  std::string RegionName = p_reg_sel_svc->GetRegionSelection();

  const std::vector<MuonCalib::NtupleStationId> stationsInRegion = p_reg_sel_svc->GetStationsInRegions();

  //----------------------------------//
  //-- Create Root Files and Histos --//
  //----------------------------------//
  
  m_qfitter = NULL;
    
  // loop over stations in region
  m_nb_stations = stationsInRegion.size();
  for (int istation=0;istation<m_nb_stations;istation++) {
    for (int k=0;k<4;k++) m_nb_layers_tubes[istation][k] = -1;
  }

  for (int istation=0;istation<m_nb_stations;istation++) {
    std::string stationNameString = stationsInRegion.at(istation).regionId();
    std::string chamberType = stationNameString.substr(0,3);
    int phi_id = stationsInRegion.at(istation).GetPhi();
    int eta_id = stationsInRegion.at(istation).GetEta();

    Identifier station_id = m_idHelper->mdtIdHelper().elementID(chamberType, eta_id, phi_id);
    int stationIntId = static_cast<int>(station_id.get_identifier32().get_compact());
    int numberOfML = m_idHelper->mdtIdHelper().numberOfMultilayers(station_id);
    
    for (int multilayer=1;multilayer<=numberOfML; multilayer++) {
      Identifier MdtML = m_idHelper->mdtIdHelper().multilayerID(station_id, multilayer);
      int layerMin = m_idHelper->mdtIdHelper().tubeLayerMin(MdtML);
      int layerMax = m_idHelper->mdtIdHelper().tubeLayerMax(MdtML);
      int tubeMin = m_idHelper->mdtIdHelper().tubeMin(MdtML);
      int tubeMax = m_idHelper->mdtIdHelper().tubeMax(MdtML);
      m_nb_layers_tubes[istation][0] = stationIntId;
      m_nb_layers_tubes[istation][1] = layerMax-layerMin+1;
      m_nb_layers_tubes[istation][1+multilayer] = tubeMax-tubeMin+1;
    }
  } //end loop on stations
  
  return StatusCode::SUCCESS;
}  //end MdtDqaTubeEfficiency::initialize

//*****************************************************************************

//::::::::::::::::::::::::
//:: METHOD handleEvent ::
//::::::::::::::::::::::::
StatusCode MdtDqaTubeEfficiency::handleEvent( const MuonCalibEvent &event, 
					      int /*eventnumber*/,
					      const std::vector<MuonCalibSegment *> &segments, 
					      unsigned int position) {

  bool RPCTimeCorrection = false;          // SHOULD BE SETVIA jobOption if useful in the future!
  bool t0RefinementTimeCorrection = true;  // SHOULD BE SET VIA jobOption if useful in the future!
  if( RPCTimeCorrection && t0RefinementTimeCorrection ) return StatusCode::FAILURE;

  if (segments.size()<=position) return StatusCode::SUCCESS;

  DCSLFitter *fitter = new DCSLFitter();
  GlobalTimeFitter *GTFitter = new GlobalTimeFitter(fitter);
  const IRtRelation *calibRt(0);
  const IRtRelation *GTFitRt(0);
  if ( m_GTFitON ) {
    if (!m_useNewCalibConstants ) {
      GTFitRt = GTFitter->getDefaultRtRelation();
      GTFitter->setRtRelation(GTFitRt);
    }
  } 
    
  for (unsigned int k=position; k<segments.size(); k++) {    // LOOP OVER SEGMENTS 

    MuonCalibSegment segment(*segments[k]);

    //---------------//
    //-- Variables --//
    //---------------//
    
    int nb_hits;
     
    // station identifiers //
  
    MuonFixedId Mid((segment.mdtHOT()[0])->identify());
    MDTName chamb(Mid);
    
    //this has to be set in order to get the m_detMgr->getMdtReadoutElement() method working correctly
    //otherwise also for the first multilayer, the second is returned
    //
    // M.I. ----> ???? TO CHECK FURTHER !
    Mid.setMdtMultilayer(1);

    //
    // Get REGION and STATION of the first hit of the segment :
    // 
    int phi  = Mid.phi();
    int eta  = Mid.eta();
    std::string stationNameStr = Mid.stationNameString();

    // 
    // Check that all the hits in the segment belongs to the same chamber :
    //
    bool segInOneChamber = true;
    for (unsigned int l=0; l<segment.mdtHitsOnTrack(); l++) {
      bool samestation( ((segment.mdtHOT()[l])->identify()).stationNameString()==stationNameStr );
      bool samephi( ((segment.mdtHOT()[l])->identify()).phi()==phi );
      bool sameeta( ((segment.mdtHOT()[l])->identify()).eta()==eta );
      bool sameChamber = samestation && samephi && sameeta;
      if (!sameChamber){
	segInOneChamber = false;
	// REINCLUDE THE BREAK !
	break;
      }
    }

    // WE SHOULD DECIDE IF A SEGMENT BUILT ON ADJACIENT CHAMBERS SHOULD BE SKIPPED
    // ...now skipped
    if (!segInOneChamber) continue;

    // 
    // Get numberOfMultiLayers, numberOfLayers, numberOfTubes :
    // 
    int stationIntId = static_cast<int>(m_idHelper->mdtIdHelper(). elementID(stationNameStr,eta,phi).get_compact());
    Identifier station_id = m_id_tool->fixedIdToId(Mid);
    
    int numberOfML, numberOfLayers, numberOfTubes[2];
    numberOfML = 0;
    numberOfLayers = 0;
    numberOfTubes[0] = 0;
    numberOfTubes[1] = 0;

    for (int ii=0; ii<m_nb_stations; ++ii) {
      if (m_nb_layers_tubes[ii][0] == stationIntId) {
	numberOfLayers = m_nb_layers_tubes[ii][1];
	numberOfTubes[0] = m_nb_layers_tubes[ii][2];
	numberOfTubes[1] = m_nb_layers_tubes[ii][3];
	if (numberOfTubes[0]>0 || numberOfTubes[1]>0 ) numberOfML = 1;
	if (numberOfTubes[0]>0 && numberOfTubes[1]>0 ) numberOfML = 2;
	break;
      }
    }

    if (numberOfML == 1 ) continue;  // GO TO NEXT SEGMENT

    int  minNumOfHits = numberOfLayers*2 - 1;

    if((int)segment.mdtHitsOnTrack()<minNumOfHits) continue;  // GO TO NEXT SEGMENT

    // Get Histograms
    TFile *mdtDqaRoot =  m_histoManager->rootFile();
    std::string region = chamb.getRegion();
    std::string side = chamb.getSide();

    PhiEtaNameConverter phiEtaConverter;
    std::string chamberType = chamb.getName();

    std::string chamberDirName = m_histoManager->GetMdtDirectoryName(chamb);
    std::string effiDirName = chamberDirName+"/Efficiency";
    std::string expertDirName = chamberDirName+"/Expert";
    
    TDirectory *chamberRootDir = mdtDqaRoot->GetDirectory(chamberDirName.c_str());
    TDirectory *effiRootDir = mdtDqaRoot->GetDirectory(effiDirName.c_str());
    TDirectory *expertRootDir = mdtDqaRoot->GetDirectory(expertDirName.c_str());
  
    if ( !chamberRootDir || !effiRootDir ) {
      delete GTFitter; GTFitter=0;
      return StatusCode::FAILURE;
    }

    std::string histoName;
    TH1F* heffiEntries;
    TH1F* heffiCounts;
    TH2F* heffiVsRadius;

    expertRootDir->cd();
    histoName = "EfficiencyEntries";
    heffiEntries = (TH1F*) expertRootDir->FindObjectAny(histoName.c_str());
    histoName = "EfficiencyCounts";
    heffiCounts = (TH1F*) expertRootDir->FindObjectAny(histoName.c_str());
    if (!heffiEntries || !heffiCounts ) {
      delete GTFitter; GTFitter=0;
      return StatusCode::FAILURE;
    }

    histoName = "EffiResidVsRadius";
    heffiVsRadius = (TH2F*) m_histoManager->GetMdtHisto(histoName,chamb);

    chamberRootDir->cd();
    float toffset(-9999.);
    float timeCorrection(-9999.);
    
    if( m_useTimeCorrections && RPCTimeCorrection ) timeCorrection = segment.mdtHOT()[0]->timeOfFlight();
    if( m_useTimeCorrections && t0RefinementTimeCorrection ) timeCorrection = segment.fittedT0();

    if ( m_GTFitON ) {
      MdtCalibHitBase *segHit = segment.mdtHOT()[0]; 
      MuonFixedId id(segHit->identify());
      if (m_useNewCalibConstants ) {
	calibRt = p_calib_input_svc->GetRtRelation(id);
	if (calibRt==NULL ) {
	  continue;
	}
	GTFitRt = calibRt;
	GTFitter->setRtRelation(GTFitRt);
	// here a method on GTFitter should be implemented to setResolution !
	// something like
      }
      
      toffset = GTFitter->GTFit(&segment);
      if ((int)segment.mdtHitsOnTrack() < minNumOfHits ) continue;

      // Recalibrate all rawhits on this chamber
      const MuonCalibRawHitCollection *raw_hits(event.rawHitCollection());
      for (MuonCalibRawHitCollection::MuonCalibRawMdtHitVecCit it=
             raw_hits->rawMdtHitCollectionBegin();
	   it!=raw_hits->rawMdtHitCollectionEnd(); ++it) {   // LOOP OVER RawHitCollection 
	
	if (GTFitRt==NULL) {
	  continue;
	}
	MuonCalibRawMdtHit *hit = *it;
	bool samestation( (hit->identify()).stationNameString()==stationNameStr );
	bool samephi( (hit->identify()).phi()==phi );
	bool sameeta( (hit->identify()).eta()==eta );
	bool sameChamber = samestation && samephi && sameeta;
	if (p_reg_sel_svc->isInRegion(hit->identify()) && sameChamber ){
	  int rawTime = hit->tdcCount();
	  double newDriftTime = (double)rawTime*25./32. - toffset;
	  double newRadius = GTFitRt->radius(newDriftTime);
	  double newResol(9999.);
	  if (m_defaultResol) newResol = defaultResolution(newRadius);
	  if (!m_defaultResol) newResol = GTFitter->getDefaultResolution(newRadius); //it is in fact the same
	  hit->setDriftTime(newDriftTime);
	  hit->setDriftRadius(newRadius);
	  hit->setDriftRadiusError(newResol);

	} // close IF the hit is in the same chamber
      } // END LOOP OVER RawHitCollection
    } //END IF GTFIT ON
    
    // RECALIBRATION with NEW CALIB CONSTANTS if GTFIT is OFF 
    if (m_useNewCalibConstants && !m_GTFitON ) {
      // Recalibrate all MdtCalibHitBase on the segment 
      // (in case calibrations used now are different from those of original segments)
      for (unsigned int l=0; l<segment.mdtHitsOnTrack(); l++) {   // LOOP OVER MdtCalibHitBase in the segment
        MdtCalibHitBase * segHit = segment.mdtHOT()[l]; 
        MuonFixedId id(segHit->identify());
        const MdtStationT0Container *t0=p_calib_input_svc->GetT0(id);
        const IRtRelation *rt_relation = p_calib_input_svc->GetRtRelation(id);
        const IRtResolution *spat_res  = p_calib_input_svc->GetResolution(id);
        if (t0==NULL || rt_relation==NULL || spat_res==NULL) {
	  continue;
        }

        unsigned short rawTime = segHit->tdcCount();
        double newDriftTime = (double)rawTime*25./32. - t0->t0(id.mdtMultilayer(),id.mdtTubeLayer(),id.mdtTube());
        if( m_useTimeCorrections ) newDriftTime = newDriftTime - timeCorrection;
        double newRadius = rt_relation->radius(newDriftTime);
        double newResol(9999.);
        if (!m_defaultResol) newResol = spat_res->resolution(newDriftTime);
        if (m_defaultResol) newResol = defaultResolution(newRadius);
        segHit->setDriftTime(newDriftTime);
        segHit->setDriftRadius(newRadius, newResol);
      } // END OVER MdtCalibHitBase in the segment

      // Recalibrate all rawhits on this chamber
      const MuonCalibRawHitCollection *raw_hits(event.rawHitCollection());
      for (MuonCalibRawHitCollection::MuonCalibRawMdtHitVecCit it=
	     raw_hits->rawMdtHitCollectionBegin();
	   it!=raw_hits->rawMdtHitCollectionEnd(); ++it) {   // LOOP OVER RawHitCollection 
	
	MuonCalibRawMdtHit *hit = *it;
	bool samestation( (hit->identify()).stationNameString()==stationNameStr );
	bool samephi( (hit->identify()).phi()==phi );
	bool sameeta( (hit->identify()).eta()==eta );
	bool sameChamber = samestation && samephi && sameeta;
	if (p_reg_sel_svc->isInRegion(hit->identify()) && sameChamber ) {
	  const MuonFixedId & id(hit->identify());
	  const MdtStationT0Container *t0=p_calib_input_svc->GetT0(id);
	  const IRtRelation *rt_relation = p_calib_input_svc->GetRtRelation(id);
	  const IRtResolution *spat_res  = p_calib_input_svc->GetResolution(id);

	  if (t0==NULL || rt_relation==NULL || spat_res==NULL) {
	    continue;
	  }

	  int rawTime = hit->tdcCount();
	  double newDriftTime = (double)rawTime*25./32. - t0->t0(id.mdtMultilayer(),id.mdtTubeLayer(),id.mdtTube());
	  newDriftTime = newDriftTime - timeCorrection;
	  double newRadius = rt_relation->radius(newDriftTime);
	  double newResol(9999.);
	  if (!m_defaultResol) newResol = spat_res->resolution(newDriftTime);
	  if (m_defaultResol) newResol = defaultResolution(newRadius);
	  hit->setDriftTime(newDriftTime);
	  hit->setDriftRadius(newRadius);
	  hit->setDriftRadiusError(newResol);

	} // close IF the hit is in the same chamber
      } // END LOOP OVER RawHitCollection
    } // CLOSE If m_useNewCalibConstants
    // END RECALIBRATION with NEW CALIB CONSTANTS 


    // Now everything is recalibrated and segments are ready.
    // Will start to remove one hit per layer
    MTStraightLine track0;
    track0 = MTStraightLine(segment.position(),segment.direction(),
			    Amg::Vector3D(0,0,0), Amg::Vector3D(0,0,0));

    // loop over MultiLayers
    for (int multilayer=1; multilayer<=numberOfML; multilayer++) {  // LOOP OVER MULTILAYERS
      
      const MuonGM::MdtReadoutElement *MdtRoEl = 
	m_detMgr->getMdtReadoutElement( m_idHelper->mdtIdHelper().channelID(station_id,multilayer,1,1) );
    
      //loop over layers
      for (int layer=1; layer<=numberOfLayers; layer++) {   // LOOP OVER LAYERS

	nb_hits = 0;
            
	// hit selection vector for refits: 
	// Exclude hit in the current layer 
	IMdtSegmentFitter::HitSelection 
	  hit_selection = IMdtSegmentFitter::HitSelection(segment.mdtHitsOnTrack());

	for (unsigned int l=0; l<segment.mdtHitsOnTrack(); l++) {
	  MuonFixedId id((segment.mdtHOT()[l])->identify());
	  if (id.mdtMultilayer() == multilayer &&
	      id.mdtTubeLayer()  == layer) {
	    hit_selection[l] = 1;
	  } else {
	    hit_selection[l] = 0;
	    nb_hits = nb_hits+1;
	  }
	}
	
	if (nb_hits<minNumOfHits)  continue;  // GO TO NEXT LAYER

	fitter->fit(segment,hit_selection);

	if ((int)segment.mdtHitsOnTrack() < minNumOfHits ) continue;
	if (segment.chi2()>m_chi2Cut)  continue;
	
	// counts the hits per Layers in the segment with the excluded layer
	int hit_ML_Ly[2][4]; 
	int totLayersWithHits = 0;
	for (int iml=0; iml<2; iml++) {
	  for (int il=0; il<4; il++) hit_ML_Ly[iml][il]= 0;
	} 
	
	for (unsigned int l=0; l<segment.mdtHitsOnTrack(); l++) {
	  MuonFixedId id((segment.mdtHOT()[l])->identify());
	  hit_ML_Ly[id.mdtMultilayer()-1][id.mdtTubeLayer()-1]++;
	}
	for (int iml=0; iml<2; iml++) {
	  for (int il=0; il<4; il++) if (hit_ML_Ly[iml][il]>0) totLayersWithHits++;
	} 
	if ( totLayersWithHits < minNumOfHits ) continue;
	
	// NOW WE HAVE A SELECTED SEGMENT WITH EXCLUDED HIT
	
	MTStraightLine track1;
	track1 = MTStraightLine(segment.position(),segment.direction(),
				Amg::Vector3D(0,0,0), Amg::Vector3D(0,0,0));
	
	std::vector<int> traversed_tube(0);
	std::vector<int> hit_tube(0);
	std::vector<int> hit_found(0);
	double distanceTraversedTube(0);
	
	// find tubes which have been traversed by the track //
	for (int k=0; k<numberOfTubes[multilayer-1]; k++) {
	  Amg::Vector3D TubePos = 
	    MdtRoEl->GlobalToAmdbLRSCoords(MdtRoEl->tubePos(multilayer,layer,k+1));
	  
	  Amg::Vector3D tube_position  = Amg::Vector3D(TubePos.x(), TubePos.y(), TubePos.z());
	  Amg::Vector3D tube_direction = Amg::Vector3D(1,0,0);
	  
	  MTStraightLine tube = MTStraightLine( tube_position, tube_direction,
						Amg::Vector3D(0,0,0), Amg::Vector3D(0,0,0) );

	  double distance = std::abs(track1.signDistFrom(tube));
	  
	  if ( distance < (MdtRoEl->innerTubeRadius()) ){
	    int traversedTube = k+1;
	    traversed_tube.push_back(k+1);
	    distanceTraversedTube = distance;   
	    
	    // TRAVERSED TUBE FOUND! NOW CHECK WHETHER THERE IS A HIT IN THIS TUBE FROM THE RAW HIT COLLECTION:
	    
	    bool hitFound = false;
	    const MuonCalibRawHitCollection *raw_hits(event.rawHitCollection());
	    for (MuonCalibRawHitCollection::MuonCalibRawMdtHitVecCit it=
		   raw_hits->rawMdtHitCollectionBegin();
		 it!=raw_hits->rawMdtHitCollectionEnd(); ++it) {   // LOOP OVER RawHitCollection 
	      
	      MuonCalibRawMdtHit *hit = *it;
	      bool samestation( (hit->identify()).stationNameString()==stationNameStr );
	      bool samephi( (hit->identify()).phi()==phi );
	      bool sameeta( (hit->identify()).eta()==eta );
	      bool sameChamber = samestation && samephi && sameeta;
	      if (p_reg_sel_svc->isInRegion(hit->identify()) && sameChamber &&
		  (hit->identify()).mdtMultilayer() == multilayer &&
		  (hit->identify()).mdtTubeLayer()  == layer) {

		if ( hit->adcCount() < m_adcCut )  continue;

		int tubeHit = (hit->identify()).mdtTube();

		if (tubeHit == traversedTube) {    // THE HIT IS FOUND 
		  // check if the same hit was already found
		  bool alreadyThere = false;
		  //loop over hit tubes
		  for (unsigned int j=0; j<hit_found.size(); j++) {
		    if (tubeHit==hit_found[j]){
		      alreadyThere=true;
		      break;
		    }
		  }

		  if (!alreadyThere) {    // A NEW HIT HAS BEEN FOUND
		    hitFound = true;
		    hit_found.push_back( tubeHit );
		    if ( m_nsigma < 0 ) hit_tube.push_back( tubeHit );
		    
		    Amg::Vector3D TubePos = 
		      MdtRoEl->GlobalToAmdbLRSCoords(MdtRoEl->tubePos(multilayer,layer,tubeHit));
	
		    Amg::Vector3D tube_position  = Amg::Vector3D(TubePos.x(), TubePos.y(), TubePos.z());
		    Amg::Vector3D tube_direction = Amg::Vector3D(1,0,0);
		    
		    MTStraightLine tube = MTStraightLine( tube_position, tube_direction,
							  Amg::Vector3D(0,0,0), Amg::Vector3D(0,0,0) );
                
		    double distance = std::abs(track1.signDistFrom(tube));
		    double hitRadius = std::abs(hit->driftRadius());
		    double resol = hit->driftRadiusError();
		    double resid = distance-hitRadius;
		    if(heffiVsRadius) heffiVsRadius->Fill(distance, resid);
		    float averageExtrapolError = 0.090; // ..an educated guess!
		    float sig = std::hypot(resol, averageExtrapolError);
		    
		    if ( m_nsigma>0. && std::abs(resid) < m_nsigma*sig ) hit_tube.push_back( tubeHit );
		  } // END NEW HIT FOUND
		} // close IF the Hit is found
	      } // close IF the hit is in the same chamber, same layer
	    } // END LOOP OVER RawHitCollection
	    if (!hitFound) if(heffiVsRadius) heffiVsRadius->Fill(distanceTraversedTube,15.5);
	    
	  } // Close IF Traversed Tube Found
	} // END LOOP OVER ALL TUBES IN THE LAYER
	
	// Efficiencies //

	//loop over traversed tubes
	for (unsigned int k=0; k<traversed_tube.size(); k++) {
	  int hit_flag = 0;
	  //loop over hit tubes
	  for (unsigned int j=0; j<hit_tube.size(); j++) {
	    if(traversed_tube[k]==hit_tube[j]){
	      hit_flag = 1;
	      break;
	    }
	  }
	  int offset=0;
	  if(multilayer==1) offset= m_histoManager->GetTubeOffsetML1(chamb.getOnlineName());
	  
	  int offset_atend=0;
	  if(multilayer==1) offset_atend= m_histoManager->GetTubeOffsetAtEndML1(chamb.getOnlineName());
          
	  float iTube = traversed_tube[k];
	  float ibin = (multilayer-1)*numberOfLayers*numberOfTubes[multilayer-1]+
	    (layer-1)*(numberOfTubes[multilayer-1]+offset+offset_atend)+iTube+offset;
	  heffiEntries->Fill(ibin);
	  if ( hit_flag) heffiCounts->Fill(ibin);
	}
      } //end of loop over layers
    } //end of loop over multilayers
    
  } // end LOOP OVER SEGMENTS

  delete fitter;
  delete GTFitter;

  return StatusCode::SUCCESS;
}  //end MdtDqaTubeEfficiency::handleEvent

//*****************************************************************************

//::::::::::::::::::::::::::::
//:: METHOD analyseSegments ::
//::::::::::::::::::::::::::::

StatusCode MdtDqaTubeEfficiency::analyseSegments(const std::vector<MuonCalibSegment *> & /*segments*/) {

  TFile *mdtDqaRoot =  m_histoManager->rootFile();
  const std::vector<MuonCalib::NtupleStationId> stationsInRegion = 
    p_reg_sel_svc->GetStationsInRegions();
  
  ToString ts;
  //loop over stations in region
  for ( int istation=0; istation<m_nb_stations; istation++ ) {
    int phi = stationsInRegion.at(istation).GetPhi();
    int eta = stationsInRegion.at(istation).GetEta();
    std::string stationNameString = stationsInRegion.at(istation).regionId();
    std::string chamberType = stationNameString.substr(0,3);
    
    MDTName chamb(chamberType,phi,eta);

    // In the following lines the numberOfML, numberOfLayers, numberOfTubes
    // are extracted RELYING on the order of the vector m_nb_layers_tubes[istation][]
    // with istation following the same order of stationsInRegion.at(istation)
    // ...if this is not the case, then the service m_mdtIdHelper should be used
    // matching the stationIntId :
    int numberOfML = 0;
    int numberOfTubes[2];
    int numberOfLayers = m_nb_layers_tubes[istation][1];
    numberOfTubes[0] = m_nb_layers_tubes[istation][2];
    numberOfTubes[1] = m_nb_layers_tubes[istation][3];
    if (numberOfTubes[0]>0 || numberOfTubes[1]>0 ) numberOfML = 1;
    if (numberOfTubes[0]>0 && numberOfTubes[1]>0 ) numberOfML = 2;
    
    std::string region= chamb.getRegion();
    std::string side=chamb.getSide();
    
    PhiEtaNameConverter phiEtaConverter;
    std::string chamberDirName = m_histoManager->GetMdtDirectoryName(chamb);
    
    std::string effiDirName = chamberDirName+"/Efficiency";
    std::string expertDirName = chamberDirName+"/Expert";
    TDirectory *chamberRootDir = mdtDqaRoot->GetDirectory(chamberDirName.c_str());
    TDirectory *effiRootDir = mdtDqaRoot->GetDirectory(effiDirName.c_str());
    TDirectory *expertRootDir = mdtDqaRoot->GetDirectory(expertDirName.c_str());
    
    if ( !chamberRootDir || !effiRootDir ) {
      return StatusCode::FAILURE;
    }
    
    std::string histoName;
    TH1F *heffiEntries;
    TH1F *heffiCounts;

    expertRootDir->cd();
    histoName = "EfficiencyEntries";
    heffiEntries = (TH1F*) expertRootDir->FindObjectAny(histoName.c_str());
    histoName = "EfficiencyCounts";
    heffiCounts = (TH1F*) expertRootDir->FindObjectAny(histoName.c_str());
    if (!heffiEntries || !heffiCounts ) {
      return StatusCode::FAILURE;
    }
    
    chamberRootDir->cd();
    TH1F *hg;
    histoName = "UNDEFINED";
    if (stationNameString.substr(1,1) == "I" ) histoName = "TubeEfficiency_Inner";
    if (stationNameString.substr(1,1) == "M" ) histoName = "TubeEfficiency_Middle";
    if (stationNameString.substr(1,1) == "O" ) histoName = "TubeEfficiency_Outer";
    hg = (TH1F*) m_histoManager->GetMdtHisto(histoName,region,side);

    // HERE NOW COMPUTE EFFICIENCY ERRORS AND FILL THE HISTOGRAMS

    if (heffiEntries->GetEntries() != 0 ) {
    //loop over multilayers
      for (int k=0; k<numberOfML; k++) {
	
	//loop over layers
	for (int l=0; l<numberOfLayers; l++) {
	    
            //loop over tubes
	  for (int m=0; m<numberOfTubes[k]; m++) {
	    int iML = k+1;
	    int iLy = l+1;
	    int iTube = m+1;
	    int ibin = (iML-1)*numberOfLayers*numberOfTubes[k]+(iLy-1)*numberOfTubes[k]+iTube;
	    //calculate efficiency and errors

	    // HERE WE USE THE Efficiency definition and Error using the Bayesian Statistics:
	    // 
	    float entries    = heffiEntries->GetBinContent(ibin);
	    float counts     = heffiCounts->GetBinContent(ibin);
	    float efficiency = (counts+1.)/(entries+2.);
	    float error      = std::sqrt(efficiency*(1-efficiency))/std::sqrt(entries+3.);
	    //
	    // Fill MdtDqa Histos
	    //

	    std::string histoName;
	    TH1F *heffi;
	    chamberRootDir->cd();
	    histoName = "b_EfficiencyPerTube";
	    heffi = (TH1F*) chamberRootDir->FindObjectAny(histoName.c_str());
	    if (!heffi) {
	      continue;
	    }
	    heffi->SetBinContent(ibin,efficiency);
	    heffi->SetBinError(ibin,error);
	    
	    histoName = "EffiPerTube_ML"+ts(iML)+"_L"+ts(iLy);
	    effiRootDir->cd();
	    heffi= (TH1F*) effiRootDir->FindObjectAny(histoName.c_str());
	    if (!heffi) {
	      continue;
	    }
	    heffi->SetBinContent(iTube,efficiency);
	    heffi->SetBinError(iTube,error);
	    
	    // Filling Global plots
	    if (hg && efficiency>0. && error >0. && error<0.05) hg->Fill(efficiency);
	    
	    // HERE WE MUST ADD THE EFFICIENCY PER MULTILAYER 
	    // and in case per chamber to put in a NEW overview plot!
	    // ...
	    //
	    
	  } // loop over tube
	} // loop over Layer 
      } // loop over ML
    } 
    
  } //LOOP on istation 

  return StatusCode::SUCCESS;
}  //end MdtDqaTubeEfficiency::analyseSegments

} // namespace MuonCalib 
