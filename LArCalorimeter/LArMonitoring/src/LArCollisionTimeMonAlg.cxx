/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// ********************************************************************
//
// NAME:     LArCollisionTimeMonAlg.cxx
// PACKAGE:  LArMonitoring
//
// AUTHOR:   Margherita Spalla (migrated from LArCollisionTimeMonTool by Vikas Bansal)
//
// Class for monitoring : EC timing from LArCollisionTime Container 

//
// ********************************************************************



#include "LArCollisionTimeMonAlg.h"


#include "TrigAnalysisInterfaces/IBunchCrossingTool.h"


using namespace std;




/*---------------------------------------------------------*/
LArCollisionTimeMonAlg::LArCollisionTimeMonAlg( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
  ,m_timeCut(5.0)
  ,m_minCells(2)
  ,m_eWeighted(true)
  ,m_LArCollisionTimeKey("LArCollisionTime")
  ,m_bunchCrossingTool("BunchCrossingTool")
{
  declareProperty( "Key"                ,      m_LArCollisionTimeKey);
  declareProperty( "BunchCrossingTool"  ,      m_bunchCrossingTool); 
}

/*---------------------------------------------------------*/
LArCollisionTimeMonAlg::~LArCollisionTimeMonAlg()
{}

/*---------------------------------------------------------*/
StatusCode 
LArCollisionTimeMonAlg::initialize() {

  //init handlers
  ATH_CHECK( m_LArCollisionTimeKey.initialize() );

  return AthMonitorAlgorithm::initialize();
}

/*---------------------------------------------------------*/
/*
TO BE MOVED IN PYTHON
  MonGroup generalGroupShift( this, "/LAr/"+m_histPath+"/", run, ATTRIB_MANAGED);
  MonGroup generalGroupLB( this, "/LAr/"+m_histPath+"/", run, ATTRIB_X_VS_LB, "", "merge");

*/

/*---------------------------------------------------------*/
StatusCode 
LArCollisionTimeMonAlg::fillHistograms( const EventContext& ctx ) const
{
  using namespace Monitored;

  ATH_MSG_DEBUG( "in fillHists()" );

  //monitored variables
  auto ecTimeDiff = Monitored::Scalar<float>("ecTimeDiff",0.0);
  auto ecTimeAvg = Monitored::Scalar<float>("ecTimeAvg",0.0);
  auto lumi_block = Monitored::Scalar<unsigned int>("lumi_block",0);
  auto lumi_block_timeWindow = Monitored::Scalar<float>("lumi_block_timeWindow",0);
  auto lumi_block_singleBeam_timeWindow = Monitored::Scalar<float>("lumi_block_singleBeam_timeWindow",0);
  auto bunch_crossing_id = Monitored::Scalar<unsigned int>("bunch_crossing_id",0);
  auto weight = Monitored::Scalar<float>("weight",1.);

  // --- retrieve event information ---
  auto event_info = GetEventInfo(ctx);

  // bunch crossing ID:
  bunch_crossing_id = event_info->bcid();

  // luminosity block number
  lumi_block = event_info->lumiBlock();


  if(m_bunchCrossingTool->bcType(bunch_crossing_id) == Trig::IBunchCrossingTool::Empty) {
    ATH_MSG_INFO("BCID: "<<bunch_crossing_id<<" empty ? not filling the coll. time" );
    return StatusCode::SUCCESS; // not filling anything in empty bunches
  }
  
  int bcid_distance = m_bunchCrossingTool->distanceFromFront(bunch_crossing_id, Trig::IBunchCrossingTool::BunchCrossings);
  ATH_MSG_DEBUG("BCID distance: "<<bcid_distance );

  // Retrieve LArCollision Timing information
  SG::ReadHandle<LArCollisionTime> larTime(m_LArCollisionTimeKey, ctx);
  if(! larTime.isValid())
  {
    ATH_MSG_WARNING( "Unable to retrieve LArCollisionTime event store" );
    return StatusCode::SUCCESS; // Check if failure shd be returned. VB
  } else {
    ATH_MSG_DEBUG( "LArCollisionTime successfully retrieved from event store" );
  }


  if(!event_info->isEventFlagBitSet(xAOD::EventInfo::LAr,3)) { // Do not fill histo if noise burst suspected

    // Calculate the time diff between ECC and ECA
    ecTimeDiff = larTime->timeC() - larTime->timeA();
    ecTimeAvg  = (larTime->timeC() + larTime->timeA()) / 2.0;
    if (larTime->ncellA() > m_minCells && larTime->ncellC() > m_minCells && std::fabs(ecTimeDiff) < m_timeCut ) { // Only fill histograms if a minimum number of cells were found and time difference was sensible

      //set the weight if needed
      if (m_eWeighted) weight = (larTime->energyA()+larTime->energyC())*1e-3; 

      //fill your group
      fill(m_MonGroupName,ecTimeDiff,ecTimeAvg,lumi_block,bunch_crossing_id,weight); 

      //check timeWindow
      lumi_block_timeWindow=lumi_block;
      if ( fabs(ecTimeDiff) < 10 ) fill(m_MonGroupName,lumi_block_timeWindow,weight);
   
      //check singleBeam-timeWindow
      lumi_block_singleBeam_timeWindow=lumi_block;
      if ( fabs(ecTimeDiff) > 20 && fabs(ecTimeDiff) < 30 ) fill(m_MonGroupName,lumi_block_singleBeam_timeWindow,weight); 

      //check in-train (online only)
      if(m_IsOnline && bcid_distance > m_distance) { // fill histos inside the train

        ATH_MSG_INFO("BCID: "<<bunch_crossing_id<<" distance from Front: "<<bcid_distance<<"Filling in train...");    
	if(m_InTrain_MonGroupName!="") { //group name is empty by default, give it a name when you define it in the python

	  //fill the intrain group
	  fill(m_InTrain_MonGroupName,ecTimeDiff,ecTimeAvg,lumi_block,bunch_crossing_id,weight); 
	  //check timeWindow
	  if ( fabs(ecTimeDiff) < 10 ) fill(m_InTrain_MonGroupName,lumi_block_timeWindow,weight); 
   
	  //check singleBeam-timeWindow
	  if ( fabs(ecTimeDiff) > 20 && fabs(ecTimeDiff) < 30 ) fill(m_InTrain_MonGroupName,lumi_block_singleBeam_timeWindow,weight);
	}
	else ATH_MSG_WARNING( "I should be filling the 'inTrain' group now, but it looks undefined. Did you remember to set 'm_InTrain_MonGroupName' in the python?" );
      }
    }
  }
  
  return StatusCode::SUCCESS;
}


