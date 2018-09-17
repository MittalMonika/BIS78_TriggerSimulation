/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


#ifndef TrackClusterAssValidationITk_H
#define TrackClusterAssValidationITk_H

#include <string>
#include <map>

#include "AthenaBaseComps/AthAlgorithm.h"
#include "TrkSpacePoint/SpacePointContainer.h" 
#include "TrkSpacePoint/SpacePointOverlapCollection.h" 
#include "InDetPrepRawData/SiClusterContainer.h"
#include "InDetPrepRawData/TRT_DriftCircleContainer.h"
#include "HepMC/GenParticle.h"
#include "HepPDT/ParticleDataTable.hh"
#include "TrkTruthData/PRD_MultiTruthCollection.h"
#include "InDetTrackClusterAssValidation/TrackClusterAssValidationUtils.h"
#include "InDetRecToolInterfaces/IInDetEtaDependentCutsSvc.h"


namespace InDet {

  // Class-algorithm for track cluster association validation
  //
  class TrackClusterAssValidationITk : public AthAlgorithm 
    {
    
      ///////////////////////////////////////////////////////////////////
      // Public methods:
      ///////////////////////////////////////////////////////////////////
      
    public:
      
      ///////////////////////////////////////////////////////////////////
      // Standard Algotithm methods
      ///////////////////////////////////////////////////////////////////

      TrackClusterAssValidationITk(const std::string &name, ISvcLocator *pSvcLocator);
      virtual ~TrackClusterAssValidationITk() {}
      StatusCode initialize();
      StatusCode execute();
      StatusCode finalize();

      ///////////////////////////////////////////////////////////////////
      // Print internal tool parameters and status
      ///////////////////////////////////////////////////////////////////

      MsgStream&    dump     (MsgStream&    out) const;
      std::ostream& dump     (std::ostream& out) const;

    protected:

      ///////////////////////////////////////////////////////////////////
      // Protected data 
      ///////////////////////////////////////////////////////////////////

      bool                               m_usePIX                 ;
      bool                               m_useSCT                 ;
      bool                               m_useTRT                 ;
      bool                               m_useOutliers            ;
      int                                m_pdg                    ;
      int                                m_outputlevel            ;
      int                                m_nprint                 ;
      int                                m_ncolection             ; 
      int                                m_nspacepoints           ;
      int                                m_nclusters              ;
      int                                m_nevents                ;
      int                                m_nclustersPTOT          ;
      int                                m_nclustersSTOT          ;
      int                                m_nspacepointsPTOT       ;
      int                                m_nspacepointsSTOT       ;
      int                                m_nspacepointsOTOT       ;
      int                                m_nclustersTRT           ;
      int                                m_nqtracks               ;
      int                                m_efficiency   [100][6]  ;
      int                                m_efficiencyN  [100][6][5];
      int                                m_efficiencyBTE[100][6][5][4];
      int                                m_efficiencyPOS[100][6]  ;
      int                                m_efficiencyNEG[100][6]  ;
      int                                m_ntracksPOSB  [100]     ;
      int                                m_ntracksPOSE  [100]     ;
      int                                m_ntracksPOSDBM[100];
      int                                m_ntracksNEGB  [100]     ;
      int                                m_ntracksNEGE  [100]     ;
      int                                m_ntracksNEGDBM[100];
      int                                m_total        [100][50] ;
      int                                m_fake         [100][50] ;
      int                                m_events                 ;
      int                                m_eventsPOS              ;
      int                                m_eventsNEG              ;
      int                                m_eventsBTE[4]           ;
      int                                m_particleClusters   [50];
      int                                m_particleSpacePoints[50];
      int                                m_particleClustersBTE   [50][4];
      int                                m_particleSpacePointsBTE[50][4];
      int                                m_nclustersPosBP         ;
      int                                m_nclustersPosBS         ;
      int                                m_nclustersPosEP         ;
      int                                m_nclustersPosES         ;
      int                                m_nclustersNegBP         ;
      int                                m_nclustersNegBS         ;
      int                                m_nclustersNegEP         ;
      int                                m_nclustersNegES         ;
      unsigned int                       m_clcut                  ;
      unsigned int                       m_clcutTRT               ;
      unsigned int                       m_spcut                  ;
      double                             m_ptcut                  ;
      double                             m_ptcutmax               ;
      double                             m_rapcut                 ;
      double                             m_tcut                   ;
      double                             m_rmin                   ;
      double                             m_rmax                   ;

