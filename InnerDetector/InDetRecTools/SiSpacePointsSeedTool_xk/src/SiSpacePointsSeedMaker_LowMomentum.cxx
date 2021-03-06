/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   Implementation file for class SiSpacePointsSeedMaker_LowMomentum
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// AlgTool used for TRT_DriftCircleOnTrack object production
///////////////////////////////////////////////////////////////////
// Version 1.0 21/04/2004 I.Gavrilenko
///////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>

#include "TrkToolInterfaces/IPRD_AssociationTool.h"
#include "SiSpacePointsSeedTool_xk/SiSpacePointsSeedMaker_LowMomentum.h"

///////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////

InDet::SiSpacePointsSeedMaker_LowMomentum::SiSpacePointsSeedMaker_LowMomentum
(const std::string& t,const std::string& n,const IInterface* p)
  : AthAlgTool(t,n,p),
    m_fieldServiceHandle("AtlasFieldSvc",n), 
    m_spacepointsSCT("SCT_SpacePoints"),
    m_spacepointsPixel("PixelSpacePoints"),
    m_spacepointsOverlap("OverlapSpacePoints"),
    m_assoTool("InDet::InDetPRD_AssociationToolGangedPixels")
{
  m_useassoTool = true    ;
  m_useOverlap  = true    ;
  m_state       = 0       ;
  m_pixel       = true    ;
  m_sct         = true    ;
  m_nspoint     = 2       ;
  m_mode        = 0       ;
  m_nlist       = 0       ;
  m_endlist     = true    ;
  m_maxsize     = 2000    ;
  m_ptmin       = 100.    ;
  m_ptmax       =  500.   ;
  m_r1min       = 0.      ; 
  m_r1max       = 600.    ; 
  m_r2min       = 0.      ; 
  m_r2max       = 600.    ; 
  m_r3min       = 0.      ;
  m_r3max       = 600.    ;
  m_drmin       = 10.     ; 
  m_drmax       = 200.    ;    
  m_rapcut      = 2.7     ;
  m_zmin        = -250.   ;
  m_zmax        = +250.   ;
  m_dzver       = 5.      ;
  m_dzdrver     = .02     ;
  m_diver       =  7.     ;
  m_diverpps    =  1.2    ;
  m_dazmax      = .02     ;
  m_iptmax      = 1./400. ;
  r_rmax        = 200.    ;
  r_rstep       =  2.     ;
  r_Sorted      = 0       ;
  r_index       = 0       ;
  r_map         = 0       ;    
  m_maxsizeSP   = 1500    ;
  m_maxOneSize  = 5       ;
  m_SP          = 0       ;
  m_R           = 0       ;
  m_Tz          = 0       ;
  m_Er          = 0       ;
  m_U           = 0       ;
  m_V           = 0       ;
  m_Zo          = 0       ;
  m_OneSeeds    = 0       ;

  m_xbeam[0]    = 0.      ; m_xbeam[1]= 1.; m_xbeam[2]=0.; m_xbeam[3]=0.;
  m_ybeam[0]    = 0.      ; m_ybeam[1]= 0.; m_ybeam[2]=1.; m_ybeam[3]=0.;
  m_zbeam[0]    = 0.      ; m_zbeam[1]= 0.; m_zbeam[2]=0.; m_zbeam[3]=1.;
  
//  m_spacepointsSCTname     = "SCT_SpacePoints"   ;
//  m_spacepointsPixelname   = "PixelSpacePoints"  ;
//  m_spacepointsOverlapname = "OverlapSpacePoints"; 
  m_beamconditions         = "BeamCondSvc"       ;
//  m_spacepointsSCT         = 0                   ;
//  m_spacepointsPixel       = 0                   ;
//  m_spacepointsOverlap     = 0                   ;

  declareInterface<ISiSpacePointsSeedMaker>(this);

  declareProperty("AssociationTool"       ,m_assoTool              );
  declareProperty("usePixel"              ,m_pixel                 );
  declareProperty("useSCT"                ,m_sct                   );
  declareProperty("pTmin"                 ,m_ptmin                 );
  declareProperty("pTmax"                 ,m_ptmax                 );
  declareProperty("radMax"                ,r_rmax                  );
  declareProperty("radStep"               ,r_rstep                 );
  declareProperty("maxSize"               ,m_maxsize               );
  declareProperty("maxSizeSP"             ,m_maxsizeSP             );
  declareProperty("minZ"                  ,m_zmin                  );
  declareProperty("maxZ"                  ,m_zmax                  );
  declareProperty("minRadius1"            ,m_r1min                 );
  declareProperty("minRadius2"            ,m_r2min                 );
  declareProperty("minRadius3"            ,m_r3min                 );
  declareProperty("maxRadius1"            ,m_r1max                 );
  declareProperty("maxRadius2"            ,m_r2max                 );
  declareProperty("maxRadius3"            ,m_r3max                 );
  declareProperty("mindRadius"            ,m_drmin                 );
  declareProperty("maxdRadius"            ,m_drmax                 );
  declareProperty("RapidityCut"           ,m_rapcut                );
  declareProperty("maxdZver"              ,m_dzver                 );
  declareProperty("maxdZdRver"            ,m_dzdrver               );
  declareProperty("maxdImpact"            ,m_diver                 );
  declareProperty("maxdImpactPPS"         ,m_diverpps              );
  declareProperty("maxSeedsForSpacePoint" ,m_maxOneSize            );
  declareProperty("SpacePointsSCTName"    ,m_spacepointsSCT    );
  declareProperty("SpacePointsPixelName"  ,m_spacepointsPixel  );
  declareProperty("SpacePointsOverlapName",m_spacepointsOverlap);
  declareProperty("BeamConditionsService" ,m_beamconditions        ); 
  declareProperty("useOverlapSpCollection", m_useOverlap           );
  declareProperty("UseAssociationTool"    ,m_useassoTool           ); 
  declareProperty("MagFieldSvc"           , m_fieldServiceHandle   );
}

///////////////////////////////////////////////////////////////////
// Destructor  
///////////////////////////////////////////////////////////////////

