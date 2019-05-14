// -*- C++ -*-

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
//  Header file for class SiSpacePointsSeedMaker_LowMomentum
/////////////////////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////////////////////
// Class for track candidates generation using space points information
// for standard Atlas geometry
/////////////////////////////////////////////////////////////////////////////////
// Version 1.0 3/10/2004 I.Gavrilenko
/////////////////////////////////////////////////////////////////////////////////

#ifndef SiSpacePointsSeedMaker_LowMomentum_H
#define SiSpacePointsSeedMaker_LowMomentum_H

#include "InDetRecToolInterfaces/ISiSpacePointsSeedMaker.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "BeamSpotConditionsData/BeamSpotData.h"
#include "MagFieldInterfaces/IMagFieldSvc.h"
#include "SiSpacePointsSeedTool_xk/SiSpacePointForSeed.h"
#include "TrkSpacePoint/SpacePointContainer.h" 
#include "TrkSpacePoint/SpacePointOverlapCollection.h"

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include <list>
#include <map>
#include <mutex>
#include <vector>

class MsgStream;

namespace Trk {
  class IPRD_AssociationTool;
}

namespace InDet {

  class SiSpacePointsSeedMaker_LowMomentum : 
    virtual public ISiSpacePointsSeedMaker, public AthAlgTool
  {
    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
      
  public:
      
    ///////////////////////////////////////////////////////////////////
    // Standard tool methods
    ///////////////////////////////////////////////////////////////////

    SiSpacePointsSeedMaker_LowMomentum
    (const std::string&,const std::string&,const IInterface*);
    virtual ~SiSpacePointsSeedMaker_LowMomentum() = default;
    virtual StatusCode initialize();
    virtual StatusCode finalize();

    ///////////////////////////////////////////////////////////////////
    // Methods to initialize tool for new event or region
    ///////////////////////////////////////////////////////////////////

    virtual void newEvent(int iteration);
    virtual void newRegion(const std::vector<IdentifierHash>& vPixel, const std::vector<IdentifierHash>& vSCT);
    virtual void newRegion(const std::vector<IdentifierHash>& vPixel, const std::vector<IdentifierHash>& vSCT, const IRoiDescriptor& IRD);
       
    ///////////////////////////////////////////////////////////////////
    // Methods to initilize different strategies of seeds production
    // with two space points with or without vertex constraint
    ///////////////////////////////////////////////////////////////////

    virtual void find2Sp(const std::list<Trk::Vertex>& lv);

    ///////////////////////////////////////////////////////////////////
    // Methods to initilize different strategies of seeds production
    // with three space points with or without vertex constraint
    ///////////////////////////////////////////////////////////////////

    virtual void find3Sp(const std::list<Trk::Vertex>& lv);
    virtual void find3Sp(const std::list<Trk::Vertex>& lv, const double* ZVertex);

    ///////////////////////////////////////////////////////////////////
    // Methods to initilize different strategies of seeds production
    // with variable number space points with or without vertex constraint
    // Variable means (2,3,4,....) any number space points
    ///////////////////////////////////////////////////////////////////
 
    virtual void findVSp(const std::list<Trk::Vertex>& lv);
      
    ///////////////////////////////////////////////////////////////////
    // Iterator through seeds pseudo collection produced accordingly
    // methods find    
    ///////////////////////////////////////////////////////////////////
      
    virtual const SiSpacePointsSeed* next();
      
    ///////////////////////////////////////////////////////////////////
    // Print internal tool parameters and status
    ///////////////////////////////////////////////////////////////////

    virtual MsgStream&    dump(MsgStream   & out) const;
    virtual std::ostream& dump(std::ostream& out) const;

  private:
    enum Size {SizeRF=20,
               SizeZ=11,
               SizeRFZ=SizeRF*SizeZ,
               SizeI=9};

    ///////////////////////////////////////////////////////////////////
    // Private data and methods
    ///////////////////////////////////////////////////////////////////
      
    ServiceHandle<MagField::IMagFieldSvc> m_fieldServiceHandle{this, "MagFieldSvc", "AtlasFieldSvc"};
    PublicToolHandle<Trk::IPRD_AssociationTool> m_assoTool{this, "AssociationTool", "InDet::InDetPRD_AssociationToolGangedPixels"};

    ///////////////////////////////////////////////////////////////////
    // Space points container
    ///////////////////////////////////////////////////////////////////
    SG::ReadHandleKey<SpacePointContainer> m_spacepointsSCT{this, "SpacePointsSCTName", "SCT_SpacePoints"};
    SG::ReadHandleKey<SpacePointContainer> m_spacepointsPixel{this, "SpacePointsPixelName", "PixelSpacePoints"};
    SG::ReadHandleKey<SpacePointOverlapCollection> m_spacepointsOverlap{this, "SpacePointsOverlapName", "OverlapSpacePoints"};

    SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };

