/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LAR_RAW_CHANNEL_MON_TOOL_H
#define LAR_RAW_CHANNEL_MON_TOOL_H
/*!
  \class LArRawChannelMonTool
  \author Frank Berghaus <Frank.Olaf.Berghaus@cern.ch>
 */

// --- stl ---
#include <string>
#include <vector>
#include <set>
// --- boost ---
#include <boost/shared_ptr.hpp>

// --- athena ---
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/StatusCode.h"
#include "AthenaMonitoring/ManagedMonitorToolBase.h"
#include "AthenaMonitoring/DQAtlasReadyFilterTool.h"
//#include "AthenaMonitoring/IDQFilterTool.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloInterface/ICaloNoiseTool.h"
#include "LArTools/LArCablingService.h"
#include "LArRecConditions/ILArBadChannelMasker.h"
#include "LArMonTools/LArRawChannelMonTools.h"


// --- forward declarations ---
class CaloDetDescrManager;
class CaloIdManager;
class LArOnlineID;
class LArOnlineIDStrHelper;
//class DQAtlasReadyFilterTool;

class TProfile_LW;
class TProfile2D_LW;
class TH2F_LW;
class TH1I_LW;
class LWHist;

//! A monitor for the LArg's Raw Channels
/*!
 A tool desigened to monitor the raw output of the liquid argon
 calorimeter channels. Originally written by Rob McPherson,
 re-written and expanded by Frank Berghaus <frank@uvic.ca>

 The tool inherits from ManagedMonitorToolBase. The job options
 are described in their detail sections.
*/
class LArRawChannelMonTool: public ManagedMonitorToolBase
{

 public:

  //! The constructor
  LArRawChannelMonTool(const std::string&, const std::string&,
		       const IInterface*);

  //! The Destructor
  virtual ~LArRawChannelMonTool();


  //! Connects to services and tools
  /*!
    The LArOnlineID and LArCablingService are retrieved. The
    CaloNoiseTool and LArBadChannelMasker configurables are also
    retrieved. The LArOnlineIDStrHelper is instantiated. Mapping
    from FEB hash to feedthrough hash, from FEB hash to sub-detector, and
    from superslot (Halfcrate+slot number) to sparcified axis bin are
    created.
  */
  virtual StatusCode initialize();


  //! Book and register histograms
  /*!
    The histograms demaded by the job options are booked for the
    Interval_t and LevelOfDetail_t specified in the job options.
    automatically called through ManagedMonitorToolBase everytime
    an interval expires.
    \param isNewEventsBlock true if bookHistograms was called because a new events block has begun
    \param isNewLumiBlock true if bookHistograms was called because a new luminosity block has begun
    \param isNewRun true if bookHistograms was called because a new run has begun
  */
  virtual StatusCode bookHistograms();


  //! Fill histograms with monitoring quantities
  /*!
   Returns success if no LArRawChannelContainer exists, returns failure if
   LArRawChannelContainer cannot be retrieved. Retrieves the bunch crossing
   ID from the EventInfo, then loops over all LArRawChannels in the
   LArRawChannelContainer and fills requested monitoring histograms.
  */
  virtual StatusCode fillHistograms();


  //! Fill RMS histograms
  /*!
    Fills histograms that use information from many events such as the
    histograms of noise RMS.
   */
  virtual StatusCode procHistograms();


 private:

  // --- Job Option Variables ---
  bool _useElecNoiseOnly;
  bool _monitor_occupancy;
  bool _monitor_signal;
  bool _monitor_positive_noise;
  bool _monitor_negative_noise;
  bool _monitor_time;
  bool _monitor_quality;
  bool _monitor_burst;

  bool _monitor_febs;
  bool _monitor_feedthroughs;
  bool _monitor_detectors;

  // --- Naming Conventions ---
  std::string _data_name_base;
  std::string _LArRawChannel_container_key;

  // --- Monitoring Controls ---
  std::vector<double>      _occupancy_thresholds;
  std::vector<double>      _signal_thresholds;
  std::vector<int>         _pos_noise_thresholds;
  std::vector<int>         _neg_noise_thresholds;
  double                   _bcid_signal_threshold;
  short                    _time_threshold;
  unsigned short           _quality_threshold;
  short                    _noise_threshold;
  std::vector<double>      _noise_burst_percent_thresholds;
  std::vector<unsigned int>         _noise_burst_nChannel_thresholds;
  unsigned int             _n_lumi_blocks;
  std::vector<std::string> _noise_streams;
  std::set<std::string>    m_noise_streams_set;

  // --- tools ---
  const LArOnlineID         *_lar_online_id_ptr; //!< LAr online structure
  const CaloDetDescrManager *_calo_description_mgr_ptr;
  const CaloIdManager       *_calo_id_mgr_ptr; //!< offline calo structure

  ToolHandle<LArCablingService>    _cabel_service_tool;//!< LAr connections
  ToolHandle<ICaloNoiseTool>       _calo_noise_tool;   //!< Calorimeter noise
  ToolHandle<ILArBadChannelMasker> _masking_tool;      //!< LAr Masking

  // -- for ATLAS Ready Filter
  bool isATLASReady() { return _atlas_ready; }
  void setIsATLASReady();
  ToolHandleArray<IDQFilterTool>        _filterAtlasReady_tools;   //!< DQ ATLAS Ready Filter Tool
  bool _atlas_ready;

  LArOnlineIDStrHelper      *_lar_online_id_str_helper_ptr; //!< naming convetions

  //! Switches tool off if there is no data in _LArRawChannel_container_key
  bool _has_lar_raw_channels;
  //! Use channels only if pedestal and adc2e value from DB and OFC was used
  bool _db_and_ofc_only;