InDet::SiSpacePointsSeedMaker_LowMomentum::~SiSpacePointsSeedMaker_LowMomentum()
{
  if(r_index ) delete [] r_index ;
  if(r_map   ) delete [] r_map   ; 
  if(r_Sorted) delete [] r_Sorted;

  // Delete seeds
  //
  for(i_seed=l_seeds.begin(); i_seed!=l_seeds.end (); ++i_seed) {
    delete *i_seed;
  }
  // Delete space points for reconstruction
  //
  i_spforseed=l_spforseed.begin();
  for(; i_spforseed!=l_spforseed.end(); ++i_spforseed) {
    delete *i_spforseed;
  } 

  if(m_SP) delete [] m_SP;
  if(m_R ) delete [] m_R ;
  if(m_Tz) delete [] m_Tz;
  if(m_Er) delete [] m_Er;
  if(m_U ) delete [] m_U ;
  if(m_V ) delete [] m_V ;
  if(m_Zo) delete [] m_Zo;
  if(m_OneSeeds) delete [] m_OneSeeds;
 }

///////////////////////////////////////////////////////////////////
// Initialisation
///////////////////////////////////////////////////////////////////

StatusCode InDet::SiSpacePointsSeedMaker_LowMomentum::initialize()
{
  StatusCode sc = AlgTool::initialize(); 

  // Get beam geometry
  //
  p_beam = 0;
  if(m_beamconditions!="") {
    sc = service(m_beamconditions,p_beam);
  }

  // Get magnetic field service
  //
  if( !m_fieldServiceHandle.retrieve() ){
    ATH_MSG_FATAL("Failed to retrieve " << m_fieldServiceHandle );
    return StatusCode::FAILURE;
  }    
  ATH_MSG_DEBUG("Retrieved " << m_fieldServiceHandle );
  m_fieldService = &*m_fieldServiceHandle;

  // Get tool for track-prd association
  //
  if( m_useassoTool ) {
    if( m_assoTool.retrieve().isFailure()) {
      msg(MSG::FATAL)<<"Failed to retrieve tool "<< m_assoTool<<endreq; 
      return StatusCode::FAILURE;
    } else {
      msg(MSG::INFO) << "Retrieved tool " << m_assoTool << endreq;
    }
  }

  // Build framework
  //
  buildFrameWork();

  // Get output print level
  //
  m_outputlevel = msg().level()-MSG::DEBUG;
  if(m_outputlevel<=0) {
    m_nprint=0; msg(MSG::DEBUG)<<(*this)<<endreq;
  }
  return sc;
}

///////////////////////////////////////////////////////////////////
// Finalize
///////////////////////////////////////////////////////////////////

StatusCode InDet::SiSpacePointsSeedMaker_LowMomentum::finalize()
{
   StatusCode sc = AlgTool::finalize(); return sc;
}

///////////////////////////////////////////////////////////////////
// Initialize tool for new event 
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::newEvent (int)
{
  m_trigger = false;
  if(!m_pixel && !m_sct) return; erase();
  i_spforseed   = l_spforseed.begin();
  buildBeamFrameWork();

  float irstep = 1./r_rstep;
  int   irmax  = r_size-1  ;

  // Get pixels space points containers from store gate 
  //
  if(m_pixel) {

//    m_spacepointsPixel = 0;
//    StatusCode sc = evtStore()->retrieve(m_spacepointsPixel,m_spacepointsPixelname);
    
    if(m_spacepointsPixel.isValid()) {

      SpacePointContainer::const_iterator spc  =  m_spacepointsPixel->begin();
      SpacePointContainer::const_iterator spce =  m_spacepointsPixel->end  ();

      for(; spc != spce; ++spc) {

	SpacePointCollection::const_iterator sp  = (*spc)->begin();
	SpacePointCollection::const_iterator spe = (*spc)->end  ();
    
	for(; sp != spe; ++sp) {

	  float r = (*sp)->r(); if(r<0. || r>=r_rmax) continue;
	  if(m_useassoTool &&  isUsed(*sp)           ) continue;

	  InDet::SiSpacePointForSeed* sps = newSpacePoint((*sp)); 

	  int   ir = int(sps->radius()*irstep); if(ir>irmax) ir = irmax;
	  r_Sorted[ir].push_back(sps); ++r_map[ir];
	  if(r_map[ir]==1) r_index[m_nr++] = ir;
	  ++m_ns;
	}
      }
    }
  }

  // Get sct space points containers from store gate 
  //
  if(m_sct) {

//    m_spacepointsSCT = 0;
//    StatusCode sc = evtStore()->retrieve(m_spacepointsSCT,m_spacepointsSCTname);
    if(m_spacepointsSCT.isValid()) {

      SpacePointContainer::const_iterator spc  =  m_spacepointsSCT->begin();
      SpacePointContainer::const_iterator spce =  m_spacepointsSCT->end  ();

      for(; spc != spce; ++spc) {

	SpacePointCollection::const_iterator sp  = (*spc)->begin();
	SpacePointCollection::const_iterator spe = (*spc)->end  ();
    
	for(; sp != spe; ++sp) {

	  float r = (*sp)->r(); if(r<0. || r>=r_rmax) continue;
	  if(m_useassoTool &&  isUsed(*sp)           ) continue;

	  InDet::SiSpacePointForSeed* sps = newSpacePoint((*sp)); 

	  int   ir = int(sps->radius()*irstep); if(ir>irmax) ir = irmax;
	  r_Sorted[ir].push_back(sps); ++r_map[ir];
	  if(r_map[ir]==1) r_index[m_nr++] = ir;
	  ++m_ns;
	}
      }
    }
  }
  fillLists();
}

