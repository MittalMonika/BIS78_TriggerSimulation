/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// Athena/Gaudi includes
#include "GaudiKernel/ITHistSvc.h"
#include "StoreGate/StoreGateSvc.h"
#include "AthenaMonitoring/IMonitorToolBase.h"

//Event info includes
#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"

// root includes
#include "TTree.h"

// Local includes
#include "TrigT1NSW/NSWL1Simulation.h"

#include <vector>

namespace NSWL1 {

  NSWL1Simulation::NSWL1Simulation( const std::string& name, ISvcLocator* pSvcLocator )
    : AthAlgorithm( name, pSvcLocator ),
      m_monitors(this),
      m_pad_tds("NSWL1::PadTdsOfflineTool",this),
      m_pad_trigger("NSWL1::PadTriggerLogicOfflineTool",this),
      m_strip_tds("NSWL1::StripTdsOfflineTool",this),
      m_strip_cluster("NSWL1::StripClusterTool",this),
      m_strip_segment("NSWL1::StripSegmentTool",this),
      m_mmstrip_tds("NSWL1::MMStripTdsOfflineTool",this),
      m_mmtrigger("NSWL1::MMTriggerTool",this),
      m_tree(nullptr),
      m_current_run(-1),
      m_current_evt(-1)
  {

    m_counters.clear();

    // Services

    // Property setting general behaviour:
    declareProperty( "DoOffline",    m_doOffline    = false, "Steers the offline emulation of the LVL1 logic" );
    declareProperty( "DoNtuple",     m_doNtuple     = false, "Create an ntuple for data analysis" );
    declareProperty( "DoMM",         m_doMM         = false, "Run data analysis for MM" );
    declareProperty( "DosTGC",       m_dosTGC       = true, "Run data analysis for sTGCs" );
    declareProperty( "DoPadTrigger", m_doPadTrigger = true, "Run the pad trigger simulation" );

    // declare monitoring tools
    declareProperty( "AthenaMonTools",  m_monitors, "List of monitoring tools to be run with this instance, if incorrect then tool is silently skipped.");

    declareProperty( "PadTdsTool",      m_pad_tds,  "Tool that simulates the functionalities of the PAD TDS");
    declareProperty( "PadTriggerTool",  m_pad_trigger, "Tool that simulates the pad trigger logic");
    declareProperty( "StripTdsTool",    m_strip_tds,  "Tool that simulates the functionalities of the Strip TDS");
    declareProperty( "StripClusterTool",m_strip_cluster,  "Tool that simulates the Strip Clustering");
    declareProperty( "StripSegmentTool",m_strip_cluster,  "Tool that simulates the Segment finding");
    declareProperty( "MMStripTdsTool",  m_mmstrip_tds,  "Tool that simulates the functionalities of the MM STRIP TDS");
    declareProperty( "MMTriggerTool",   m_mmtrigger,    "Tool that simulates the MM Trigger");


    // declare monitoring variables
    declareMonitoredStdContainer("COUNTERS", m_counters); // custom monitoring: number of processed events
  }

  NSWL1Simulation::~NSWL1Simulation() {

  }


  StatusCode NSWL1Simulation::initialize() {
    ATH_MSG_INFO( "initialize " << name() );

    StatusCode sc;

    // Create an register the ntuple if requested, add branch for event and run number
    if ( m_doNtuple ) {
      ITHistSvc* tHistSvc;
      sc = service("THistSvc", tHistSvc);
      if(sc.isFailure()) {
        ATH_MSG_FATAL("Unable to retrieve THistSvc");
        return sc;
      }

      char ntuple_name[40];
      memset(ntuple_name,'\0',40*sizeof(char));
      sprintf(ntuple_name,"%sTree",name().c_str());

      // create Ntuple and the branches
      m_tree = new TTree(ntuple_name, "Ntuple of NSWL1Simulation");
      m_tree->Branch("runNumber",   &m_current_run, "runNumber/i");
      m_tree->Branch("eventNumber", &m_current_evt, "eventNumber/i");


      char tdir_name[80];
      memset(tdir_name,'\0',80*sizeof(char));
      sprintf(tdir_name,"/%s/%s",name().c_str(),ntuple_name);

      sc = tHistSvc->regTree(tdir_name,m_tree);
      if (sc.isFailure()) {
        ATH_MSG_FATAL("Unable to register the Tree " << m_tree->GetName() << " in the THistSvc");
        return sc;
      }
    }


    // retrieving the private tools implementing the simulation
    if ( m_pad_tds.retrieve().isFailure() ) {
      ATH_MSG_FATAL (m_pad_tds.propertyName() << ": Failed to retrieve tool " << m_pad_tds.type());
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_INFO (m_pad_tds.propertyName() << ": Retrieved tool " << m_pad_tds.type());
    }

    if ( m_pad_trigger.retrieve().isFailure() ) {
      ATH_MSG_FATAL (m_pad_trigger.propertyName() << ": Failed to retrieve tool " << m_pad_trigger.type());
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_INFO (m_pad_trigger.propertyName() << ": Retrieved tool " << m_pad_trigger.type());
    }

    if ( m_strip_tds.retrieve().isFailure() ) {
      ATH_MSG_FATAL (m_strip_tds.propertyName() << ": Failed to retrieve tool " << m_strip_tds.type());
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_INFO (m_strip_tds.propertyName() << ": Retrieved tool " << m_strip_tds.type());
    }
    if ( m_mmtrigger.retrieve().isFailure() ) {
      ATH_MSG_FATAL (m_mmtrigger.propertyName() << ": Failed to retrieve tool " << m_mmtrigger.type());
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_INFO (m_mmtrigger.propertyName() << ": Retrieved tool " << m_mmtrigger.type());
    }

    if ( m_strip_cluster.retrieve().isFailure() ) {
      ATH_MSG_FATAL (m_strip_cluster.propertyName() << ": Failed to retrieve tool " << m_strip_cluster.type());
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_INFO (m_strip_cluster.propertyName() << ": Retrieved tool " << m_strip_cluster.type());
    }

    if ( m_strip_segment.retrieve().isFailure() ) {
      ATH_MSG_FATAL (m_strip_segment.propertyName() << ": Failed to retrieve tool " << m_strip_segment.type());
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_INFO (m_strip_segment.propertyName() << ": Retrieved tool " << m_strip_segment.type());
    }


    // Connect to Monitoring Service
    sc = m_monitors.retrieve();
    if ( sc.isFailure() ) {
      ATH_MSG_ERROR("Monitoring tools not initialized: " << m_monitors);
      return sc;
    } else {
      ATH_MSG_DEBUG("Connected to " << m_monitors.typesAndNames());
    }

    return StatusCode::SUCCESS;
  }