      double                             m_rapTRAN                ;
      double                             m_rapENDS                ;
      double                             m_rapENDP                ;
   
      std::vector<std::string>           m_tracklocation          ; 
      std::string                        m_clustersSCTname        ;
      std::string                        m_clustersPixelname      ;
      std::string                        m_clustersTRTname        ;
      std::string                        m_truth_locationPixel    ;
      std::string                        m_truth_locationSCT      ;
      std::string                        m_truth_locationTRT      ;

      SG::ReadHandle<SpacePointContainer>         m_spacepointsSCT    ;
      SG::ReadHandle<SpacePointContainer>         m_spacepointsPixel  ;
      SG::ReadHandle<SpacePointOverlapCollection> m_spacepointsOverlap;

      const SiClusterContainer         * m_pixcontainer           ;
      const SiClusterContainer         * m_sctcontainer           ;
      const TRT_DriftCircleContainer   * m_trtcontainer           ;

      const PRD_MultiTruthCollection   * m_truthPIX               ;
      const PRD_MultiTruthCollection   * m_truthSCT               ;
      const PRD_MultiTruthCollection   * m_truthTRT               ;

      std::multimap<const HepMC::GenParticle*,const Trk::PrepRawData*> m_kineclusterN    ;
      std::multimap<const HepMC::GenParticle*,const Trk::PrepRawData*> m_kineclusterTRTN ;
      std::multimap<const HepMC::GenParticle*,const Trk::SpacePoint*>  m_kinespacepointN ;

      std::list<Barcode>                         m_particles[100] ;
      std::list<int>                             m_difference[100];
      std::multimap<int,int>                     m_tracks[100]    ;
      std::multimap<const HepMC::GenParticle*,int> m_tracksN[100] ;
      const HepPDT::ParticleDataTable*        m_particleDataTable ;
      
      /** service to get cut values depending on different variable */
      ServiceHandle<IInDetEtaDependentCutsSvc>     m_etaDependentCutsSvc;   
      
      ///////////////////////////////////////////////////////////////////
      // Protected methods
      ///////////////////////////////////////////////////////////////////

      void newSpacePointsEvent     ();
      void newClustersEvent        ();
      void tracksComparison        ();
      void efficiencyReconstruction();
      bool noReconstructedParticles();
      int  QualityTracksSelection  ();

      int kine (const Trk::PrepRawData*,const Trk::PrepRawData*,const HepMC::GenParticle**,int);
      int kine (const Trk::PrepRawData*,const HepMC::GenParticle**,int);
      int kine0(const Trk::PrepRawData*,const HepMC::GenParticle**,int);
      bool isTheSameDetElement(const HepMC::GenParticle*,const Trk::PrepRawData*);
      bool isTheSameDetElement(const HepMC::GenParticle*,const Trk::SpacePoint* );

      PRD_MultiTruthCollection::const_iterator findTruth
	(const Trk::PrepRawData*,PRD_MultiTruthCollection::const_iterator&);
      
      int charge(std::pair<int,const Trk::PrepRawData*>,int&);
      int charge(std::pair<int,const Trk::PrepRawData*>,int&, double&);

      int charge(std::pair<const HepMC::GenParticle*,const Trk::PrepRawData*>,int&);
      int charge(std::pair<const HepMC::GenParticle*,const Trk::PrepRawData*>,int&, double&);

      MsgStream&    dumptools(MsgStream&    out) const;
      MsgStream&    dumpevent(MsgStream&    out) const;

    };
  MsgStream&    operator << (MsgStream&   ,const TrackClusterAssValidationITk&);
  std::ostream& operator << (std::ostream&,const TrackClusterAssValidationITk&); 

}
#endif // TrackClusterAssValidationITk_H