///////////////////////////////////////////////////////////////////
// Initialize tool for new region
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::newRegion
(const std::vector<IdentifierHash>& vPixel, const std::vector<IdentifierHash>& vSCT)
{
  m_trigger = false;
  if(!m_pixel && !m_sct) return; erase();
  i_spforseed = l_spforseed.begin();
  buildBeamFrameWork();

  int   irmax  = r_size-1  ;
  float irstep = 1./r_rstep;

  // Get pixels space points containers from store gate 
  //
  if(m_pixel && vPixel.size()) {

//    m_spacepointsPixel   = 0;
//    StatusCode sc = evtStore()->retrieve(m_spacepointsPixel,m_spacepointsPixelname);
    
    if(m_spacepointsPixel.isValid()) {

      SpacePointContainer::const_iterator spce =  m_spacepointsPixel->end  ();

      std::vector<IdentifierHash>::const_iterator l = vPixel.begin(), le = vPixel.end();

      // Loop through all trigger collections
      //
      for(; l!=le; ++l) {
	
	SpacePointContainer::const_iterator  w =  m_spacepointsPixel->indexFind((*l));
	if(w==spce) continue;
	SpacePointCollection::const_iterator sp = (*w)->begin(), spe = (*w)->end();

	for(; sp != spe; ++sp) {

	  float r = (*sp)->r(); if(r<0. || r>=r_rmax) continue;
	  if(m_useassoTool &&  isUsed(*sp)           ) continue;

	  InDet::SiSpacePointForSeed* sps = newSpacePoint((*sp)); 

	  int   ir = int(sps->radius()*irstep); if(ir>irmax) ir = irmax;
	  r_Sorted[ir].push_back(sps); ++r_map[ir];
	  if(r_map[ir]==1) r_index[m_nr++] = ir;
	  ++m_ns;
	}
      }
    }
  }

  // Get sct space points containers from store gate 
  //
  if(m_sct && vSCT.size()) {

//    m_spacepointsSCT     = 0;
//    StatusCode sc = evtStore()->retrieve(m_spacepointsSCT,m_spacepointsSCTname);

    if(m_spacepointsSCT.isValid()) {

      SpacePointContainer::const_iterator spce =  m_spacepointsSCT->end  ();

      std::vector<IdentifierHash>::const_iterator l = vSCT.begin(), le = vSCT.end();

      // Loop through all trigger collections
      //
      for(; l!=le; ++l) {

	SpacePointContainer::const_iterator  w =  m_spacepointsSCT->indexFind((*l));
	if(w==spce) continue;
	SpacePointCollection::const_iterator sp = (*w)->begin(), spe = (*w)->end();

	for(; sp != spe; ++sp) {

	  float r = (*sp)->r(); if(r<0. || r>=r_rmax) continue;
	  if(m_useassoTool &&  isUsed(*sp)           ) continue;

	  InDet::SiSpacePointForSeed* sps = newSpacePoint((*sp)); 

	  int   ir = int(sps->radius()*irstep); if(ir>irmax) ir = irmax;
	  r_Sorted[ir].push_back(sps); ++r_map[ir];
	  if(r_map[ir]==1) r_index[m_nr++] = ir;
	  ++m_ns;
	}
      }
    }
  }
  fillLists();
}

///////////////////////////////////////////////////////////////////
// Initialize tool for new region
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::newRegion
(const std::vector<IdentifierHash>& vPixel, const std::vector<IdentifierHash>& vSCT,const IRoiDescriptor&) 
{
  newRegion(vPixel,vSCT);
}

///////////////////////////////////////////////////////////////////
// Methods to initilize different strategies of seeds production
// with two space points with or without vertex constraint
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::find2Sp(const std::list<Trk::Vertex>& lv) 
{
  int mode; lv.begin()!=lv.end() ?  mode = 1 : mode = 0;
  bool newv = newVertices(lv);
  
  if(newv || !m_state || m_nspoint!=2 || m_mode!=mode || m_nlist) {

    i_seede   = l_seeds.begin();
    m_state   = 1   ;
    m_nspoint = 2   ;
    m_nlist   = 0   ;
    m_mode    = mode;
    m_endlist = true;
    m_fNmin   = 0   ;
    m_zMin    = 0   ;
    production2Sp ();
  }
  i_seed  = l_seeds.begin();
  
  if(m_outputlevel<=0) {
    m_nprint=1; msg(MSG::DEBUG)<<(*this)<<endreq;
  }
}

///////////////////////////////////////////////////////////////////
// Methods to initilize different strategies of seeds production
// with three space points with or without vertex constraint
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::find3Sp(const std::list<Trk::Vertex>& lv) 
{
  int mode; lv.begin()!=lv.end() ? mode = 3 : mode = 2; 
  bool newv = newVertices(lv);

  if(newv || !m_state || m_nspoint!=3 || m_mode!=mode || m_nlist) {

    i_seede   = l_seeds.begin() ;
    m_state   = 1               ;
    m_nspoint = 3               ;
    m_nlist   = 0               ;
    m_mode    = mode            ;
    m_endlist = true            ;
    m_fNmin   = 0               ;
    m_zMin    = 0               ;
    production3Sp   ();
  }
  i_seed  = l_seeds.begin();

  if(m_outputlevel<=0) {
    m_nprint=1; msg(MSG::DEBUG)<<(*this)<<endreq;
  }
}

void InDet::SiSpacePointsSeedMaker_LowMomentum::find3Sp(const std::list<Trk::Vertex>& lv,const double*) 
{
  find3Sp(lv);
}

///////////////////////////////////////////////////////////////////
// Methods to initilize different strategies of seeds production
// with variable number space points with or without vertex constraint
// Variable means (2,3,4,....) any number space points
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::findVSp (const std::list<Trk::Vertex>& lv)
{
  int mode; lv.begin()!=lv.end() ? mode = 6 : mode = 5;
  bool newv = newVertices(lv);
  
  if(newv || !m_state || m_nspoint!=4 || m_mode!=mode || m_nlist) {

    i_seede   = l_seeds.begin() ;
    m_state   = 1               ;
    m_nspoint = 4               ;
    m_nlist   = 0               ;
    m_mode    = mode            ;
    m_endlist = true            ;
    m_fNmin   = 0               ;
    m_zMin    = 0               ;
    production3Sp();
  }
  i_seed  = l_seeds.begin();

  if(m_outputlevel<=0) {
    m_nprint=1; msg(MSG::DEBUG)<<(*this)<<endreq;
  }
}

