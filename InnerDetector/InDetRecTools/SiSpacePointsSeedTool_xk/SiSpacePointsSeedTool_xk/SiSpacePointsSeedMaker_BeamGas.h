// -*- C++ -*-

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
//  Header file for class SiSpacePointsSeedMaker_BeamGas
/////////////////////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////////////////////
// Class for track candidates generation using space points information
// for standard Atlas geometry
/////////////////////////////////////////////////////////////////////////////////
// Version 1.0 3/10/2004 I.Gavrilenko
/////////////////////////////////////////////////////////////////////////////////

#ifndef SiSpacePointsSeedMaker_BeamGas_H
#define SiSpacePointsSeedMaker_BeamGas_H

#include "InDetRecToolInterfaces/ISiSpacePointsSeedMaker.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "BeamSpotConditionsData/BeamSpotData.h"
#include "MagFieldInterfaces/IMagFieldSvc.h"
#include "SiSpacePointsSeedTool_xk/SiSpacePointForSeed.h"
#include "TrkSpacePoint/SpacePointContainer.h" 
#include "TrkSpacePoint/SpacePointOverlapCollection.h"

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include <iosfwd>
#include <list>
#include <map>
#include <mutex>
#include <vector>

class MsgStream;

namespace Trk {
  class IPRD_AssociationTool;
}

namespace InDet {

