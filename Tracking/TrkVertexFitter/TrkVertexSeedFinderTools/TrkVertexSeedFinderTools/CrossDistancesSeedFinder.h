/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKVERTEXSEEDFINDERTOOLS_CROSSDISTANCESSEEDFINDER_H
#define TRKVERTEXSEEDFINDERTOOLS_CROSSDISTANCESSEEDFINDER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrkVertexFitterInterfaces/IVertexSeedFinder.h"
#include "GaudiKernel/ToolHandle.h"

namespace Trk
{
  class Vertex;
  class Track;
  class IMode3dFinder;
  class ITrkDistanceFinder;

  // @author N. Giacinto Piacquadio (Albert-Ludwigs-Universitaet Freiburg - Germany)
  //
  // @ATLAS software
  //
  // This class implements a seed finder for the vertexing algorithm 
  // which is based on the mean points of the distances between all tracks 
  // (taken two by two). There's actually no selection on the tracks.
  // 
  // 
  // A very similar point finder was implemented for the first time by Waltenberger 
  // in CMS. I suggest his PhD thesis (2005) as a reference for the different concepts 
  // of point finders.


  class CrossDistancesSeedFinder : public AthAlgTool, virtual public IVertexSeedFinder
  {
  public:
    StatusCode initialize();
    StatusCode finalize();

    //default constructor due to Athena interface
    CrossDistancesSeedFinder(const std::string& t, const std::string& n, const IInterface*  p);
    
    //destructor
    virtual ~CrossDistancesSeedFinder();

    // Interface for Tracks with starting seed/linearization point
    virtual Vertex findSeed(const std::vector<const Trk::Track*> & vectorTrk,const RecVertex * constraint=0);
    
    /** Interface for MeasuredPerigee with starting point */
    virtual Vertex findSeed(const std::vector<const Trk::TrackParameters*> & perigeeList,const RecVertex * constraint=0);
        
    // Interface for finding vector of seeds from tracks
    virtual std::vector<Vertex> findMultiSeeds(const std::vector<const Trk::Track*>& vectorTrk,const RecVertex * constraint=0);

    // Interface for finding vector of seeds from track parameters
    virtual std::vector<Vertex> findMultiSeeds(const std::vector<const Trk::TrackParameters*>& perigeeList,const RecVertex * constraint=0);

  private:

    bool m_useweights;
    int m_typeofweight;
    float m_trackdistcutoff;
    int m_trackdistexppower;
    float m_constraintcutoff;
    float m_constrainttemp;
    ToolHandle<IMode3dFinder> m_mode3dfinder;
    ToolHandle<ITrkDistanceFinder> m_distancefinder;
    unsigned int m_maximumTracksNoCut;
    double m_maximumDistanceCut;

  };
}
#endif