///////////////////////////////////////////////////////////////////
// Dumps relevant information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::SiSpacePointsSeedMaker_LowMomentum::dump( MsgStream& out ) const
{
  if(m_nprint)  return dumpEvent(out); return dumpConditions(out);
}

///////////////////////////////////////////////////////////////////
// Dumps conditions information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::SiSpacePointsSeedMaker_LowMomentum::dumpConditions( MsgStream& out ) const
{
  int n = 42-m_spacepointsPixel.name().size();
  std::string s2; for(int i=0; i<n; ++i) s2.append(" "); s2.append("|");
  n     = 42-m_spacepointsSCT.name().size();
  std::string s3; for(int i=0; i<n; ++i) s3.append(" "); s3.append("|");
  n     = 42-m_spacepointsOverlap.name().size();
  std::string s4; for(int i=0; i<n; ++i) s4.append(" "); s4.append("|");
  n     = 42-m_beamconditions.size();
  std::string s5; for(int i=0; i<n; ++i) s5.append(" "); s5.append("|");

  out<<"|---------------------------------------------------------------------|"
     <<std::endl;
  out<<"| Pixel    space points   | "<<m_spacepointsPixel.name() <<s2
     <<std::endl;
  out<<"| SCT      space points   | "<<m_spacepointsSCT.name()<<s3
     <<std::endl;
  out<<"| Overlap  space points   | "<<m_spacepointsOverlap.name()<<s4
     <<std::endl;
  out<<"| BeamConditionsService   | "<<m_beamconditions<<s5
     <<std::endl;
  out<<"| usePixel                | "
     <<std::setw(12)<<m_pixel 
     <<"                              |"<<std::endl;
  out<<"| useSCT                  | "
     <<std::setw(12)<<m_sct 
     <<"                              |"<<std::endl;
  out<<"| Use association tool ?  | "
     <<std::setw(12)<<m_useassoTool 
     <<"                              |"<<std::endl;
  out<<"| maxSize                 | "
     <<std::setw(12)<<m_maxsize 
     <<"                              |"<<std::endl;
  out<<"| maxSizeSP               | "
     <<std::setw(12)<<m_maxsizeSP
     <<"                              |"<<std::endl;
  out<<"| pTmin  (mev)            | "
     <<std::setw(12)<<std::setprecision(5)<<m_ptmin
     <<"                              |"<<std::endl;
  out<<"| pTmax  (mev)            | "
     <<std::setw(12)<<std::setprecision(5)<<m_ptmax
     <<"                              |"<<std::endl;
  out<<"| |rapidity|          <=  | " 
     <<std::setw(12)<<std::setprecision(5)<<m_rapcut
     <<"                              |"<<std::endl;
  out<<"| max radius SP           | "
     <<std::setw(12)<<std::setprecision(5)<<r_rmax 
     <<"                              |"<<std::endl;
  out<<"| radius step             | "
     <<std::setw(12)<<std::setprecision(5)<<r_rstep
     <<"                              |"<<std::endl;
  out<<"| min Z-vertex position   | "
     <<std::setw(12)<<std::setprecision(5)<<m_zmin
     <<"                              |"<<std::endl;
  out<<"| max Z-vertex position   | "
     <<std::setw(12)<<std::setprecision(5)<<m_zmax
     <<"                              |"<<std::endl;
  out<<"| min radius first  SP(3) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r1min
     <<"                              |"<<std::endl;
  out<<"| min radius second SP(3) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r2min
     <<"                              |"<<std::endl;
  out<<"| min radius last   SP(3) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r3min
     <<"                              |"<<std::endl;
  out<<"| max radius first  SP(3) | "
     <<std::setw(12)<<std::setprecision(4)<<m_r1max
     <<"                              |"<<std::endl;
  out<<"| max radius second SP(3) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r2max
     <<"                              |"<<std::endl;
  out<<"| max radius last   SP(3) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r3max
     <<"                              |"<<std::endl;
  out<<"| min space points dR     | "
     <<std::setw(12)<<std::setprecision(5)<<m_drmin
     <<"                              |"<<std::endl;
  out<<"| max space points dR     | "
     <<std::setw(12)<<std::setprecision(5)<<m_drmax
     <<"                              |"<<std::endl;
  out<<"| max dZ    impact        | "
     <<std::setw(12)<<std::setprecision(5)<<m_dzver 
     <<"                              |"<<std::endl;
  out<<"| max dZ/dR impact        | "
     <<std::setw(12)<<std::setprecision(5)<<m_dzdrver 
     <<"                              |"<<std::endl;
  out<<"| max       impact        | "
     <<std::setw(12)<<std::setprecision(5)<<m_diver
     <<"                              |"<<std::endl;
  out<<"| max       impact pps    | "
     <<std::setw(12)<<std::setprecision(5)<<m_diverpps
     <<"                              |"<<std::endl;
  out<<"|---------------------------------------------------------------------|"
     <<std::endl;
  out<<"| Beam X center           | "
     <<std::setw(12)<<std::setprecision(5)<<m_xbeam[0]
     <<"                              |"<<std::endl;
  out<<"| Beam Y center           | "
     <<std::setw(12)<<std::setprecision(5)<<m_ybeam[0]
     <<"                              |"<<std::endl;
  out<<"| Beam Z center           | "
     <<std::setw(12)<<std::setprecision(5)<<m_zbeam[0]
     <<"                              |"<<std::endl;
  out<<"| Beam X-axis direction   | "
     <<std::setw(12)<<std::setprecision(5)<<m_xbeam[1]
     <<std::setw(12)<<std::setprecision(5)<<m_xbeam[2]
     <<std::setw(12)<<std::setprecision(5)<<m_xbeam[3]
     <<"      |"<<std::endl;
  out<<"| Beam Y-axis direction   | "
     <<std::setw(12)<<std::setprecision(5)<<m_ybeam[1]
     <<std::setw(12)<<std::setprecision(5)<<m_ybeam[2]
     <<std::setw(12)<<std::setprecision(5)<<m_ybeam[3]
     <<"      |"<<std::endl;
  out<<"| Beam Z-axis direction   | "
     <<std::setw(12)<<std::setprecision(5)<<m_zbeam[1]
     <<std::setw(12)<<std::setprecision(5)<<m_zbeam[2]
     <<std::setw(12)<<std::setprecision(5)<<m_zbeam[3]
     <<"      |"<<std::endl;
  out<<"|---------------------------------------------------------------------|"
     <<std::endl;
  return out;
}

