/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRKVERTEXSEEDFINDERTOOLS_INDEXCROSSDISTANCESSEEDFINDER_H
#define TRKVERTEXSEEDFINDERTOOLS_INDEXCROSSDISTANCESSEEDFINDER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrkVertexFitterInterfaces/IVertexSeedFinder.h"
#include "TrkVertexSeedFinderUtils/IMode3dFinder.h"
#include "GaudiKernel/ToolHandle.h"

namespace Trk
{
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
  //
  // ------------------------------------

  class IndexedCrossDistancesSeedFinder : public AthAlgTool, virtual public IVertexSeedFinder
  {
  public:
    StatusCode initialize();
    StatusCode finalize();

    //default constructor due to Athena interface
    IndexedCrossDistancesSeedFinder(const std::string& t, const std::string& n, const IInterface*  p);
    
    //destructor
    virtual ~IndexedCrossDistancesSeedFinder();

    virtual void setPriVtxPosition( double vx, double vy ) ;

    // Interface for Tracks with starting seed/linearization point
    virtual Amg::Vector3D findSeed(const std::vector<const Trk::Track*> & vectorTrk,const xAOD::Vertex * constraint=0);
    
    /** Interface for MeasuredPerigee with starting point */
    virtual Amg::Vector3D findSeed(const std::vector<const Trk::TrackParameters*> & perigeeList,const xAOD::Vertex * constraint=0);
        
    std::vector<Amg::Vector3D> findMultiSeeds(const std::vector<const Trk::Track*>& vectorTrk,
                          const xAOD::Vertex * constraint=0) ;

    std::vector<Amg::Vector3D> findMultiSeeds(
        const std::vector<const Trk::TrackParameters*>& perigeeList,const xAOD::Vertex * constraint=0) ;


    // 
    virtual int getModes1d(   std::vector<float> &, std::vector<float> &, 
                              std::vector<float> &, std::vector<float> & ) const ;
    // the tracks really used
    virtual int perigeesAtSeed( std::vector<const Trk::TrackParameters*> * , 
                              const std::vector<const Trk::TrackParameters*> & ) const ;

    inline void getCorrelationDistance( double &cXY, double &cZ )
      { cXY = m_correXY ; cZ = m_correZ ; return ; }
 
  private:

    bool m_useweights;
    float m_trackdistcutoff;
    int m_trackdistexppower;
    float m_constraintcutoff;
    float m_constrainttemp;
    ToolHandle<IMode3dFinder> m_mode3dfinder;
    ToolHandle<ITrkDistanceFinder> m_distancefinder;
    unsigned int m_maximumTracksNoCut;
    double m_maximumDistanceCut;

//    std::vector< std::pair <int, int> > * m_trkidx ;
    std::vector< std::pair <int, int> > m_trkidx ;

    mutable double m_correXY ;
    mutable double m_correZ ;

    double m_vx = 0;
    double m_vy = 0;

    std::unique_ptr<Trk::IMode3dFinder::IMode3dInfo> m_info;
  };
}
#endif