    // Properties, which will not be changed after construction
    BooleanProperty m_pixel{this, "usePixel", true};
    BooleanProperty m_sct{this, "useSCT", true};
    BooleanProperty m_useOverlap{this, "useOverlapSpCollection", false};
    BooleanProperty m_useassoTool{this, "UseAssociationTool", true};
    IntegerProperty m_maxsize{this, "maxSize", 2000};
    IntegerProperty m_maxsizeSP{this, "maxSizeSP", 1500};
    IntegerProperty m_maxOneSize{this, "maxSeedsForSpacePoint", 5};
    FloatProperty m_r1min{this, "minRadius1", 0.};
    FloatProperty m_r1max{this, "maxRadius1", 600.};
    FloatProperty m_r2min{this, "minRadius2", 0.};
    FloatProperty m_r2max{this, "maxRadius2", 600.};
    FloatProperty m_r3min{this, "minRadius3", 0.};
    FloatProperty m_drmin{this, "mindRadius", 10.};
    FloatProperty m_drmax{this, "maxdRadius", 200.};
    FloatProperty m_zmin{this, "minZ", -250.};
    FloatProperty m_zmax{this, "maxZ", +250.};
    FloatProperty m_r_rmax{this, "radMax", 200.};
    FloatProperty m_r_rstep{this, "radStep", 2.};
    FloatProperty m_dzver{this, "maxdZver", 5.};
    FloatProperty m_dzdrver{this, "maxdZdRver", 0.02};
    FloatProperty m_diver{this, "maxdImpact", 7.};

    // Properties, which can be updated in initialize
    FloatProperty m_r3max{this, "maxRadius3", 600.}; // Always overwritten by m_r_rmax
    FloatProperty m_rapcut{this, "RapidityCut", 2.7};
    FloatProperty m_ptmin{this, "pTmin", 100.};
    FloatProperty m_ptmax{this, "pTmax", 500.};

    // Properties, which are not used in this implementation of SiSpacePointsSeedMaker_LowMomentum class
    FloatProperty m_diverpps{this, "maxdImpactPPS", 1.2};

    // Updated only in initialize
    bool m_initialized{false};
    int m_outputlevel{0};
    int m_fNmax{0};
    int m_r_size{0};
    int m_rfz_b[SizeRFZ];
    int m_rfz_t[SizeRFZ];
    int m_rfz_ib[SizeRFZ][SizeI];
    int m_rfz_it[SizeRFZ][SizeI];
    float m_dzdrmin{0.};
    float m_dzdrmax{0.};
    float m_iptmin{0.};
    float m_iptmax{1./400.};
    float m_sF{0.};