///////////////////////////////////////////////////////////////////
// Dumps event information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::SiSpacePointsSeedMaker_LowMomentum::dumpEvent( MsgStream& out ) const
{
  //const float pi2    = 2.*M_PI;
  out<<"|---------------------------------------------------------------------|"
     <<std::endl;
  out<<"| m_ns                    | "
     <<std::setw(12)<<m_ns
     <<"                              |"<<std::endl;
  out<<"| m_nsaz                  | "
     <<std::setw(12)<<m_nsaz
     <<"                              |"<<std::endl;
  out<<"| seeds                   | "
     <<std::setw(12)<<l_seeds.size()
     <<"                              |"<<std::endl;
  out<<"|---------------------------------------------------------------------|"
     <<std::endl;
  /*
  if(m_outputlevel==0) return out; 

  out<<"|-------------|--------|-------|-------|-------|-------|-------|";
  out<<"-------|-------|-------|-------|-------|-------|"
     <<std::endl;

  out<<"|  Azimuthal  |    n   | z[ 0] | z[ 1] | z[ 2] | z[ 3] | z[4]  |";
  out<<" z[ 5] | z[ 6] | z[ 7] | z[ 8] | z[ 9] | z[10] |"
     <<std::endl;
  out<<"|-------------|--------|-------|-------|-------|-------|-------|";
  out<<"-------|-------|-------|-------|-------|-------|"
     <<std::endl;
  
  float sF1 = pi2/float(m_fNmax+1);
  
  
  for(int f=0; f<=m_fNmax; ++f) {
    out<<"|  "
       <<std::setw(10)<<std::setprecision(4)<<sF1*float(f)<<" | "
       <<std::setw(6)<<rf_map[f]<<" |";
    for(int z=0; z!=11; ++z) {
      out<<std::setw(6)<<rfz_map[(f*11+z)]<<" |";
    }
    out<<std::endl;
  } 
  out<<"|-------------|--------|-------|-------|-------|-------|-------|";
  out<<"-------|-------|-------|-------|-------|-------|"
     <<std::endl;
  out<<std::endl;
  */
  return out;
}

///////////////////////////////////////////////////////////////////
// Dumps relevant information into the ostream
///////////////////////////////////////////////////////////////////

std::ostream& InDet::SiSpacePointsSeedMaker_LowMomentum::dump( std::ostream& out ) const
{
  return out;
}

///////////////////////////////////////////////////////////////////
// Overload of << operator MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::operator    << 
  (MsgStream& sl,const InDet::SiSpacePointsSeedMaker_LowMomentum& se)
{ 
  return se.dump(sl); 
}

///////////////////////////////////////////////////////////////////
// Overload of << operator std::ostream
///////////////////////////////////////////////////////////////////

std::ostream& InDet::operator << 
  (std::ostream& sl,const InDet::SiSpacePointsSeedMaker_LowMomentum& se)
{ 
  return se.dump(sl); 
}   

///////////////////////////////////////////////////////////////////
// Find next set space points
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::findNext () 
{
  if(m_endlist) return;
  
  i_seede = l_seeds.begin();
  if     (m_mode==0 || m_mode==1) production2Sp ();
  else if(m_mode==2 || m_mode==3) {

    production3Sp   ();

  }
  else if(m_mode==5 || m_mode==6) {

    production3Sp   ();

  }

  i_seed  = l_seeds.begin();
  ++m_nlist;
}                       

///////////////////////////////////////////////////////////////////
// New and old list vertices comparison
///////////////////////////////////////////////////////////////////

bool InDet::SiSpacePointsSeedMaker_LowMomentum::newVertices(const std::list<Trk::Vertex>& lV)
{
  unsigned int s1 = l_vertex.size(); 
  unsigned int s2 = lV      .size(); 

  if(s1==0 && s2==0) return false;

  std::list<Trk::Vertex>::const_iterator v;
  l_vertex.erase(l_vertex.begin(),l_vertex.end());
  
  for(v=lV.begin(); v!=lV.end(); ++v) {
    l_vertex.push_back(float((*v).position().z()));
  }
  return false;
}

