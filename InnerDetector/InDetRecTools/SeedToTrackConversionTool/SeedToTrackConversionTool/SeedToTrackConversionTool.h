// -*- C++ -*-

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SeedToTrackConversionTool.h, (c) ATLAS Detector software
// Class for converting Seeds to Track Collection 
// Version 0.0 01/02/2016 Weiming Yao(LBNL)  
///////////////////////////////////////////////////////////////////

#ifndef INDETSEEDTOTRACKCONVERSIONTOOL_H
#define INDETSEEDTOTRACKCONVERSIONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetRecToolInterfaces/ISeedToTrackConversionTool.h"
#include "StoreGate/WriteHandleKey.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"
#include "TrkTrack/TrackCollection.h"

#include "GaudiKernel/ToolHandle.h"

#include <atomic>
#include <list>

class MsgStream;

namespace InDet 
{

  /** @class SeedToTrackConversionTool 

      InDet::SeedToTrackConversionTool is tool that converts the seeds
      into track collection. 
      
      @author  Weiming Yao <Weiming.Yao>
  */  

  class SeedToTrackConversionTool : public extends<AthAlgTool, ISeedToTrackConversionTool>
  {
  public:
    SeedToTrackConversionTool(const std::string&,const std::string&,const IInterface*);

    /** default destructor */
    virtual ~SeedToTrackConversionTool() = default;
      
    /** standard Athena-Algorithm method */
    virtual StatusCode initialize() override;
    /** standard Athena-Algorithm method */
    virtual StatusCode finalize() override;

    // Main methods for seeds conversion
    virtual void executeSiSPSeedSegments(SeedToTrackConversionData& data, const Trk::TrackParameters*, const int&, const std::list<const Trk::SpacePoint*>&) const override;
    //!<seed trackparameters, number of tracks found:m_track.size(), list of spacepoints
    virtual void newEvent(SeedToTrackConversionData& data, const Trk::TrackInfo&, const std::string&) const override;
    virtual void endEvent(SeedToTrackConversionData& data) const override;

    //////////////////////////////////////////////////////////////////
    // Print internal tool parameters and status
    ///////////////////////////////////////////////////////////////////
      
    virtual MsgStream& dump(SeedToTrackConversionData& data, MsgStream& out) const override;
      
  private:
    PublicToolHandle<Trk::IExtrapolator> m_extrapolator
      {this, "Extrapolator","Trk::Extrapolator", "extrapolator"};
    PublicToolHandle<Trk::IRIO_OnTrackCreator > m_rotcreator
      {this, "RIO_OnTrackCreator", "Trk::RIO_OnTrackCreator/InDetRotCreator", "Creator ROT"};
    SG::WriteHandleKey<TrackCollection> m_seedsegmentsOutput{this, "OutputName", "SiSPSeedSegments", "SiSpSeedSegments Output Collection"};

    mutable std::atomic_int m_nprint{0}; //!< Kind output information
    mutable std::atomic_int m_totseed{0}; //!< number of total seeds in the pass
    mutable std::atomic_int m_survived{0}; //!< number of survived seeds 

    MsgStream& dumpconditions(SeedToTrackConversionData& data, MsgStream& out) const;
    MsgStream& dumpevent     (SeedToTrackConversionData& data, MsgStream& out) const;

  }; 
} // end of namespace

#endif 