  StatusCode NSWL1Simulation::start() {
    ATH_MSG_INFO("start " << name() );

    ToolHandleArray<IMonitorToolBase>::iterator it;
    for ( it = m_monitors.begin(); it != m_monitors.end(); ++it ) {
      if ( (*it)->bookHists().isFailure() ) {
        ATH_MSG_WARNING("Monitoring tool: " <<  (*it)
                          << " in Algo: " << name()
                          << " can't book histograms successfully, remove it or fix booking problem");
        return StatusCode::FAILURE;
      } else {
        ATH_MSG_DEBUG("Monitoring tool: " <<  (*it)
                        << " in Algo: " << name() << " bookHists successful");
      }
    }

    return StatusCode::SUCCESS;
  }


  StatusCode NSWL1Simulation::beginRun() {
    ATH_MSG_INFO( "beginRun " << name() );
    m_counters.clear();
    return StatusCode::SUCCESS;
  }


  StatusCode NSWL1Simulation::execute() {
    ATH_MSG_INFO( "execute" << name() );
    m_counters.clear();

    // retrieve the current run number and event number
    const EventInfo* pevt = 0;
    ATH_CHECK( evtStore()->retrieve(pevt) );
    m_current_run = pevt->event_ID()->run_number();
    m_current_evt = pevt->event_ID()->event_number();


    m_counters.push_back(1.);  // a new event is being processed

    std::vector<spPadData> pads;
    std::vector<upPadTrigger> padTriggers;
    std::vector<upStripData> strips;
    std::vector< upStripClusterData > clusters;
    // retrieve the PAD hit and compute pad trigger
    if(m_dosTGC){
        // DG-2015-10-02
        // Currently storing pads from all sectors in one vector<>.
        // Perhaps we could do here for(sector) instead of inside
        // PadTriggerLogicOfflineTool (since all the pad and
        // pad-trigger info is per-sector...)
      
        //std::vector<PadData*> pads;
      ATH_CHECK( m_pad_tds->gather_pad_data(pads) );

      //std::vector<NSWL1::PadTrigger*> padTriggers; // will be passed on to m_strip_tds
      if(m_doPadTrigger){
          ATH_CHECK( m_pad_trigger->compute_pad_triggers(pads, padTriggers) );
      }
      pads.clear();
      // retrieve the STRIP hit data
      ATH_CHECK( m_strip_tds->gather_strip_data(strips,padTriggers) );
      padTriggers.clear();
      //std::vector< NSWL1::StripClusterData* > clusters;
      // Cluster STRIPs readout by TDS
      ATH_CHECK( m_strip_cluster->cluster_strip_data(strips,clusters) );
      strips.clear();
      ATH_CHECK( m_strip_segment->find_segments(clusters) );
      clusters.clear();
    } // if(dosTGC)


    //retrive the MM Strip hit data
    if(m_doMM){
 //      std::vector<MMStripData*> mmstrips;
 //      sc = m_mmstrip_tds->gather_mmstrip_data(mmstrips);
 //      if ( sc.isFailure() ) {
	// ATH_MSG_ERROR( "Could not gather the MM Strip hit data" );
	// return sc;
 //      }
      ATH_CHECK( m_mmtrigger->runTrigger() );
    }

    //
    // do monitoring
    //
    ToolHandleArray<IMonitorToolBase>::iterator it;
    for ( it = m_monitors.begin(); it != m_monitors.end(); ++it ) {
      (*it)->fillHists().ignore();
    }

    if (m_tree) m_tree->Fill();

    return StatusCode::SUCCESS;
  }


  StatusCode NSWL1Simulation::finalize() {
    ATH_MSG_INFO( "finalize" << name() );

    //
    // monitoring
    //
    ToolHandleArray<IMonitorToolBase>::iterator it;
    for ( it = m_monitors.begin(); it != m_monitors.end(); ++it ) {
      (*it)->finalHists().ignore();
    }

    return StatusCode::SUCCESS;
  }


  int NSWL1Simulation::resultBuilder() const {
    return 0;
  }

}