///////////////////////////////////////////////////////////////////
// Initiate frame work for seed generator
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::buildFrameWork() 
{
  m_ptmin     = fabs(m_ptmin)                  ;  if(m_ptmin < 50.) m_ptmin = 50.;
  m_iptmax  = 1./fabs(m_ptmax)                 ;
  m_iptmin  = 1./fabs(m_ptmin)                 ;
  m_rapcut    = fabs(m_rapcut)                 ;
  m_dzdrmax   = 1./tan(2.*atan(exp(-m_rapcut))); 
  m_dzdrmin   =-m_dzdrmax                      ;
  m_r3max     = r_rmax                         ; 

  m_ns = m_nsaz = m_nr = m_nrfz = 0;

  // Build radius sorted containers
  //
  r_size = int((r_rmax+.1)/r_rstep);
  r_Sorted = new std::list<InDet::SiSpacePointForSeed*>[r_size];
  r_index  = new int[r_size];
  r_map    = new int[r_size];  
  m_nr   = 0; for(int i=0; i!=r_size; ++i) {r_index[i]=0; r_map[i]=0;}

  // Build radius-azimuthal sorted containers
  //
  const float pi2     = 2.*M_PI            ;
  const int   NFmax    = 20                ;
  const float sFmax   = float(NFmax )/pi2;
  const float sFmin   = 100./60.          ;

  m_sF        = m_ptmin /60. ; 
  if(m_sF    >sFmax ) m_sF    = sFmax  ; else if(m_sF < sFmin) m_sF = sFmin;
  m_fNmax     = int(pi2*m_sF); if(m_fNmax >=NFmax) m_fNmax = NFmax-1;

  // Build radius-azimuthal-Z sorted containers
  //
  m_nrfz  = 0; for(int i=0; i!=220; ++i) {rfz_index [i]=0; rfz_map [i]=0;}


  // Build maps for radius-azimuthal-Z sorted collections 
  //
  for(int f=0; f<=m_fNmax; ++f) {

    int fb = f-1; if(fb<0      ) fb=m_fNmax; 
    int ft = f+1; if(ft>m_fNmax) ft=0; 
    
    // For each azimuthal region loop through all Z regions
    //
    for(int z=0; z!=11; ++z) {
 
      int a        = f *11+z;
      int b        = fb*11+z;
      int c        = ft*11+z;
      rfz_b [a]    = 3; rfz_t [a]    = 3;
      rfz_ib[a][0] = a; rfz_it[a][0] = a; 
      rfz_ib[a][1] = b; rfz_it[a][1] = b; 
      rfz_ib[a][2] = c; rfz_it[a][2] = c; 
      if     (z==5) {

	rfz_t [a]    = 9 ;
	rfz_it[a][3] = a+1; 
	rfz_it[a][4] = b+1; 
	rfz_it[a][5] = c+1; 
	rfz_it[a][6] = a-1; 
	rfz_it[a][7] = b-1; 
	rfz_it[a][8] = c-1; 
      }
      else if(z> 5) {

	rfz_b [a]    = 6 ;
	rfz_ib[a][3] = a-1; 
	rfz_ib[a][4] = b-1; 
	rfz_ib[a][5] = c-1; 

	if(z<10) {

	  rfz_t [a]    = 6 ;
	  rfz_it[a][3] = a+1; 
	  rfz_it[a][4] = b+1; 
	  rfz_it[a][5] = c+1; 
	}
      }
      else {

	rfz_b [a]    = 6 ;
	rfz_ib[a][3] = a+1; 
	rfz_ib[a][4] = b+1; 
	rfz_ib[a][5] = c+1; 

	if(z>0) {

	  rfz_t [a]    = 6 ;
	  rfz_it[a][3] = a-1; 
	  rfz_it[a][4] = b-1; 
	  rfz_it[a][5] = c-1; 
	}
      }

      if     (z==3) {
	rfz_b[a]      = 9;
	rfz_ib[a][6] = a+2; 
	rfz_ib[a][7] = b+2; 
	rfz_ib[a][8] = c+2; 
      }
      else if(z==7) {
	rfz_b[a]      = 9;
	rfz_ib[a][6] = a-2; 
	rfz_ib[a][7] = b-2; 
	rfz_ib[a][8] = c-2; 
      }
    }
  }
  
  if(!m_SP) m_SP   = new InDet::SiSpacePointForSeed*[m_maxsizeSP];
  if(!m_R ) m_R    = new                      float[m_maxsizeSP];
  if(!m_Tz) m_Tz   = new                      float[m_maxsizeSP];
  if(!m_Er) m_Er   = new                      float[m_maxsizeSP];
  if(!m_U ) m_U    = new                      float[m_maxsizeSP]; 
  if(!m_V ) m_V    = new                      float[m_maxsizeSP];
  if(!m_Zo) m_Zo   = new                      float[m_maxsizeSP];
  if(!m_OneSeeds) m_OneSeeds  = new InDet::SiSpacePointsSeed [m_maxOneSize];  

  i_seed  = l_seeds.begin();
  i_seede = l_seeds.end  ();
}

///////////////////////////////////////////////////////////////////
// Initiate beam frame work for seed generator
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::buildBeamFrameWork() 
{ 
  if(!p_beam) return;

  Amg::Vector3D cb =     p_beam->beamPos();
  double     tx = tan(p_beam->beamTilt(0));
  double     ty = tan(p_beam->beamTilt(1));

  double ph   = atan2(ty,tx);
  double th   = acos(1./sqrt(1.+tx*tx+ty*ty));
  double sint = sin(th);
  double cost = cos(th);
  double sinp = sin(ph);
  double cosp = cos(ph);
  
  m_xbeam[0] = float(cb.x())                  ; 
  m_xbeam[1] = float(cost*cosp*cosp+sinp*sinp);
  m_xbeam[2] = float(cost*sinp*cosp-sinp*cosp);
  m_xbeam[3] =-float(sint*cosp               );
  
  m_ybeam[0] = float(cb.y())                  ; 
  m_ybeam[1] = float(cost*cosp*sinp-sinp*cosp);
  m_ybeam[2] = float(cost*sinp*sinp+cosp*cosp);
  m_ybeam[3] =-float(sint*sinp               );
  
  m_zbeam[0] = float(cb.z())                  ; 
  m_zbeam[1] = float(sint*cosp)               ;
  m_zbeam[2] = float(sint*sinp)               ;
  m_zbeam[3] = float(cost)                    ;
}

///////////////////////////////////////////////////////////////////
// Initiate beam frame work for seed generator
///////////////////////////////////////////////////////////////////

void  InDet::SiSpacePointsSeedMaker_LowMomentum::convertToBeamFrameWork
(Trk::SpacePoint*const& sp,float* r) 
{
  
  r[0] = float(sp->globalPosition().x())-m_xbeam[0];
  r[1] = float(sp->globalPosition().y())-m_ybeam[0];
  r[2] = float(sp->globalPosition().z())-m_zbeam[0];
}
   
