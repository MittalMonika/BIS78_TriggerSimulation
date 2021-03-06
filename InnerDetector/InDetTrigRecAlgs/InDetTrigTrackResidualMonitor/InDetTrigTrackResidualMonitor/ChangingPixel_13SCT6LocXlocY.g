/////////////////////////////////////////////////////////////////////////////
/**
 // filename: TrigTrackSlimmer.h
 //
 // author: Jiri Masik 
 //
 // Description:  Slim input track collection 
 */
////////////////////////////////////////////////////////////////////////////

#ifndef INDETTRIGTRACKSLIMMER_H
#define INDETTRIGTRACKSLIMMER_H

#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "DataModel/DataVector.h"
#include "GaudiKernel/ITHistSvc.h"


//typedef
#include "TrkTrack/TrackCollection.h"
//#include "TrkTrack/Track.h"
#include "TrkTrack/TrackInfo.h"

//!< Trigger specific stuff
#include "TrigInterfaces/FexAlgo.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkToolInterfaces/IResidualPullCalculator.h"
#include "TrkEventPrimitives/TrackStateDefs.h"
#include "TrkEventPrimitives/FitQualityOnSurface.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include "TrkValEvent/TrackStateData.h"
#include "Identifier/IdentifierHash.h"
#include "Identifier/Identifier.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "InDetPrepRawData/PixelClusterCollection.h"
#include "InDetPrepRawData/PixelClusterContainer.h"
#include "InDetPrepRawData/TRT_DriftCircleContainer.h"

#include "InDetReadoutGeometry/SCT_DetectorManager.h"  
#include "InDetReadoutGeometry/PixelDetectorManager.h"  
#include "InDetPrepRawData/SiClusterContainer.h"
#include "InDetPrepRawData/SCT_ClusterCollection.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"


class AtlasDetectorID;
class Track;
class IInDetConditionsSvc;
class PixelID;
class SCT_ID;

class SvcLocator;
class StoreGateSvc;
class SCT_NeighboursTable;

namespace InDetDD{
  class SCT_DetectorManager;
  class PixelDetectorManager;
}

class IInDetAlignHitQualSelTool; 

namespace Trk {

  class IUpdator;
  class IPropagator;
  
  class ITrackSlimmingTool;
  class ITrackSelectorTool;
  class ITrackSummaryTool;

  class MeasurementBase;
  class IResidualPullCalculator;
  class TrackStateOnSurface;
  class ResidualPull;
  class AlignTSOS;
  class Surface;
  class RIO_OnTrack;


}

namespace InDet
{

  class TrigTrackPrint : public HLT::FexAlgo {

    
 
  public:
    TrigTrackPrint(const std::string &name, ISvcLocator *pSvcLocator);
    virtual ~TrigTrackPrint();
    HLT::ErrorCode hltBeginRun();
    HLT::ErrorCode hltInitialize();
    HLT::ErrorCode hltExecute(const HLT::TriggerElement* input, HLT::TriggerElement* output);
    HLT::ErrorCode hltFinalize();
    HLT::ErrorCode hltEndRun();
 
    //   HLT::ErrorCode  getSiResiduals(const Trk::Track*, const Trk::TrackStateOnSurface*, bool, double*);

 
 
  private:
     const TrackCollection*                m_allTracksFromStoreGate;


     // tools 
  
    //  const AtlasDetectorID* m_idHelper;

    const PixelID*           m_idHelperPixel;
    const SCT_ID*            m_idHelperSCT;
    const InDetDD::SCT_DetectorManager*   m_SCT_Mgr;

 
    //   ToolHandle<IInDetAlignHitQualSelTool>  m_hitQualityTool;

    // StatusCode getSiResiduals(const Trk::Track*, const Trk::TrackStateOnSurface*, bool, double*);
   
    //const Trk::TrackParameters* getUnbiasedTrackParameters(const Trk::Track*, const Trk::TrackStateOnSurface*);
    //monitoring stuff

  
 
    bool                     m_monitoring; //!< monitoring variables
    std::string              m_path;      //!< monitoring path


    ToolHandle< Trk::ITrackSelectorTool > m_trackSelectorTool;
    ToolHandle<Trk::ITrackSummaryTool>    m_trkSummaryTool;

    ToolHandle<Trk::IUpdator>             m_updator;
    ToolHandle<Trk::IPropagator>          m_propagator; 
   
    Trk::ParticleHypothesis   m_ParticleHypothesis;         

    bool                      m_unbiased;         //!< flag for unbiased residual (for SCT hits, only one hit on the module is removed)
    bool                      m_TrueUnbiased;     //!< Flag for true unbiased residual (both hits of a SCT module are removed)
  
  
    //! The residual and pull calculator tool
    ToolHandle<Trk::IResidualPullCalculator> m_residualPullCalculator;
    ToolHandle<Trk::IResidualPullCalculator> m_SCTresidualTool; //!< the ResidualPullCalculator for the SCT

    const InDetDD::SCT_DetectorManager    *m_SCT_Manager;
    const InDetDD::PixelDetectorManager   *m_Pixel_Manager;


    //    std::string m_Pixel_Manager;
    //    std::string m_SCT_Manager;
    
    int    m_resType;
    int    m_detectorType;

    
    std::vector<double> m_resPixelPhiBiased;
    std::vector<double> m_resPixelEtaBiased;
    std::vector<double> m_pullPixelPhiBiased;
    std::vector<double> m_pullPixelEtaBiased;
    std::vector<double> m_resSCTBiased;
    std::vector<double> m_pullSCTBiased;
    
    std::vector<double> m_resPixelPhiUnbiased;
    std::vector<double> m_resPixelEtaUnbiased;
    std::vector<double> m_pullPixelPhiUnbiased;
    std::vector<double> m_pullPixelEtaUnbiased;
    std::vector<double> m_resSCTUnbiased;
    std::vector<double> m_pullSCTUnbiased;
    
    std::vector<double> m_resSCTUnbiasedTRIAL;
    std::vector<double> m_pullSCTUnbiasedTRIAL; 
    
    std::vector<double> m_resSCTBiasedTRIAL;
    std::vector<double> m_pullSCTBiasedTRIAL;
    
    
    std::vector<double> m_TrackPt;
    std::vector<double> m_npix;
    std::vector<double> m_nsct;
    std::vector<double> m_npixh;
    std::vector<double> m_nscth;
    

    /*
     std::vector<double> m_resPixelPhiResTypeUnbiased;
     std::vector<double> m_resPixelEtaResTypeUnbiased;
     std::vector<double> m_pullPixelPhiResTypeUnbiased;
     std::vector<double> m_pullPixelEtaResTypeUnbiased;
     std::vector<double> m_resSCTResTypeUnbiased;
     std::vector<double> m_pullSCTResTypeUnbiased;
    */

    };
  
}

#endif