  class SiSpacePointsSeedMaker_BeamGas : 
    virtual public ISiSpacePointsSeedMaker, public AthAlgTool
  {
    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
      
  public:
      
    ///////////////////////////////////////////////////////////////////
    // Standard tool methods
    ///////////////////////////////////////////////////////////////////

    SiSpacePointsSeedMaker_BeamGas
    (const std::string&, const std::string&, const IInterface*);
    virtual ~SiSpacePointsSeedMaker_BeamGas() = default;
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
    enum Size {SizeRF=53,
               SizeZ=11,
               SizeRFZ=SizeRF*SizeZ,
               SizeI=12};

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

    SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey{this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot"};

    // Properties, which will not be changed after construction
    BooleanProperty m_pixel{this, "usePixel", true};
    BooleanProperty m_sct{this, "useSCT""useSCT", true};
    BooleanProperty m_useOverlap{this, "useOverlapSpCollection", true};
    BooleanProperty m_useassoTool{this, "UseAssociationTool", true};
    IntegerProperty m_maxsize{this, "maxSize", 20000};
    IntegerProperty m_maxsizeSP{this, "maxSizeSP", 1500};
    IntegerProperty m_maxOneSize{this, "maxSeedsForSpacePoint", 5};
    FloatProperty m_drmax{this, "maxdRadius", 270.};
    FloatProperty m_zmin{this, "minZ", -5000.};
    FloatProperty m_zmax{this, "maxZ", +5000.};
    FloatProperty m_r_rmax{this, "radMax", 600.};
    FloatProperty m_r_rstep{this, "radStep", 2.};
    FloatProperty m_r1min{this, "minRadius1", 0.};
    FloatProperty m_r1max{this, "maxRadius1", 450.};
    FloatProperty m_r2min{this, "minRadius2", 0.};
    FloatProperty m_r2max{this, "maxRadius2", 600.};
    FloatProperty m_r3min{this, "minRadius3", 0.};
    FloatProperty m_r3max{this, "maxRadius3", 600.};
    FloatProperty m_drmin{this, "mindRadius", 10.};
    FloatProperty m_diver{this, "maxdImpact", 10.};
    FloatProperty m_diverpps{this, "maxdImpactPPS", 1.7};

    // Properties, which can be updated in buildFrameWork method which is called in initialize method
    FloatProperty m_ptmin{this, "pTmin", 1000.};
    FloatProperty m_rapcut{this, "RapidityCut", 5.3};

    // Properties, which are not used in this implementation of SiSpacePointsSeedMaker_BeamGas class
    FloatProperty m_dzver{this, "maxdZver", 5.};
    FloatProperty m_dzdrver{this, "maxdZdRver", 0.02};

    // Updated in only initialize
    bool m_initialized{false};
    int m_outputlevel{0};
    int m_r_size{0};
    int m_fNmax{0};
    int m_rfz_b[SizeRFZ];
    int m_rfz_t[SizeRFZ];
    int m_rfz_ib[SizeRFZ][SizeI];
    int m_rfz_it[SizeRFZ][SizeI];
    float m_dzdrmax{0.};
    float m_dzdrmin{0.};
    float m_COF{0.};
    float m_ipt{0.};
    float m_ipt2{0.};
    float m_sF{0.};

    mutable std::mutex m_mutex;
    mutable std::vector<EventContext::ContextEvt_t> m_cache ATLAS_THREAD_SAFE; // Guarded by m_mutex
    struct EventData { // To hold event dependent data
      bool endlist{true};
      int state{0};
      int nspoint{2};
      int nlist{0};
      int mode{0};
      int fNmin{0};
      int zMin{0};
      int nprint{0};
      int ns{0};
      int nsaz{0};
      int nr{0};
      int nrf{0};
      int nrfz{0};
      int nOneSeeds{0};
      int rf_index[SizeRF];
      int rf_map[SizeRF];
      int rfz_index[SizeRFZ];
      int rfz_map[SizeRFZ];
      float K{0.};

      ///////////////////////////////////////////////////////////////////
      // Beam geometry
      ///////////////////////////////////////////////////////////////////
      float xbeam[4]{0., 1., 0., 0.}; // x,ax,ay,az - center and x-axis direction
      float ybeam[4]{0., 0., 1., 0.}; // y,ax,ay,az - center and y-axis direction
      float zbeam[4]{0., 0., 0., 1.}; // z,ax,ay,az - center and z-axis direction

      std::vector<int> r_index;
      std::vector<int> r_map;
      std::vector<std::list<InDet::SiSpacePointForSeed*>> r_Sorted;
      std::vector<InDet::SiSpacePointsSeed> OneSeeds;
      std::multimap<float, InDet::SiSpacePointsSeed*> mapOneSeeds;
      std::list<InDet::SiSpacePointForSeed*> rf_Sorted[SizeRF];
      std::list<InDet::SiSpacePointForSeed*> rfz_Sorted[SizeRFZ];
      std::list<InDet::SiSpacePointForSeed> l_spforseed;
      std::list<InDet::SiSpacePointForSeed>::iterator i_spforseed;
      std::list<InDet::SiSpacePointForSeed*>::iterator rMin;
      std::list<InDet::SiSpacePointsSeed>           l_seeds;
      std::list<InDet::SiSpacePointsSeed>::iterator i_seed;
      std::list<InDet::SiSpacePointsSeed>::iterator i_seede;

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
    };
    mutable std::vector<EventData> m_eventData ATLAS_THREAD_SAFE; // Guarded by m_mutex

    ///////////////////////////////////////////////////////////////////
    // Private methods
    ///////////////////////////////////////////////////////////////////

    /**    @name Disallow default instantiation, copy, assignment */
    //@{
    SiSpacePointsSeedMaker_BeamGas() = delete;
    SiSpacePointsSeedMaker_BeamGas(const SiSpacePointsSeedMaker_BeamGas&) = delete;
    SiSpacePointsSeedMaker_BeamGas &operator=(const SiSpacePointsSeedMaker_BeamGas&) = delete;
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
     int,int,int&) const;

    void findNext(EventData& data) const;
    bool isZCompatible(float&) const;
    void convertToBeamFrameWork(EventData& data, const Trk::SpacePoint*const& sp, float* r) const;
    bool isUsed(const Trk::SpacePoint*) const;

    EventData& getEventData() const;
  };

  MsgStream&    operator << (MsgStream&   ,const SiSpacePointsSeedMaker_BeamGas&);
  std::ostream& operator << (std::ostream&,const SiSpacePointsSeedMaker_BeamGas&);

} // end of name space

#endif // SiSpacePointsSeedMaker_BeamGas_H