///////////////////////////////////////////////////////////////////
// Initiate space points seed maker
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::fillLists() 
{
  const float pi2 = 2.*M_PI;
  std::list<InDet::SiSpacePointForSeed*>::iterator r;
  
  for(int i=0; i!= r_size;  ++i) {

    if(!r_map[i]) continue; r = r_Sorted[i].begin();

    while(r!=r_Sorted[i].end()) {
      
      // Azimuthal angle sort
      //
      float F = (*r)->phi(); if(F<0.) F+=pi2;

      int   f = int(F*m_sF); f<0 ? f = m_fNmax : f>m_fNmax ? f = 0 : f=f;

      int z; float Z = (*r)->z();

      // Azimuthal angle and Z-coordinate sort
      //
      if(Z>0.) {
	Z< 250.?z=5:Z< 450.?z=6:Z< 925.?z=7:Z< 1400.?z=8:Z< 2500.?z=9:z=10;
      }
      else     {
	Z>-250.?z=5:Z>-450.?z=4:Z>-925.?z=3:Z>-1400.?z=2:Z>-2500.?z=1:z= 0;
      }
      int n = f*11+z; ++m_nsaz;
      rfz_Sorted[n].push_back(*r); if(!rfz_map[n]++) rfz_index[m_nrfz++] = n;
      r_Sorted[i].erase(r++);
    }
    r_map[i] = 0;
  }
  m_nr    = 0;
  m_state = 0;
}
 
///////////////////////////////////////////////////////////////////
// Erase space point information
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::erase()
{
  for(int i=0; i!=m_nr;    ++i) {
    int n = r_index[i]; r_map[n] = 0;
    r_Sorted[n].clear();
  }

  for(int i=0; i!=m_nrfz;  ++i) {
    int n = rfz_index[i]; rfz_map[n] = 0;
    rfz_Sorted[n].clear();
  }

  m_state = 0;
  m_ns    = 0;
  m_nsaz  = 0;
  m_nr    = 0;
  m_nrfz  = 0;
}

///////////////////////////////////////////////////////////////////
// 2 space points seeds production
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::production2Sp()
{
  m_endlist = true;
}

///////////////////////////////////////////////////////////////////
// Production 3 space points seeds (new version)
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::production3Sp()
{ 
  if(m_nsaz<3) return;

  float K = 0.;
  double f[3], gP[3] ={10.,10.,0.};

  if(m_fieldService->solenoidOn()) {
    m_fieldService->getFieldZR(gP,f); K = 2./(300.*f[2]);
  }
  else K = 2./(300.* 5. );

  const int   ZI[11]= {5,6,7,8,9,10,4,3,2,1,0};
  std::list<InDet::SiSpacePointForSeed*>::iterator rt[9],rte[9],rb[9],rbe[9];
  int nseed = 0; 

  // Loop thorugh all azimuthal regions
  //
  for(int f=m_fNmin; f<=m_fNmax; ++f) {
    
    // For each azimuthal region loop through all Z regions
    //
    int z = 0; if(!m_endlist) z = m_zMin;

    for(; z!=11; ++z) {


      int a  = f *11+ZI[z];  if(!rfz_map[a]) continue;
      int NB = 0, NT = 0;
      for(int i=0; i!=rfz_b[a]; ++i) {
	
	int an =  rfz_ib[a][i];
	if(!rfz_map[an]) continue;
	rb [NB] = rfz_Sorted[an].begin(); rbe[NB++] = rfz_Sorted[an].end();
      } 
      for(int i=0; i!=rfz_t[a]; ++i) {
	
	int an =  rfz_it[a][i];
	if(!rfz_map[an]) continue; 
	rt [NT] = rfz_Sorted[an].begin(); rte[NT++] = rfz_Sorted[an].end();
      } 
      production3Sp(rb,rbe,rt,rte,NB,NT,nseed,K);
      if(!m_endlist) {m_fNmin=f; m_zMin = z; return;} 
    }
  }
  m_endlist = true;
}