  // --- Histograms ---
  //! pointers to per FEB histograms
  std::map< LArMonTools::RawChHisto, std::vector<boost::shared_ptr<LArMonTools::IHistoProxyBase> > > _per_feb_hists;

  //! pointers to per feedthrough 2D histograms
  std::map< LArMonTools::RawChHisto, std::vector<boost::shared_ptr<LArMonTools::IHistoProxyBase> > > _per_feedthrough_hists;

  // per detector view
  std::map< LArMonTools::RawChHisto, std::map<LArMonTools::Detector,boost::shared_ptr<LArMonTools::IHistoProxyBase> > >
    _per_detector_hists;


  // global lar histograms
  //  TProfile2D_LW* _global_lar_eta_phi_acc;
  //  TProfile_LW* _global_bcid_occupancy;

  std::map<LArMonTools::Detector,std::vector<TProfile2D_LW*> > _eta_phi_signal;
  std::map<LArMonTools::Detector,std::vector<TH2F_LW*> > _eta_phi_noise;

  std::map<LArMonTools::Detector,std::map< LArMonTools::Sampling, TH1I_LW* > > _phi_occupancy;
  std::map<LArMonTools::Detector,std::map< LArMonTools::Sampling, TH1I_LW* > > _eta_occupancy;

  std::vector<LArMonTools::Detector>                _feb_hash_to_detector;
  std::vector<IdentifierHash>                       _feb_hash_to_ft_hash;
  //  std::map<LArMonTools::Detector,std::vector<int> > _superslot_to_sparse_bin_number;

  //! number of LArRawChannels connected in each detector
  std::map<LArMonTools::Detector,unsigned int> _det_to_nchannels;
  //! monitoring interval
  /*!
    Interval to book monitoring histograms with. Valid strings in job options
    are: "all", "fill", "run" , "minutes90", "minutes30", "minutes10",
    "lumiBlock", "eventsBlock", "file". Set though intervalStringToEnum.
  */
  Interval_t _interval;


  //! monitoring level of detail
  /*!
    Level of detail to book monitoring histograms with. Valid strings in job
    options are: "summary", "runstat", "shift", "expert", "debug" , "transient".
    Set though levelOfDetailStringToEnum
  */

  std::string _interval_str; //!< strings to set interval with job option

  //! true if the event is in a stream specified by noise_stream job option
  bool _is_noise_event;

  std::map<LArMonTools::Detector,LArMonTools::SelectionContext> _selectionContext;
  LArMonTools::QualitySelector _qualitySelector;

  //! Calculates approximate time resolution of a channel
  /*!
    Time resolution is given by:
    \f[
    \sigma_t = \frac{a}{E/\sigma_{E}} \oplus b
    \f]
    Where a and b depend on the position (partition
    and layer) of the channel. Practically since
    \f$\sigma_{E}\f$ is also a function of position
    we approximate \f$a \approx 30\f$ns and
    \f$b \approx 1\f$ns.
    @See LArMonTools::SelectionContext
   */
  LArMonTools::ResolutionCalculator _timeResolution;

  // --- some tool specific helpers ---
  std::string threshold_histogram_title_snippet( const double& );
  std::string threshold_histogram_title_snippet( const int& );
  void book_eta_phi_histograms();
  void book_emb_eta_phi_histograms ( const LArMonTools::Detector &detector );
  void book_emec_eta_phi_histograms( const LArMonTools::Detector &detector );
  void book_hec_eta_phi_histograms ( const LArMonTools::Detector &detector );
  void book_fcal_eta_phi_histograms( const LArMonTools::Detector &detector );

  //! retrieve sampling of a channel
  int channel_sampling( const LArMonTools::Detector &detector, const Identifier &id );

  //! fix empty bins in eta/phi histograms
  //   typedef const std::pair<LArMonTools::Detector,std::vector<LWHist*> > det_eta_phi_hists_t;

  void fix_eta_phi_bins( const std::pair<LArMonTools::Detector,std::vector<TProfile2D_LW*> >& );
  void fix_eta_phi_bins( const std::pair<LArMonTools::Detector,std::vector<TH2F_LW*> >& );


  LArMonTools::lar_axis_maker _phi_axes; //!< handle phi-axes
  LArMonTools::lar_axis_maker _eta_axes; //!< handle eta-axes

  unsigned int _event_counter;
  unsigned int _noise_stream_event_counter;

  //! Lable histogram axis of 1440 bins with 24h clock
  //void label_gmt_axis( TAxis * );

  //! to register histograms
//   bool registerHistogram( TH1* histo, const std::string& dir,
// 			  const std::string& chain = "", const std::string& merge = "");//tk: retire
  bool registerHistogram( LWHist* histo, const std::string& dir,
			  const std::string& merge = "");

  LArMonTools::IHistoProxyBase * createLWHistProxy(LWHist*h);

  //! Helper function to connect handles
  /*!
    Helper function to connects gaudi handles and reports progress.
    \param handle is a ToolHandle or a ServiceHandle
  */
  template <class HANDLE> bool RetrieveHandle( const HANDLE& handle );


};


/* -------------------------------------------------------------------------- */
template <class HANDLE>
bool LArRawChannelMonTool::RetrieveHandle( const HANDLE& handle )
{
  if ( handle.retrieve().isSuccess() ) {
    msg(MSG::DEBUG) << "successfully retrieved: " << handle << endreq;
  } else {
    msg(MSG::ERROR) << "unable to retrieve: " << handle << endreq;
    return false;
  }
  return true;
}


#endif // LAR_RAW_CHANNEL_MON_TOOL_H