    mutable std::mutex m_mutex;
    mutable std::vector<EventContext::ContextEvt_t> m_cache ATLAS_THREAD_SAFE; // Guarded by m_mutex
    struct EventData { // To hold event dependent data
      bool endlist{true};
      bool trigger{false};
      int nprint{0};
      int state{0};
      int nspoint{2};
      int mode{0};
      int nlist{0};
      int ns{0};
      int nsaz{0};
      int fNmin{0};
      int zMin{0};
      int nr{0};
      int nrfz{0};
      int nOneSeeds{0};
      int rfz_index[SizeRFZ];
      int rfz_map[SizeRFZ];
      std::vector<int> r_index;
      std::vector<int> r_map;
      std::vector<std::list<InDet::SiSpacePointForSeed*>> r_Sorted;
      std::vector<InDet::SiSpacePointsSeed> OneSeeds;
      std::multimap<float,InDet::SiSpacePointsSeed*> mapOneSeeds;
      std::list<InDet::SiSpacePointForSeed*> rfz_Sorted[SizeRFZ];
      std::list<InDet::SiSpacePointForSeed> l_spforseed;
      std::list<InDet::SiSpacePointForSeed>::iterator i_spforseed;
      std::list<InDet::SiSpacePointForSeed*>::iterator rMin;
      std::list<InDet::SiSpacePointsSeed> l_seeds;
      std::list<InDet::SiSpacePointsSeed>::iterator i_seed;
      std::list<InDet::SiSpacePointsSeed>::iterator i_seede;
      std::list<float> l_vertex;
      ///////////////////////////////////////////////////////////////////
      // Tables for 3 space points seeds search
      ///////////////////////////////////////////////////////////////////
      std::vector<InDet::SiSpacePointForSeed*> SP;
      std::vector<float> R;
      std::vector<float> Tz;
      std::vector<float> Er;
      std::vector<float> U;
      std::vector<float> V;
      std::vector<float> Zo; 
      ///////////////////////////////////////////////////////////////////
      // Beam geometry
      ///////////////////////////////////////////////////////////////////
      float xbeam[4]{0., 1., 0., 0.}; // x,ax,ay,az - center and x-axis direction
      float ybeam[4]{0., 0., 1., 0.}; // y,ax,ay,az - center and y-axis direction
      float zbeam[4]{0., 0., 0., 1.}; // z,ax,ay,az - center and z-axis direction
    };
    mutable std::vector<EventData> m_eventData ATLAS_THREAD_SAFE; // Guarded by m_mutex

    ///////////////////////////////////////////////////////////////////
    // Private methods
    ///////////////////////////////////////////////////////////////////
    /**    @name Disallow default instantiation, copy, assignment */
    //@{
    SiSpacePointsSeedMaker_LowMomentum() = delete;
    SiSpacePointsSeedMaker_LowMomentum(const SiSpacePointsSeedMaker_LowMomentum&) = delete;
    SiSpacePointsSeedMaker_LowMomentum &operator=(const SiSpacePointsSeedMaker_LowMomentum&) = delete;
    //@}    

    MsgStream& dumpConditions(EventData& data, MsgStream& out) const;
    MsgStream& dumpEvent(EventData& data, MsgStream& out) const;

    void buildFrameWork();
    void buildBeamFrameWork(EventData& data) const;

    SiSpacePointForSeed* newSpacePoint
    (EventData& data, const Trk::SpacePoint*const&) const;
    void newSeed
    (EventData& data,
     const Trk::SpacePoint*&,const Trk::SpacePoint*&,
     const float&) const;
    void newSeed
    (EventData& data,
     const Trk::SpacePoint*&,const Trk::SpacePoint*&,
     const Trk::SpacePoint*&,const float&) const;

    void newOneSeed
    (EventData& data,
     const Trk::SpacePoint*&,const Trk::SpacePoint*&,
     const Trk::SpacePoint*&,const float&,const float&) const;
    void fillSeeds(EventData& data) const;

    void fillLists(EventData& data) const;
    void erase(EventData& data) const;
    void production2Sp(EventData& data) const;
    void production3Sp(EventData& data) const;
    void production3Sp
    (EventData& data,
     std::list<InDet::SiSpacePointForSeed*>::iterator*,
     std::list<InDet::SiSpacePointForSeed*>::iterator*,
     std::list<InDet::SiSpacePointForSeed*>::iterator*,
     std::list<InDet::SiSpacePointForSeed*>::iterator*,
     int,int,int&,float) const;
     
    bool newVertices(EventData& data, const std::list<Trk::Vertex>&) const;
    void findNext(EventData& data) const;
    bool isZCompatible(EventData& data, float&,float&,float&) const;
    void convertToBeamFrameWork(EventData& data, const Trk::SpacePoint*const&,float*) const;
    bool isUsed(const Trk::SpacePoint*) const;

    EventData& getEventData() const;
  };

  MsgStream&    operator << (MsgStream&   ,const SiSpacePointsSeedMaker_LowMomentum&);
  std::ostream& operator << (std::ostream&,const SiSpacePointsSeedMaker_LowMomentum&);

} // end of name space

#endif // SiSpacePointsSeedMaker_LowMomentum_H