///////////////////////////////////////////////////////////////////
// Production 3 space points seeds (new version)
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::production3Sp
( std::list<InDet::SiSpacePointForSeed*>::iterator* rb ,
  std::list<InDet::SiSpacePointForSeed*>::iterator* rbe,
  std::list<InDet::SiSpacePointForSeed*>::iterator* rt ,
  std::list<InDet::SiSpacePointForSeed*>::iterator* rte,
  int NB, int NT, int& nseed, float K) 
{

  const float COF  = 134*.07*9.;
  const float COFP = 134*.2*9. ;

  std::list<InDet::SiSpacePointForSeed*>::iterator r0=rb[0],r;
  if(!m_endlist) {r0 = m_rMin; m_endlist = true;}

  // Loop through all trigger space points
  //
  for(; r0!=rbe[0]; ++r0) {

    m_nOneSeeds = 0;
    m_mapOneSeeds.erase(m_mapOneSeeds.begin(), m_mapOneSeeds.end());
	
    float              R    = (*r0)->radius(); if(R<m_r2min) continue; if(R>m_r2max) break;

    bool pix = true; if((*r0)->spacepoint->clusterList().second) pix = false;
    const Trk::Surface* sur0 = (*r0)->sur();
    float              X    = (*r0)->x();
    float              Y    = (*r0)->y();
    float              Z    = (*r0)->z();
    int                 Nb   = 0         ;
    
    
    // Bottom links production
    //
    for(int i=0; i!=NB; ++i) {
      
      for(r=rb[i]; r!=rbe[i]; ++r) {
	
	float Rb =(*r)->radius();
	if(Rb<m_r1min) {rb[i]=r; continue;}  if(Rb>m_r1max) break;
	
	float dR = R-Rb; if(dR<m_drmin) break; 

	if(dR>m_drmax || (*r)->sur()==sur0) continue;

	if( !pix && !(*r)->spacepoint->clusterList().second) continue;

	float dx = X-(*r)->x();
	float dy = Y-(*r)->y();
	float dZ = Z-(*r)->z();
	m_Tz[Nb] = dZ/sqrt(dx*dx+dy*dy); if(m_Tz[Nb]<m_dzdrmin || m_Tz[Nb]>m_dzdrmax) continue;
	m_Zo[Nb] = Z-R*m_Tz[Nb];	          

	// Comparison with vertices Z coordinates
	//
	if(!isZCompatible(m_Zo[Nb],Rb,m_Tz[Nb])) continue;
	m_SP[Nb] = (*r); if(++Nb==m_maxsizeSP) goto breakb;
      }
    }
  breakb:
    if(!Nb || Nb==m_maxsizeSP) continue;  int Nt = Nb;
    
    // Top   links production
    //
    for(int i=0; i!=NT; ++i) {
      
      for(r=rt[i]; r!=rte[i]; ++r) {
	
	float Rt =(*r)->radius();
	float dR = Rt-R; if(dR<m_drmin || Rt<m_r3min) {rt[i]=r; continue;}
	if(Rt>m_r3max || dR>m_drmax) break;

	if((*r)->sur()==sur0) continue;

	float dx = X-(*r)->x();
	float dy = Y-(*r)->y();
	float dZ = (*r)->z()-Z; m_Tz[Nt]   = dZ/sqrt(dx*dx+dy*dy);
	if(m_Tz[Nt]<m_dzdrmin || m_Tz[Nt]>m_dzdrmax) continue;
	m_Zo[Nt]   = Z-R*m_Tz[Nt];	  

	// Comparison with vertices Z coordinates
	//
	if(!isZCompatible(m_Zo[Nt],Rt,m_Tz[Nt])) continue;
	m_SP[Nt] = (*r); if(++Nt==m_maxsizeSP) goto breakt;
      }
    }
    
  breakt:
    if(!(Nt-Nb)) continue;

    float covr0 = (*r0)->covr ();
    float covz0 = (*r0)->covz ();
    float ax   = X/R;
    float ay   = Y/R;
    
    for(int i=0; i!=Nt; ++i) {

      float dx = m_SP[i]->x()-X;
      float dy = m_SP[i]->y()-Y;
      float dz = m_SP[i]->z()-Z;
      float x  = dx*ax+dy*ay   ;
      float y  =-dx*ay+dy*ax   ;
      float r2 = 1./(x*x+y*y)  ;
      float dr  = sqrt(r2)     ;

      i < Nb ?  m_Tz[i] = -dz*dr :  m_Tz[i] = dz*dr;

      m_U [i]   = x*r2;
      m_V [i]   = y*r2;
      m_Er[i]   = (covz0+m_SP[i]->covz()+m_Tz[i]*m_Tz[i]*(covr0+m_SP[i]->covr()))*r2;
      m_R [i]   = dr                                                                ;
    }

    // Three space points comparison
    //
    for(int b=Nb-1; b>=0; --b) {

      float SA  = 1.+m_Tz[b]*m_Tz[b];

      for(int t=Nb;  t!=Nt; ++t) {

	float cof = COF;
	if(!m_SP[t]->spacepoint->clusterList().second) cof = COFP;

	float Ts = .5*(m_Tz[b]+m_Tz[t]);
	float dT =  m_Tz[b]-m_Tz[t]    ;
	dT        = dT*dT-m_Er[b]-m_Er[t]-2.*m_R[b]*m_R[t]*(Ts*Ts*covr0+covz0);

	if( dT > 0. && dT > (m_iptmin*m_iptmin)*cof*SA) continue;
	float dU = m_U[t]-m_U[b]; if(dU == 0.) continue;
	float A  = (m_V[t]-m_V[b])/dU                  ;
	float B  =  m_V[t]-A*m_U[t]                    ;
	float S2 = 1.+A*A                              ;
	float S  = sqrt(S2)                            ;
	float BK = fabs(B*K)                           ;
	if(BK > m_iptmin*S || BK < m_iptmax*S) continue ; // Momentum    cut
	if(dT > 0. && dT  > (BK*BK/S2)*cof*SA) continue ; // Polar angle cut

	float Im = fabs((A-B*R)*R)                     ;
	if(Im > m_diver) continue;

	newOneSeed(m_SP[b]->spacepoint,(*r0)->spacepoint,m_SP[t]->spacepoint,m_Zo[b],Im);
      }
    }
    nseed += m_mapOneSeeds.size(); fillSeeds();
    if(nseed>=m_maxsize) {m_endlist=false; ++r0; m_rMin = r0;  return;} 
  }
}

///////////////////////////////////////////////////////////////////
// Test is space point used
///////////////////////////////////////////////////////////////////

bool InDet::SiSpacePointsSeedMaker_LowMomentum::isUsed(const Trk::SpacePoint* sp)
{
  const Trk::PrepRawData* d = sp->clusterList().first ; 
  if(!d || !m_assoTool->isUsed(*d)) return false;

  d = sp->clusterList().second;
  if(!d || m_assoTool->isUsed(*d)) return true;

  return false;
}

///////////////////////////////////////////////////////////////////
// New 3 space points seeds from one space points
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::newOneSeed
(const Trk::SpacePoint*& p1,const Trk::SpacePoint*& p2, 
 const Trk::SpacePoint*& p3,const float& z,const float& q) 
{
  if(m_nOneSeeds < m_maxOneSize) {

    m_OneSeeds [m_nOneSeeds].erase     (  ); 
    m_OneSeeds [m_nOneSeeds].add       (p1); 
    m_OneSeeds [m_nOneSeeds].add       (p2); 
    m_OneSeeds [m_nOneSeeds].add       (p3); 
    m_OneSeeds [m_nOneSeeds].setZVertex(double(z));
    m_mapOneSeeds.insert(std::make_pair(q,m_OneSeeds+m_nOneSeeds));
    ++m_nOneSeeds;
  }
  else                     {
    std::multimap<float,InDet::SiSpacePointsSeed*>::reverse_iterator 
      l = m_mapOneSeeds.rbegin();
    if((*l).first <= q) return;
    
    InDet::SiSpacePointsSeed* s = (*l).second;
    s->erase     (  ); 
    s->add       (p1); 
    s->add       (p2); 
    s->add       (p3); 
    s->setZVertex(double(z));
    std::multimap<float,InDet::SiSpacePointsSeed*>::iterator 
      i = m_mapOneSeeds.insert(std::make_pair(q,s));
	
    for(++i; i!=m_mapOneSeeds.end(); ++i) {
      if((*i).second==s) {m_mapOneSeeds.erase(i); return;}
    }
  }
}
