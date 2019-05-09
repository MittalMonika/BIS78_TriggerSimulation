/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   Implementation file for class SiSpacePointsSeedMaker_ITK
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// AlgTool used for TRT_DriftCircleOnTrack object production
///////////////////////////////////////////////////////////////////
// Version 1.0 21/04/2004 I.Gavrilenko
///////////////////////////////////////////////////////////////////

#include "SiSpacePointsSeedTool_xk/SiSpacePointsSeedMaker_ITK.h"

#include "InDetPrepRawData/SiCluster.h"
#include "TrkToolInterfaces/IPRD_AssociationTool.h"

#include <iomanip>
#include <ostream>

///////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////

InDet::SiSpacePointsSeedMaker_ITK::SiSpacePointsSeedMaker_ITK
(const std::string& t,const std::string& n,const IInterface* p)
  : AthAlgTool(t,n,p)
{
  declareInterface<ISiSpacePointsSeedMaker>(this);
}

///////////////////////////////////////////////////////////////////
// Initialisation
///////////////////////////////////////////////////////////////////

StatusCode InDet::SiSpacePointsSeedMaker_ITK::initialize()
{
  StatusCode sc = AlgTool::initialize();

  ATH_CHECK(m_spacepointsPixel.initialize(m_pixel));
  ATH_CHECK(m_spacepointsSCT.initialize(m_sct));
  ATH_CHECK(m_spacepointsOverlap.initialize(m_useOverlap));

  // Get beam geometry
  //
  ATH_CHECK(m_beamSpotKey.initialize());

  // Get magnetic field service
  //
  if ( !m_fieldServiceHandle.retrieve() ){
    ATH_MSG_FATAL("Failed to retrieve " << m_fieldServiceHandle );
    return StatusCode::FAILURE;
  }    
  ATH_MSG_DEBUG("Retrieved " << m_fieldServiceHandle );

  // Get tool for track-prd association
  //
  if ( m_useassoTool ) {
    if ( m_assoTool.retrieve().isFailure()) {
      ATH_MSG_FATAL("Failed to retrieve tool "<< m_assoTool);
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_INFO("Retrieved tool " << m_assoTool);
    }
  } else {
    m_assoTool.disable();
  }

  if (m_r_rmax < 1100.) m_r_rmax = 1100.;
  
  // Build framework
  //
  buildFrameWork();
  m_CmSp.reserve(500);

  // Get output print level
  //
  m_outputlevel = msg().level()-MSG::DEBUG;
  if (m_outputlevel<=0) {
    m_nprint=0;
    ATH_MSG_DEBUG(*this);
  }
  m_umax = 100.-fabs(m_umax)*300.;
  return sc;
}

///////////////////////////////////////////////////////////////////
// Finalize
///////////////////////////////////////////////////////////////////

StatusCode InDet::SiSpacePointsSeedMaker_ITK::finalize()
{
  return AlgTool::finalize();
}

///////////////////////////////////////////////////////////////////
// Initialize tool for new event 
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::newEvent(int iteration) 
{
  m_iteration0 = iteration;
  m_trigger = false;
  if (!m_pixel && !m_sct) return;

  iteration <=0 ? m_iteration = 0 : m_iteration = iteration;
  erase();
  m_dzdrmin =  m_dzdrmin0;
  m_dzdrmax =  m_dzdrmax0;

  if (!m_iteration) {
    buildBeamFrameWork();

    double f[3], gP[3] ={10.,10.,0.};
    if (m_fieldServiceHandle->solenoidOn()) {
      m_fieldServiceHandle->getFieldZR(gP,f);
      m_K = 2./(300.*f[2]);
    } else {
      m_K = 2./(300.* 5. );
    }

    m_ipt2K     = m_ipt2/(m_K*m_K);
    m_ipt2C     = m_ipt2*m_COF    ;
    m_COFK      = m_COF*(m_K*m_K) ;
    m_i_spforseed = m_l_spforseed.begin();
  } else {
    m_r_first = 0;
    fillLists();
    return;
  }

  m_checketa = m_dzdrmin > 1.;

  float irstep = 1./m_r_rstep;
  int   irmax  = m_r_size-1  ;
  for (int i=0; i!=m_nr; ++i) {
    int n = m_r_index[i];
    m_r_map[n] = 0;
    m_r_Sorted[n].clear();
  }
  m_ns = m_nr = 0;

  // Get pixels space points containers from store gate 
  //
  m_r_first = 0;
  if (m_pixel) {

    SG::ReadHandle<SpacePointContainer> spacepointsPixel{m_spacepointsPixel};
    if (spacepointsPixel.isValid()) {

      for (const SpacePointCollection* spc: *spacepointsPixel) {
        for (const Trk::SpacePoint* sp: *spc) {

	  if ((m_useassoTool && isUsed(sp)) || sp->r() > m_r_rmax || sp->r() < m_r_rmin ) continue;

	  InDet::SiSpacePointForSeedITK* sps = newSpacePoint(sp);
          if (!sps) continue;

	  int ir = static_cast<int>(sps->radius()*irstep);
          if (ir>irmax) ir = irmax;
	  m_r_Sorted[ir].push_back(sps);
          ++m_r_map[ir];
	  if (m_r_map[ir]==1) m_r_index[m_nr++] = ir;
	  if (ir > m_r_first) m_r_first = ir;
	  ++m_ns;
	}
      }
    }
    ++m_r_first;
  }

  // Get sct space points containers from store gate 
  //
  if (m_sct) {

    SG::ReadHandle<SpacePointContainer> spacepointsSCT{m_spacepointsSCT};
    if (spacepointsSCT.isValid()) {

      for (const SpacePointCollection* spc: *spacepointsSCT) {
        for (const Trk::SpacePoint* sp: *spc) {

	  if ((m_useassoTool && isUsed(sp)) || sp->r() > m_r_rmax || sp->r() < m_r_rmin ) continue;

	  InDet::SiSpacePointForSeedITK* sps = newSpacePoint(sp);
          if (!sps) continue;

	  int ir = static_cast<int>(sps->radius()*irstep);
          if (ir>irmax) ir = irmax;
	  m_r_Sorted[ir].push_back(sps);
          ++m_r_map[ir];
	  if (m_r_map[ir]==1) m_r_index[m_nr++] = ir;
	  ++m_ns;
	}
      }
    }

    // Get sct overlap space points containers from store gate 
    //
    if (m_useOverlap) {

      SG::ReadHandle<SpacePointOverlapCollection> spacepointsOverlap{m_spacepointsOverlap};
      if (spacepointsOverlap.isValid()) {
	
        for (const Trk::SpacePoint* sp: *spacepointsOverlap) {

	  if ((m_useassoTool && isUsed(sp)) || sp->r() > m_r_rmax || sp->r() < m_r_rmin) continue;

	  InDet::SiSpacePointForSeedITK* sps = newSpacePoint(sp);
          if (!sps) continue;

	  int ir = static_cast<int>(sps->radius()*irstep);
          if (ir>irmax) ir = irmax;
	  m_r_Sorted[ir].push_back(sps);
          ++m_r_map[ir];
	  if (m_r_map[ir]==1) m_r_index[m_nr++] = ir;
	  ++m_ns;
	}
      }
    }
  }

  if (iteration < 0) m_r_first = 0;
  fillLists();
}

///////////////////////////////////////////////////////////////////
// Initialize tool for new region
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::newRegion
(const std::vector<IdentifierHash>& vPixel, const std::vector<IdentifierHash>& vSCT)
{
  m_iteration  = 0    ;
  m_trigger    = false;
  erase();
  if (!m_pixel && !m_sct) return;

  m_dzdrmin =  m_dzdrmin0;
  m_dzdrmax =  m_dzdrmax0;

  buildBeamFrameWork();

  double f[3], gP[3] ={10.,10.,0.};

  if (m_fieldServiceHandle->solenoidOn()) {
    m_fieldServiceHandle->getFieldZR(gP,f);
    m_K = 2./(300.*f[2]);
  } else {
    m_K = 2./(300.* 5. );
  }

  m_ipt2K     = m_ipt2/(m_K*m_K);
  m_ipt2C     = m_ipt2*m_COF    ;
  m_COFK      = m_COF*(m_K*m_K) ;

  m_i_spforseed = m_l_spforseed.begin();

  float irstep = 1./m_r_rstep;
  int   irmax  = m_r_size-1  ;

  m_r_first      = 0         ;
  m_checketa   = false     ;

  for (int i=0; i!=m_nr; ++i) {
    int n = m_r_index[i];
    m_r_map[n] = 0;
    m_r_Sorted[n].clear();
  }
  m_ns = m_nr = 0;

  // Get pixels space points containers from store gate 
  //
  if (m_pixel && vPixel.size()) {

    SG::ReadHandle<SpacePointContainer> spacepointsPixel{m_spacepointsPixel};
    if (spacepointsPixel.isValid()) {
      SpacePointContainer::const_iterator spce = spacepointsPixel->end();

      // Loop through all trigger collections
      //
      for (const IdentifierHash& l: vPixel) {
	SpacePointContainer::const_iterator w = spacepointsPixel->indexFind(l);
	if (w==spce) continue;
        for (const Trk::SpacePoint* sp: **w) {
	  float r = sp->r();
          if (r > m_r_rmax || r < m_r_rmin) continue;
	  InDet::SiSpacePointForSeedITK* sps = newSpacePoint(sp);
	  int ir = static_cast<int>(sps->radius()*irstep);
          if (ir>irmax) ir = irmax;
	  m_r_Sorted[ir].push_back(sps);
          ++m_r_map[ir];
	  if (m_r_map[ir]==1) m_r_index[m_nr++] = ir;
	  ++m_ns;
	}
      }
    }
  }

  // Get sct space points containers from store gate 
  //
  if (m_sct && vSCT.size()) {

    SG::ReadHandle<SpacePointContainer> spacepointsSCT{m_spacepointsSCT};
    if (spacepointsSCT.isValid()) {
      SpacePointContainer::const_iterator spce = spacepointsSCT->end();

      // Loop through all trigger collections
      //
      for (const IdentifierHash& l: vSCT) {
	SpacePointContainer::const_iterator w = spacepointsSCT->indexFind(l);
	if (w==spce) continue;
        for (const Trk::SpacePoint* sp: **w) {
	  float r = sp->r();
          if (r > m_r_rmax || r < m_r_rmin) continue;
	  InDet::SiSpacePointForSeedITK* sps = newSpacePoint(sp);
	  int ir = static_cast<int>(sps->radius()*irstep);
          if (ir>irmax) ir = irmax;
	  m_r_Sorted[ir].push_back(sps);
          ++m_r_map[ir];
	  if (m_r_map[ir]==1) m_r_index[m_nr++] = ir;
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

void InDet::SiSpacePointsSeedMaker_ITK::newRegion
(const std::vector<IdentifierHash>& vPixel, const std::vector<IdentifierHash>& vSCT,const IRoiDescriptor& IRD)
{
  newRegion(vPixel,vSCT);
  m_trigger = true;

  double dzdrmin = 1./tan(2.*atan(exp(-IRD.etaMinus())));
  double dzdrmax = 1./tan(2.*atan(exp(-IRD.etaPlus ())));
 
  m_zminB        = IRD.zedMinus()-m_zbeam[0]; // min bottom Z
  m_zmaxB        = IRD.zedPlus ()-m_zbeam[0]; // max bottom Z
  m_zminU        = m_zminB+550.*dzdrmin;
  m_zmaxU        = m_zmaxB+550.*dzdrmax;
  double fmax    = IRD.phiPlus ();
  double fmin    = IRD.phiMinus();
  if (fmin > fmax) fmin-=(2.*M_PI);
  m_ftrig        = (fmin+fmax)*.5;
  m_ftrigW       = (fmax-fmin)*.5;
}

///////////////////////////////////////////////////////////////////
// Methods to initilize different strategies of seeds production
// with two space points with or without vertex constraint
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::find2Sp(const std::list<Trk::Vertex>& lv) 
{
  m_zminU     = m_zmin;
  m_zmaxU     = m_zmax;

  int mode; lv.begin()!=lv.end() ?  mode = 1 : mode = 0;
  bool newv = newVertices(lv);
  
  if (newv || !m_state || m_nspoint!=2 || m_mode!=mode || m_nlist) {

    m_i_seede = m_l_seeds.begin();
    m_state   = 1   ;
    m_nspoint = 2   ;
    m_nlist   = 0   ;
    m_mode    = mode;
    m_endlist = true;
    m_fvNmin  = 0   ;
    m_fNmin   = 0   ;
    m_zMin    = 0   ;
    production2Sp ();
  }
  m_i_seed = m_l_seeds.begin();
  
  if (m_outputlevel<=0) {
    m_nprint=1;
    ATH_MSG_DEBUG(*this);
  }
}

///////////////////////////////////////////////////////////////////
// Methods to initilize different strategies of seeds production
// with three space points with or without vertex constraint
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::find3Sp(const std::list<Trk::Vertex>& lv) 
{
  m_zminU     = m_zmin;
  m_zmaxU     = m_zmax;

  int mode;
  lv.begin()!=lv.end() ? mode = 3 : mode = 2;
  bool newv = newVertices(lv);

  if (newv || !m_state || m_nspoint!=3 || m_mode!=mode || m_nlist) {
    m_i_seede = m_l_seeds.begin();
    m_state   = 1               ;
    m_nspoint = 3               ;
    m_nlist   = 0               ;
    m_mode    = mode            ;
    m_endlist = true            ;
    m_fvNmin  = 0               ;
    m_fNmin   = 0               ;
    m_zMin    = 0               ;
    production3Sp();
  }
  m_i_seed = m_l_seeds.begin();
  m_seed = m_seeds.begin();

  if (m_outputlevel<=0) {
    m_nprint=1;
    ATH_MSG_DEBUG(*this);
  }
}

///////////////////////////////////////////////////////////////////
// Methods to initilize different strategies of seeds production
// with three space points with or without vertex constraint
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::find3Sp(const std::list<Trk::Vertex>& lv,const double* ZVertex) 
{
  m_zminU = ZVertex[0];
  if (m_zminU < m_zmin) m_zminU = m_zmin;
  m_zmaxU = ZVertex[1];
  if (m_zmaxU > m_zmax) m_zmaxU = m_zmax;

  int mode; lv.begin()!=lv.end() ? mode = 3 : mode = 2;
  bool newv = newVertices(lv);

  if (newv || !m_state || m_nspoint!=3 || m_mode!=mode || m_nlist) {
    m_i_seede = m_l_seeds.begin();
    m_state   = 1               ;
    m_nspoint = 3               ;
    m_nlist   = 0               ;
    m_mode    = mode            ;
    m_endlist = true            ;
    m_fvNmin  = 0               ;
    m_fNmin   = 0               ;
    m_zMin    = 0               ;
    production3Sp();
  }
  m_i_seed = m_l_seeds.begin();
  m_seed = m_seeds.begin();

  if (m_outputlevel<=0) {
    m_nprint=1;
    ATH_MSG_DEBUG(*this);
  }
}

///////////////////////////////////////////////////////////////////
// Methods to initilize different strategies of seeds production
// with variable number space points with or without vertex constraint
// Variable means (2,3,4,....) any number space points
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::findVSp (const std::list<Trk::Vertex>& lv)
{
  m_zminU     = m_zmin;
  m_zmaxU     = m_zmax;

  int mode;
  lv.begin()!=lv.end() ? mode = 6 : mode = 5;
  bool newv = newVertices(lv);
  
  if (newv || !m_state || m_nspoint!=4 || m_mode!=mode || m_nlist) {

    m_i_seede = m_l_seeds.begin();
    m_state   = 1               ;
    m_nspoint = 4               ;
    m_nlist   = 0               ;
    m_mode    = mode            ;
    m_endlist = true            ;
    m_fvNmin  = 0               ;
    m_fNmin   = 0               ;
    m_zMin    = 0               ;
    production3Sp();
  }
  m_i_seed = m_l_seeds.begin();
  m_seed = m_seeds.begin();

  if (m_outputlevel<=0) {
    m_nprint=1;
    ATH_MSG_DEBUG(*this);
  }
}

///////////////////////////////////////////////////////////////////
// Dumps relevant information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::SiSpacePointsSeedMaker_ITK::dump( MsgStream& out ) const
{
  if (m_nprint)  return dumpEvent(out);
  return dumpConditions(out);
}

///////////////////////////////////////////////////////////////////
// Dumps conditions information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::SiSpacePointsSeedMaker_ITK::dumpConditions( MsgStream& out ) const
{
  int n = 42-m_spacepointsPixel.key().size();
  std::string s2; for (int i=0; i<n; ++i) s2.append(" "); s2.append("|");
  n     = 42-m_spacepointsSCT.key().size();
  std::string s3; for (int i=0; i<n; ++i) s3.append(" "); s3.append("|");
  n     = 42-m_spacepointsOverlap.key().size();
  std::string s4; for (int i=0; i<n; ++i) s4.append(" "); s4.append("|");
  n     = 42-m_beamSpotKey.key().size();
  std::string s5; for (int i=0; i<n; ++i) s5.append(" "); s5.append("|");


  out<<"|---------------------------------------------------------------------|"
     <<endmsg;
  out<<"| Pixel    space points   | "<<m_spacepointsPixel.key() <<s2
     <<endmsg;
  out<<"| SCT      space points   | "<<m_spacepointsSCT.key()<<s3
     <<endmsg;
  out<<"| Overlap  space points   | "<<m_spacepointsOverlap.key()<<s4
     <<endmsg;
  out<<"| BeamConditionsService   | "<<m_beamSpotKey.key()<<s5
     <<endmsg;
  out<<"| usePixel                | "
     <<std::setw(12)<<m_pixel 
     <<"                              |"<<endmsg;
  out<<"| useSCT                  | "
     <<std::setw(12)<<m_sct 
     <<"                              |"<<endmsg;
  out<<"| maxSize                 | "
     <<std::setw(12)<<m_maxsize 
     <<"                              |"<<endmsg;
  out<<"| maxSizeSP               | "
     <<std::setw(12)<<m_maxsizeSP
     <<"                              |"<<endmsg;
  out<<"| pTmin  (mev)            | "
     <<std::setw(12)<<std::setprecision(5)<<m_ptmin
     <<"                              |"<<endmsg;
  out<<"| |rapidity|          <=  | " 
     <<std::setw(12)<<std::setprecision(5)<<m_rapcut
     <<"                              |"<<endmsg;
  out<<"| max radius SP           | "
     <<std::setw(12)<<std::setprecision(5)<<m_r_rmax 
     <<"                              |"<<endmsg;
  out<<"| min radius SP           | "
     <<std::setw(12)<<std::setprecision(5)<<m_r_rmin 
     <<"                              |"<<endmsg;
  out<<"| radius step             | "
     <<std::setw(12)<<std::setprecision(5)<<m_r_rstep
     <<"                              |"<<endmsg;
  out<<"| min Z-vertex position   | "
     <<std::setw(12)<<std::setprecision(5)<<m_zmin
     <<"                              |"<<endmsg;
  out<<"| max Z-vertex position   | "
     <<std::setw(12)<<std::setprecision(5)<<m_zmax
     <<"                              |"<<endmsg;
  out<<"| min radius first  SP(3) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r1min
     <<"                              |"<<endmsg;
  out<<"| min radius second SP(3) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r2min
     <<"                              |"<<endmsg;
  out<<"| min radius last   SP(3) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r3min
     <<"                              |"<<endmsg;
  out<<"| max radius first  SP(3) | "
     <<std::setw(12)<<std::setprecision(4)<<m_r1max
     <<"                              |"<<endmsg;
  out<<"| max radius second SP(3) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r2max
     <<"                              |"<<endmsg;
  out<<"| max radius last   SP(3) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r3max
     <<"                              |"<<endmsg;
  out<<"| min radius first  SP(2) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r1minv
     <<"                              |"<<endmsg;
  out<<"| min radius second SP(2) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r2minv
     <<"                              |"<<endmsg;
  out<<"| max radius first  SP(2) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r1maxv
     <<"                              |"<<endmsg;
  out<<"| max radius second SP(2) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r2maxv
     <<"                              |"<<endmsg;
  out<<"| min space points dR     | "
     <<std::setw(12)<<std::setprecision(5)<<m_drmin
     <<"                              |"<<endmsg;
  out<<"| max space points dR     | "
     <<std::setw(12)<<std::setprecision(5)<<m_drmax
     <<"                              |"<<endmsg;
  out<<"| max dZ    impact        | "
     <<std::setw(12)<<std::setprecision(5)<<m_dzver 
     <<"                              |"<<endmsg;
  out<<"| max dZ/dR impact        | "
     <<std::setw(12)<<std::setprecision(5)<<m_dzdrver 
     <<"                              |"<<endmsg;
  out<<"| max       impact        | "
     <<std::setw(12)<<std::setprecision(5)<<m_diver
     <<"                              |"<<endmsg;
  out<<"| max       impact pps    | "
     <<std::setw(12)<<std::setprecision(5)<<m_diverpps
     <<"                              |"<<endmsg;
  out<<"| max       impact sss    | "
     <<std::setw(12)<<std::setprecision(5)<<m_diversss
     <<"                              |"<<endmsg;
  out<<"|---------------------------------------------------------------------|"
     <<endmsg;
  out<<"| Beam X center           | "
     <<std::setw(12)<<std::setprecision(5)<<m_xbeam[0]
     <<"                              |"<<endmsg;
  out<<"| Beam Y center           | "
     <<std::setw(12)<<std::setprecision(5)<<m_ybeam[0]
     <<"                              |"<<endmsg;
  out<<"| Beam Z center           | "
     <<std::setw(12)<<std::setprecision(5)<<m_zbeam[0]
     <<"                              |"<<endmsg;
  out<<"| Beam X-axis direction   | "
     <<std::setw(12)<<std::setprecision(5)<<m_xbeam[1]
     <<std::setw(12)<<std::setprecision(5)<<m_xbeam[2]
     <<std::setw(12)<<std::setprecision(5)<<m_xbeam[3]
     <<"      |"<<endmsg;
  out<<"| Beam Y-axis direction   | "
     <<std::setw(12)<<std::setprecision(5)<<m_ybeam[1]
     <<std::setw(12)<<std::setprecision(5)<<m_ybeam[2]
     <<std::setw(12)<<std::setprecision(5)<<m_ybeam[3]
     <<"      |"<<endmsg;
  out<<"| Beam Z-axis direction   | "
     <<std::setw(12)<<std::setprecision(5)<<m_zbeam[1]
     <<std::setw(12)<<std::setprecision(5)<<m_zbeam[2]
     <<std::setw(12)<<std::setprecision(5)<<m_zbeam[3]
     <<"      |"<<endmsg;
  out<<"|---------------------------------------------------------------------|"
     <<endmsg;
  return out;
}

///////////////////////////////////////////////////////////////////
// Dumps event information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::SiSpacePointsSeedMaker_ITK::dumpEvent( MsgStream& out ) const
{
  out<<"|---------------------------------------------------------------------|"
     <<endmsg;
  out<<"| m_ns                    | "
     <<std::setw(12)<<m_ns
     <<"                              |"<<endmsg;
  out<<"| m_nsaz                  | "
     <<std::setw(12)<<m_nsaz
     <<"                              |"<<endmsg;
  out<<"| m_nsazv                 | "
     <<std::setw(12)<<m_nsazv
     <<"                              |"<<endmsg;
  out<<"| seeds                   | "
     <<std::setw(12)<<m_l_seeds.size()
     <<"                              |"<<endmsg;
  out<<"|---------------------------------------------------------------------|"
     <<endmsg;
  return out;
}

///////////////////////////////////////////////////////////////////
// Dumps relevant information into the ostream
///////////////////////////////////////////////////////////////////

std::ostream& InDet::SiSpacePointsSeedMaker_ITK::dump( std::ostream& out ) const
{
  return out;
}

///////////////////////////////////////////////////////////////////
// Overload of << operator MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::operator    << 
(MsgStream& sl,const InDet::SiSpacePointsSeedMaker_ITK& se)
{ 
  return se.dump(sl);
}

///////////////////////////////////////////////////////////////////
// Overload of << operator std::ostream
///////////////////////////////////////////////////////////////////

std::ostream& InDet::operator << 
(std::ostream& sl,const InDet::SiSpacePointsSeedMaker_ITK& se)
{ 
  return se.dump(sl);
}

///////////////////////////////////////////////////////////////////
// Find next set space points
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::findNext () 
{
  if (m_endlist) return;

  m_i_seede = m_l_seeds.begin();

  if      (m_mode==0 || m_mode==1) production2Sp ();
  else if (m_mode==2 || m_mode==3) production3Sp ();
  else if (m_mode==5 || m_mode==6) production3Sp ();

  m_i_seed = m_l_seeds.begin();
  m_seed = m_seeds.begin();
  ++m_nlist;
}                       

///////////////////////////////////////////////////////////////////
// New and old list vertices comparison
///////////////////////////////////////////////////////////////////

bool InDet::SiSpacePointsSeedMaker_ITK::newVertices(const std::list<Trk::Vertex>& lV)
{
  unsigned int s1 = m_l_vertex.size();
  unsigned int s2 = lV.size();

  m_isvertex = false;
  if (s1==0 && s2==0) return false;

  std::list<Trk::Vertex>::const_iterator v;
  m_l_vertex.clear();
  if (s2 == 0) return false;

  m_isvertex = true;
  for (v=lV.begin(); v!=lV.end(); ++v) {
    m_l_vertex.insert(static_cast<float>((*v).position().z()));
  }

  m_zminU = (*m_l_vertex. begin())-20.;
  if ( m_zminU < m_zmin) m_zminU = m_zmin;
  m_zmaxU = (*m_l_vertex.rbegin())+20.;
  if ( m_zmaxU > m_zmax) m_zmaxU = m_zmax;

  return false;
}

///////////////////////////////////////////////////////////////////
// Initiate frame work for seed generator
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::buildFrameWork() 
{
  m_ptmin     = fabs(m_ptmin);
  
  if (m_ptmin < 100.) m_ptmin = 100.;

  if (m_diversss < m_diver   ) m_diversss = m_diver   ;
  if (m_divermax < m_diversss) m_divermax = m_diversss;

  if (fabs(m_etamin) < .1) m_etamin = -m_etamax ;
  m_dzdrmax0  = 1./tan(2.*atan(exp(-m_etamax)));
  m_dzdrmin0  = 1./tan(2.*atan(exp(-m_etamin)));
  
  m_COF       =  134*.05*9.                    ;
  m_ipt       = 1./fabs(.9*m_ptmin)            ;
  m_ipt2      = m_ipt*m_ipt                    ;
  m_K         = 0.                             ;

  m_ns = m_nsaz = m_nsazv = m_nr = m_nrfz = m_nrfzv = 0;

  // Build radius sorted containers
  //
  m_r_size = static_cast<int>((m_r_rmax+.1)/m_r_rstep);
  m_r_Sorted.resize(m_r_size);
  m_r_index.resize(m_r_size, 0);
  m_r_map.resize(m_r_size, 0);
  m_nr = 0;

  // Build radius-azimuthal sorted containers
  //
  constexpr float pi2 = 2.*M_PI;
  const int   NFmax = SizeRF;
  const float sFmax = static_cast<float>(NFmax)/pi2;
  const float sFmin = 100./60.          ;

  float ptm = 400.;
  if (m_ptmin < ptm) ptm = m_ptmin;

  m_sF        = ptm /60. ;
  if (m_sF    >sFmax ) m_sF    = sFmax  ;
  else if (m_sF < sFmin) m_sF = sFmin;
  m_fNmax     = static_cast<int>(pi2*m_sF);
  if (m_fNmax >=NFmax) m_fNmax = NFmax-1;


  // Build radius-azimuthal-Z sorted containers
  //
  m_nrfz  = 0;
  for (int i=0; i<SizeRFZ; ++i) {
    m_rfz_index [i]=0;
    m_rfz_map [i]=0;
  }

  // Build radius-azimuthal-Z sorted containers for Z-vertices
  //
  const int   NFtmax = SizeRFV;
  const float sFvmax = static_cast<float>(NFtmax)/pi2;
  m_sFv       = m_ptmin/120.;
  if (m_sFv>sFvmax)  m_sFv = sFvmax; 
  m_fvNmax    = static_cast<int>(pi2*m_sFv);
  if (m_fvNmax>=NFtmax) m_fvNmax = NFtmax-1;
  m_nrfzv = 0;
  for (int i=0; i<SizeRFZV; ++i) {
    m_rfzv_index[i]=0;
    m_rfzv_map[i]=0;
  }

  // Build maps for radius-azimuthal-Z sorted collections 
  //
  for (int f=0; f<=m_fNmax; ++f) {

    int fb = f-1; if (fb<0      ) fb=m_fNmax;
    int ft = f+1; if (ft>m_fNmax) ft=0;
    
    // For each azimuthal region loop through all Z regions
    //
    for (int z=0; z<SizeZ; ++z) {
 
      int a        = f *SizeZ+z;
      int b        = fb*SizeZ+z;
      int c        = ft*SizeZ+z;
      m_rfz_b [a]    = 3; m_rfz_t [a]    = 3;
      m_rfz_ib[a][0] = a; m_rfz_it[a][0] = a;
      m_rfz_ib[a][1] = b; m_rfz_it[a][1] = b;
      m_rfz_ib[a][2] = c; m_rfz_it[a][2] = c;
      if (z==5) {

	m_rfz_t [a]    = 9 ;
	m_rfz_it[a][3] = a+1;
	m_rfz_it[a][4] = b+1;
	m_rfz_it[a][5] = c+1;
	m_rfz_it[a][6] = a-1;
	m_rfz_it[a][7] = b-1;
	m_rfz_it[a][8] = c-1;
      } else if (z> 5) {

	m_rfz_b [a]    = 6 ;
	m_rfz_ib[a][3] = a-1;
	m_rfz_ib[a][4] = b-1;
	m_rfz_ib[a][5] = c-1;

	if (z<10) {

	  m_rfz_t [a]    = 6 ;
	  m_rfz_it[a][3] = a+1;
	  m_rfz_it[a][4] = b+1;
	  m_rfz_it[a][5] = c+1;
	}
      } else {

	m_rfz_b [a]    = 6 ;
	m_rfz_ib[a][3] = a+1;
	m_rfz_ib[a][4] = b+1;
	m_rfz_ib[a][5] = c+1;

	if (z>0) {

	  m_rfz_t [a]    = 6 ;
	  m_rfz_it[a][3] = a-1;
	  m_rfz_it[a][4] = b-1;
	  m_rfz_it[a][5] = c-1;
	}
      }

      if (z==3) {
	m_rfz_b[a]      = 9;
	m_rfz_ib[a][6] = a+2;
	m_rfz_ib[a][7] = b+2;
	m_rfz_ib[a][8] = c+2;
      } else if (z==7) {
	m_rfz_b[a]      = 9;
	m_rfz_ib[a][6] = a-2;
	m_rfz_ib[a][7] = b-2;
	m_rfz_ib[a][8] = c-2;
      }
    }
  }

  // Build maps for radius-azimuthal-Z sorted collections for Z
  //
  for (int f=0; f<=m_fvNmax; ++f) {

    int fb = f-1;
    if (fb<0) fb=m_fvNmax;
    int ft = f+1;
    if (ft>m_fvNmax) ft=0;
    
    // For each azimuthal region loop through central Z regions
    //
    for (int z=0; z<SizeZV; ++z) {
      
      int a  = f *SizeZV+z;
      int b  = fb*SizeZV+z;
      int c  = ft*SizeZV+z;
      m_rfzv_n[a]    = 3;
      m_rfzv_i[a][0] = a;
      m_rfzv_i[a][1] = b;
      m_rfzv_i[a][2] = c;
      if (z>1) {
	m_rfzv_n[a]    = 6;
	m_rfzv_i[a][3] = a-1;
	m_rfzv_i[a][4] = b-1;
	m_rfzv_i[a][5] = c-1;
      } else if (z<1) {
	m_rfzv_n[a]    = 6;
	m_rfzv_i[a][3] = a+1;
	m_rfzv_i[a][4] = b+1;
	m_rfzv_i[a][5] = c+1;
      }
    }
  }

  m_SP.resize(m_maxsizeSP, nullptr);
  m_R.resize(m_maxsizeSP, 0.);
  m_X.resize(m_maxsizeSP, 0.);
  m_Y.resize(m_maxsizeSP, 0.);
  m_Tz.resize(m_maxsizeSP, 0.);
  m_Er.resize(m_maxsizeSP, 0.);
  m_U.resize(m_maxsizeSP, 0.);
  m_V.resize(m_maxsizeSP, 0.);
  m_Zo.resize(m_maxsizeSP, 0.);
  m_OneSeeds.resize(m_maxOneSize);

  m_i_seed  = m_l_seeds.begin();
  m_i_seede = m_l_seeds.end  ();
}

///////////////////////////////////////////////////////////////////
// Initiate beam frame work for seed generator
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::buildBeamFrameWork() 
{ 
  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey };

  const Amg::Vector3D &cb =     beamSpotHandle->beamPos();
  double     tx = tan(beamSpotHandle->beamTilt(0));
  double     ty = tan(beamSpotHandle->beamTilt(1));

  double ph   = atan2(ty,tx);
  double th   = acos(1./sqrt(1.+tx*tx+ty*ty));
  double sint = sin(th);
  double cost = cos(th);
  double sinp = sin(ph);
  double cosp = cos(ph);
  
  m_xbeam[0] = static_cast<float>(cb.x())                  ;
  m_xbeam[1] = static_cast<float>(cost*cosp*cosp+sinp*sinp);
  m_xbeam[2] = static_cast<float>(cost*sinp*cosp-sinp*cosp);
  m_xbeam[3] =-static_cast<float>(sint*cosp               );
  
  m_ybeam[0] = static_cast<float>(cb.y())                  ;
  m_ybeam[1] = static_cast<float>(cost*cosp*sinp-sinp*cosp);
  m_ybeam[2] = static_cast<float>(cost*sinp*sinp+cosp*cosp);
  m_ybeam[3] =-static_cast<float>(sint*sinp               );
  
  m_zbeam[0] = static_cast<float>(cb.z())                  ;
  m_zbeam[1] = static_cast<float>(sint*cosp)               ;
  m_zbeam[2] = static_cast<float>(sint*sinp)               ;
  m_zbeam[3] = static_cast<float>(cost)                    ;
}

///////////////////////////////////////////////////////////////////
// Initiate beam frame work for seed generator
///////////////////////////////////////////////////////////////////
void  InDet::SiSpacePointsSeedMaker_ITK::convertToBeamFrameWork
(const  Trk::SpacePoint*const& sp,float* r)
{
  
  r[0] = static_cast<float>(sp->globalPosition().x())-m_xbeam[0];
  r[1] = static_cast<float>(sp->globalPosition().y())-m_ybeam[0];
  r[2] = static_cast<float>(sp->globalPosition().z())-m_zbeam[0];

  if (!sp->clusterList().second) return;

  // Only for SCT space points
  //
  const InDet::SiCluster* c0 = static_cast<const InDet::SiCluster*>(sp->clusterList().first );
  const InDet::SiCluster* c1 = static_cast<const InDet::SiCluster*>(sp->clusterList().second);
  
  Amg::Vector2D lc0 = c0->localPosition();
  Amg::Vector2D lc1 = c1->localPosition();
  
  std::pair<Amg::Vector3D, Amg::Vector3D > e0 =
    (c0->detectorElement()->endsOfStrip(InDetDD::SiLocalPosition(lc0.y(),lc0.x(),0.)));
  std::pair<Amg::Vector3D, Amg::Vector3D > e1 =
    (c1->detectorElement()->endsOfStrip(InDetDD::SiLocalPosition(lc1.y(),lc1.x(),0.)));

  Amg::Vector3D b0 (e0.second-e0.first);
  Amg::Vector3D b1 (e1.second-e1.first);
  Amg::Vector3D d02(e0.first -e1.first);

  // b0
  r[ 3] = static_cast<float>(b0[0]);
  r[ 4] = static_cast<float>(b0[1]);
  r[ 5] = static_cast<float>(b0[2]);
  
  // b1
  r[ 6] = static_cast<float>(b1[0]);
  r[ 7] = static_cast<float>(b1[1]);
  r[ 8] = static_cast<float>(b1[2]);

  // r0-r2
  r[ 9] = static_cast<float>(d02[0]);
  r[10] = static_cast<float>(d02[1]);
  r[11] = static_cast<float>(d02[2]);

  // r0
  r[12] = static_cast<float>(e0.first[0])-m_xbeam[0];
  r[13] = static_cast<float>(e0.first[1])-m_ybeam[0];
  r[14] = static_cast<float>(e0.first[2])-m_zbeam[0];
}
   
///////////////////////////////////////////////////////////////////
// Initiate space points seed maker
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::fillLists() 
{
  constexpr float pi2 = 2.*M_PI;
  std::list<InDet::SiSpacePointForSeedITK*>::iterator r,re;

  int  ir0 =0;
  
  for (int i=m_r_first; i!=m_r_size;  ++i) {

    if (!m_r_map[i]) continue;
    r = m_r_Sorted[i].begin();
    re = m_r_Sorted[i].end();
    if (!ir0) ir0 = i;

    if (m_iteration && (*r)->spacepoint->clusterList().second) break;

    for (; r!=re; ++r) {
      
      // Azimuthal angle sort
      //
      float F = (*r)->phi();
      if (F<0.) F+=pi2;

      int   f = static_cast<int>(F*m_sF);
      if (f < 0) f = m_fNmax;
      else if (f > m_fNmax) f = 0;

      int z;
      float Z = (*r)->z();

      // Azimuthal angle and Z-coordinate sort
      //
      if (Z>0.) {
	Z< 250.?z=5:Z< 450.?z=6:Z< 925.?z=7:Z< 1400.?z=8:Z< 2500.?z=9:z=10;
      } else {
	Z>-250.?z=5:Z>-450.?z=4:Z>-925.?z=3:Z>-1400.?z=2:Z>-2500.?z=1:z= 0;
      }

      int n = f*SizeZ+z;
      ++m_nsaz;
      m_rfz_Sorted[n].push_back(*r);
      if (!m_rfz_map[n]++) m_rfz_index[m_nrfz++] = n;
      
      if (!m_iteration && (*r)->spacepoint->clusterList().second == 0 && z>=3 && z<=7) { 
	z<=4 ? z=0 : z>=6 ? z=2 : z=1;

	// Azimuthal angle and Z-coordinate sort for fast vertex search
	//
	f = static_cast<int>(F*m_sFv);
        if (f < 0) f += m_fvNmax;
        else if (f> m_fvNmax) f -= m_fvNmax;

        n = f*3+z;
        ++m_nsazv;
	m_rfzv_Sorted[n].push_back(*r);
        if (!m_rfzv_map[n]++) m_rfzv_index[m_nrfzv++] = n;
      }
    }
  }
  m_state = 0;
}
   
///////////////////////////////////////////////////////////////////
// Erase space point information
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::erase()
{
  for (int i=0; i!=m_nrfz; ++i) {
    int n = m_rfz_index[i];
    m_rfz_map[n] = 0;
    m_rfz_Sorted[n].clear();
  }
  
  for (int i=0; i!=m_nrfzv; ++i) {
    int n = m_rfzv_index[i];
    m_rfzv_map[n] = 0;
    m_rfzv_Sorted[n].clear();
  }
  m_state = 0;
  m_nsaz  = 0;
  m_nsazv = 0;
  m_nrfz  = 0;
  m_nrfzv = 0;
}

///////////////////////////////////////////////////////////////////
// Test is space point used
///////////////////////////////////////////////////////////////////

bool InDet::SiSpacePointsSeedMaker_ITK::isUsed(const Trk::SpacePoint* sp)
{
  const Trk::PrepRawData* d = sp->clusterList().first;
  if (!d || !m_assoTool->isUsed(*d)) return false;

  d = sp->clusterList().second;
  if (!d || m_assoTool->isUsed(*d)) return true;

  return false;
}

///////////////////////////////////////////////////////////////////
// 2 space points seeds production
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::production2Sp()
{
  if (m_nsazv<2) return;

  std::list<InDet::SiSpacePointForSeedITK*>::iterator r0,r0e,r,re;
  int nseed = 0;

  // Loop thorugh all azimuthal regions
  //
  for (int f=m_fvNmin; f<=m_fvNmax; ++f) {

    // For each azimuthal region loop through Z regions
    //
    int z = 0;
    if (!m_endlist) z = m_zMin;
    for (; z!=3; ++z) {
      
      int a = f*3+z;
      if (!m_rfzv_map[a]) continue;
      r0  = m_rfzv_Sorted[a].begin();
      r0e = m_rfzv_Sorted[a].end  ();

      if (!m_endlist) {
        r0 = m_rMin;
        m_endlist = true;
      }

      // Loop through trigger space points
      //
      for (; r0!=r0e; ++r0) {

	float X  = (*r0)->x();
	float Y  = (*r0)->y();
	float R  = (*r0)->radius();
	if (R<m_r2minv) continue;
        if (R>m_r2maxv) break;
	float Z  = (*r0)->z();
	float ax = X/R;
	float ay = Y/R;

	// Bottom links production
	//
	int NB = m_rfzv_n[a];
	for (int i=0; i!=NB; ++i) {
	  
	  int an = m_rfzv_i[a][i];
	  if (!m_rfzv_map[an]) continue;

	  r  =  m_rfzv_Sorted[an].begin();
	  re =  m_rfzv_Sorted[an].end  ();
	  
	  for (; r!=re; ++r) {
	    
	    float Rb =(*r)->radius();
	    if (Rb<m_r1minv) continue;
            if (Rb>m_r1maxv) break;
	    float dR = R-Rb;
	    if (dR<m_drminv) break;
            if (dR>m_drmax) continue;
	    float dZ = Z-(*r)->z();
	    float Tz = dZ/dR;
            if (Tz<m_dzdrmin || Tz>m_dzdrmax) continue;
	    float Zo = Z-R*Tz;

	    // Comparison with vertices Z coordinates
	    //
	    if (!isZCompatible(Zo,Rb,Tz)) continue;

	    // Momentum cut
	    //
	    float dx =(*r)->x()-X;
	    float dy =(*r)->y()-Y;
	    float x  = dx*ax+dy*ay          ;
	    float y  =-dx*ay+dy*ax          ;
	    float xy = x*x+y*y              ; if (xy == 0.) continue;
	    float r2 = 1./xy                ;
	    float Ut = x*r2                 ;
	    float Vt = y*r2                 ;
	    float UR = Ut*R+1.              ; if (UR == 0.) continue;
	    float A  = Vt*R/UR              ;
	    float B  = Vt-A*Ut              ;
	    if (fabs(B*m_K) > m_ipt*sqrt(1.+A*A)) continue;
            ++nseed;
	    newSeed((*r),(*r0),Zo);
	  }
	}
	if (nseed < m_maxsize) continue;
	m_endlist=false;
        m_rMin = (++r0);
        m_fvNmin=f;
        m_zMin=z;
	return;
      }
    }
  }
  m_endlist = true;
}

///////////////////////////////////////////////////////////////////
// Production 3 space points seeds 
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::production3Sp()
{ 
  if (m_nsaz<3) return;
  m_seeds.clear();

  const int   ZI[SizeZ]= {5,6,7,8,9,10,4,3,2,1,0};
  std::list<InDet::SiSpacePointForSeedITK*>::iterator rt[9],rte[9],rb[9],rbe[9];
  int nseed = 0;

  // Loop thorugh all azimuthal regions
  //
  for (int f=m_fNmin; f<=m_fNmax; ++f) {
    
    // For each azimuthal region loop through all Z regions
    //
    int z = 0;
    if (!m_endlist) z = m_zMin;

    for (; z<SizeZ; ++z) {

      int a  = f *SizeZ+ZI[z];
      if (!m_rfz_map[a]) continue;
      int NB = 0, NT = 0;
      for (int i=0; i!=m_rfz_b[a]; ++i) {
	
	int an =  m_rfz_ib[a][i];
	if (!m_rfz_map[an]) continue;
	rb [NB] = m_rfz_Sorted[an].begin();
        rbe[NB++] = m_rfz_Sorted[an].end();
      } 
      for (int i=0; i!=m_rfz_t[a]; ++i) {
	
	int an =  m_rfz_it[a][i];
	if (!m_rfz_map[an]) continue;
	rt [NT] = m_rfz_Sorted[an].begin();
        rte[NT++] = m_rfz_Sorted[an].end();
      } 

      if (m_iteration == 0  && m_iteration0 ==0) production3SpSSS(rb,rbe,rt,rte,NB,NT,nseed);
      else                                       production3SpPPP(rb,rbe,rt,rte,NB,NT,nseed);

      if (!m_endlist) {
        m_fNmin=f; 
        m_zMin = z; return;
      }
    }
  }
  m_endlist = true;
}

///////////////////////////////////////////////////////////////////
// Production 3 pixel space points seeds for full scan
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::production3SpPPP
( std::list<InDet::SiSpacePointForSeedITK*>::iterator* rb ,
  std::list<InDet::SiSpacePointForSeedITK*>::iterator* rbe,
  std::list<InDet::SiSpacePointForSeedITK*>::iterator* rt ,
  std::list<InDet::SiSpacePointForSeedITK*>::iterator* rte,
  int NB, int NT, int& nseed) 
{
  std::list<InDet::SiSpacePointForSeedITK*>::iterator r0=rb[0],r;
  if (!m_endlist) {
    r0 = m_rMin;
    m_endlist = true;
  }

  float ipt2K = m_ipt2K   ;
  float ipt2C = m_ipt2C   ;
  float COFK  = m_COFK    ;
  float imaxp = m_diver   ;
  float imaxs = m_divermax;

  m_CmSp.clear();

  // Loop through all trigger space points
  //
  for (; r0!=rbe[0]; ++r0) {

    m_nOneSeeds = 0;
    m_mapOneSeeds.clear();

    float R  = (*r0)->radius();

    const Trk::Surface* sur0 = (*r0)->sur();
    const Trk::Surface* surn = (*r0)->sun();
    float               X    = (*r0)->x()  ;
    float               Y    = (*r0)->y()  ;
    float               Z    = (*r0)->z()  ;
    int                 Nb   = 0           ;

    // Bottom links production
    //
    for (int i=0; i!=NB; ++i) {

      for (r=rb[i]; r!=rbe[i]; ++r) {
	
	float Rb =(*r)->radius();
	float dR = R-Rb;

	if (dR > m_drmax) {
          rb[i]=r;
          continue;
        }
	if (dR < m_drmin) break;
	if ((*r)->sur()==sur0 || (surn && surn==(*r)->sun())) continue;

	float Tz = (Z-(*r)->z())/dR, aTz =fabs(Tz);

	if (aTz < m_dzdrmin || aTz > m_dzdrmax) continue;
	
	// Comparison with vertices Z coordinates
	//
	float Zo = Z-R*Tz;
        if (!isZCompatible(Zo,Rb,Tz)) continue;
	m_SP[Nb] = (*r);
        if (++Nb==m_maxsizeSP) goto breakb;
      }
    }
  breakb:
    if (!Nb || Nb==m_maxsizeSP) continue;
    int Nt = Nb;
    
    // Top   links production
    //
    for (int i=0; i!=NT; ++i) {
      
      for (r=rt[i]; r!=rte[i]; ++r) {
	
	float Rt =(*r)->radius();
	float dR = Rt-R;
	
	if (dR<m_drmin) {
          rt[i]=r;
          continue;
        }
	if (dR>m_drmax) break;

	if ( (*r)->sur()==sur0 || (surn && surn==(*r)->sun())) continue;

	float Tz = ((*r)->z()-Z)/dR, aTz =fabs(Tz);

	if (aTz < m_dzdrmin || aTz > m_dzdrmax) continue;

	// Comparison with vertices Z coordinates
	//
	float Zo = Z-R*Tz;
        if (!isZCompatible(Zo,R ,Tz)) continue;
  	m_SP[Nt] = (*r);
        if (++Nt==m_maxsizeSP) goto breakt;
      }
    }
    
  breakt:
    if (!(Nt-Nb)) continue;
    float covr0 = (*r0)->covr ();
    float covz0 = (*r0)->covz ();
    float ax    = X/R           ;
    float ay    = Y/R           ;

    for (int i=0; i!=Nt; ++i) {

      InDet::SiSpacePointForSeedITK* sp = m_SP[i];

      float dx  = sp->x()-X   ;
      float dy  = sp->y()-Y   ;
      float dz  = sp->z()-Z   ;
      float x   = dx*ax+dy*ay ;
      float y   = dy*ax-dx*ay ;
      float r2  = 1./(x*x+y*y);
      float dr  = sqrt(r2)    ;
      float tz  = dz*dr       ;
      if (i < Nb) tz = -tz;

      m_Tz[i]   = tz                                            ;
      m_Zo[i]   = Z-R*tz                                        ;
      m_R [i]   = dr                                            ;
      m_U [i]   = x*r2                                          ;
      m_V [i]   = y*r2                                          ;
      m_Er[i]   = ((covz0+sp->covz())+(tz*tz)*(covr0+sp->covr()))*r2;
    }
    covr0      *= .5;
    covz0      *= 2.;
   
    // Three space points comparison
    //
    for (int b=0; b!=Nb; ++b) {
    
      float  Zob  = m_Zo[b]      ;
      float  Tzb  = m_Tz[b]      ;
      float  Rb2r = m_R [b]*covr0;
      float  Rb2z = m_R [b]*covz0;
      float  Erb  = m_Er[b]      ;
      float  Vb   = m_V [b]      ;
      float  Ub   = m_U [b]      ;
      float  Tzb2 = (1.+Tzb*Tzb) ;
      float sTzb2 = sqrt(Tzb2)   ;
      float  CSA  = Tzb2*COFK    ;
      float ICSA  = Tzb2*ipt2C   ;
      float imax  = imaxp        ;
      if (m_SP[b]->spacepoint->clusterList().second) imax = imaxs;
  
      for (int t=Nb; t!=Nt; ++t) {
	
	float dT  = ((Tzb-m_Tz[t])*(Tzb-m_Tz[t])-m_R[t]*Rb2z-(Erb+m_Er[t]))-(m_R[t]*Rb2r)*((Tzb+m_Tz[t])*(Tzb+m_Tz[t]));
	if ( dT > ICSA) continue;

	float dU  = m_U[t]-Ub; if (dU == 0.) continue;
	float A   = (m_V[t]-Vb)/dU                   ;
	float S2  = 1.+A*A                           ;
	float B   = Vb-A*Ub                          ;
	float B2  = B*B                              ;
	if (B2  > ipt2K*S2 || dT*S2 > B2*CSA) continue;

	float Im  = fabs((A-B*R)*R)                  ;

	if (Im <= imax) {
	  float dr;
          m_R[t] < m_R[b] ? dr = m_R[t] : dr = m_R[b];
          Im+=fabs((Tzb-m_Tz[t])/(dr*sTzb2));
	  m_CmSp.push_back(std::make_pair(B/sqrt(S2),m_SP[t]));
          m_SP[t]->setParam(Im);
	}
      }
      if (!m_CmSp.empty()) {newOneSeedWithCurvaturesComparison(m_SP[b],(*r0),Zob);}
    }
    fillSeeds();
    nseed += m_fillOneSeeds;
    if (nseed>=m_maxsize) {
      m_endlist=false;
      ++r0;
      m_rMin = r0;
      return;
    }
  }
}

///////////////////////////////////////////////////////////////////
// Production 3 SCT space points seeds for full scan
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::production3SpSSS
( std::list<InDet::SiSpacePointForSeedITK*>::iterator* rb ,
  std::list<InDet::SiSpacePointForSeedITK*>::iterator* rbe,
  std::list<InDet::SiSpacePointForSeedITK*>::iterator* rt ,
  std::list<InDet::SiSpacePointForSeedITK*>::iterator* rte,
  int NB, int NT, int& nseed) 
{
  std::list<InDet::SiSpacePointForSeedITK*>::iterator r0=rb[0],r;
  if (!m_endlist) {
    r0 = m_rMin;
    m_endlist = true;
  }

  float ipt2K = m_ipt2K   ;
  float ipt2C = m_ipt2C   ;
  float COFK  = m_COFK    ;
  float imaxs = m_divermax;

  m_CmSp.clear();

  // Loop through all trigger space points
  //
  for (; r0!=rbe[0]; ++r0) {

    m_nOneSeeds = 0;
    m_mapOneSeeds.clear();

    float R  = (*r0)->radius();

    const Trk::Surface* sur0 = (*r0)->sur();
    const Trk::Surface* surn = (*r0)->sun();
    float               X    = (*r0)->x()  ;
    float               Y    = (*r0)->y()  ;
    float               Z    = (*r0)->z()  ;
    int                 Nb   = 0           ;

    // Bottom links production
    //
    for (int i=0; i!=NB; ++i) {

      for (r=rb[i]; r!=rbe[i]; ++r) {
	
	float Rb =(*r)->radius();
	float dR = R-Rb;

	if (dR > m_drmax) {
          rb[i]=r;
          continue;
        }
	if (dR < m_drmin) break;
	if ((*r)->sur()==sur0 || (surn && surn==(*r)->sun())) continue;
	float Tz = (Z-(*r)->z())/dR, aTz =fabs(Tz);

	if (aTz < m_dzdrmin || aTz > m_dzdrmax) continue;

	// Comparison with vertices Z coordinates
	//
	float Zo = Z-R*Tz;
        if (!isZCompatible(Zo,Rb,Tz)) continue;
	m_SP[Nb] = (*r);
        if (++Nb==m_maxsizeSP) goto breakb;
      }
    }
  breakb:
    if (!Nb || Nb==m_maxsizeSP) continue;
    int Nt = Nb;
    
    // Top   links production
    //
    for (int i=0; i!=NT; ++i) {
      
      for (r=rt[i]; r!=rte[i]; ++r) {
	
	float Rt =(*r)->radius();
	float dR = Rt-R;
	
	if (dR<m_drmin) {
          rt[i]=r; 
          continue;
        }
	if (dR>m_drmax) break;

	if ( (*r)->sur()==sur0 || (surn && surn==(*r)->sun())) continue;
	float Tz = ((*r)->z()-Z)/dR, aTz =fabs(Tz);

	if (aTz < m_dzdrmin || aTz > m_dzdrmax) continue;

	// Comparison with vertices Z coordinates
	//
	float Zo = Z-R*Tz;
        if (!isZCompatible(Zo,R ,Tz)) continue;
  	m_SP[Nt] = (*r);
        if (++Nt==m_maxsizeSP) goto breakt;
      }
    }
    
  breakt:
    if (!(Nt-Nb)) continue;
    float covr0 = (*r0)->covr ();
    float covz0 = (*r0)->covz ();
    float ax    = X/R           ;
    float ay    = Y/R           ;

    for (int i=0; i!=Nt; ++i) {

      InDet::SiSpacePointForSeedITK* sp = m_SP[i];

      float dx  = sp->x()-X   ;
      float dy  = sp->y()-Y   ;
      float dz  = sp->z()-Z   ;
      float x   = dx*ax+dy*ay ;
      float y   = dy*ax-dx*ay ;
      float r2  = 1./(x*x+y*y);
      float dr  = sqrt(r2)    ;
      float tz  = dz*dr       ;
      if (i < Nb) tz = -tz;

      m_X [i]   = x                                             ;
      m_Y [i]   = y                                             ;
      m_Tz[i]   = tz                                            ;
      m_Zo[i]   = Z-R*tz                                        ;
      m_R [i]   = dr                                            ;
      m_U [i]   = x*r2                                          ;
      m_V [i]   = y*r2                                          ;
      m_Er[i]   = ((covz0+sp->covz())+(tz*tz)*(covr0+sp->covr()))*r2;
    }
    covr0      *= .5;
    covz0      *= 2.;
   
    // Three space points comparison
    //
    for (int b=0; b!=Nb; ++b) {
    
      float  Zob  = m_Zo[b]      ;
      float  Tzb  = m_Tz[b]      ;
      float  Rb2r = m_R [b]*covr0;
      float  Rb2z = m_R [b]*covz0;
      float  Erb  = m_Er[b]      ;
      float  Vb   = m_V [b]      ;
      float  Ub   = m_U [b]      ;
      float  Tzb2 = (1.+Tzb*Tzb) ;
      float sTzb2 = sqrt(Tzb2)   ;
      float  CSA  = Tzb2*COFK    ;
      float ICSA  = Tzb2*ipt2C   ;
      float imax  = imaxs        ;
      
      float Se    = 1./sqrt(1.+Tzb*Tzb);
      float Ce    = Se*Tzb             ;
      float Sx    = Se*ax              ;
      float Sy    = Se*ay              ;

      for (int t=Nb; t!=Nt; ++t) {


	// Trigger point
	//	
	float dU0   =  m_U[t]-Ub       ;
        if (dU0 == 0.) continue; 
	float A0    = (m_V[t]-Vb)/dU0  ;
	float C0    = 1./sqrt(1.+A0*A0); 
	float S0    = A0*C0            ;
	float d0[3] = {Sx*C0-Sy*S0, Sx*S0+Sy*C0, Ce};
	float rn[3];
        if (!(*r0)->coordinates(d0,rn)) continue;

	// Bottom  point
	//
	float B0    = 2.*(Vb-A0*Ub);
	float Cb    = (1.-B0*m_Y[b])*C0;
	float Sb    = (A0+B0*m_X[b])*C0;
	float db[3] = {Sx*Cb-Sy*Sb,Sx*Sb+Sy*Cb,Ce};
	float rbDup[3]; //a new and different rb
	if (!m_SP[b]->coordinates(db,rbDup)) continue;

	// Top     point
	//
	float Ct    = (1.-B0*m_Y[t])*C0;
	float St    = (A0+B0*m_X[t])*C0;
	float dt[3] = {Sx*Ct-Sy*St,Sx*St+Sy*Ct,Ce};
	float rtDup[3]; //doesnt hide previous declaration of rt
	if (!m_SP[t]->coordinates(dt,rtDup)) continue;

	float xb    = rbDup[0]-rn[0];
	float yb    = rbDup[1]-rn[1];
	float xt    = rtDup[0]-rn[0];
	float yt    = rtDup[1]-rn[1];

	float rb2   = 1./(xb*xb+yb*yb);
	float rt2   = 1./(xt*xt+yt*yt);
	
	float tb    =  (rn[2]-rbDup[2])*sqrt(rb2);
	float tz    =  (rtDup[2]-rn[2])*sqrt(rt2);

	float dT  = ((tb-tz)*(tb-tz)-m_R[t]*Rb2z-(Erb+m_Er[t]))-(m_R[t]*Rb2r)*((tb+tz)*(tb+tz));
	if ( dT > ICSA) continue;

	float Rn    = sqrt(rn[0]*rn[0]+rn[1]*rn[1]);
	float Ax    = rn[0]/Rn;
	float Ay    = rn[1]/Rn;

	float ub    = (xb*Ax+yb*Ay)*rb2;
	float vb    = (yb*Ax-xb*Ay)*rb2;
	float ut    = (xt*Ax+yt*Ay)*rt2;
	float vt    = (yt*Ax-xt*Ay)*rt2;
	
	float dU  = ut-ub;
	if (dU == 0.) continue;
	float A   = (vt-vb)/dU;
	float S2  = 1.+A*A                           ;
	float B   = vb-A*ub                          ;
	float B2  = B*B                              ;
	if (B2  > ipt2K*S2 || dT*S2 > B2*CSA) continue;

	float Im  = fabs((A-B*Rn)*Rn)                ;

	if (Im <= imax) {
	  float dr;
	  m_R[t] < m_R[b] ? dr = m_R[t] : dr = m_R[b];
	  Im+=fabs((Tzb-m_Tz[t])/(dr*sTzb2));
	  m_CmSp.push_back(std::make_pair(B/sqrt(S2),m_SP[t]));
	  m_SP[t]->setParam(Im);
	}
	
      }
      if (!m_CmSp.empty()) {newOneSeedWithCurvaturesComparison(m_SP[b],(*r0),Zob);}
    }
    fillSeeds();
    nseed += m_fillOneSeeds;
    if (nseed>=m_maxsize) {
      m_endlist=false;
      ++r0;
      m_rMin = r0;
      return;
    }
  }
}

///////////////////////////////////////////////////////////////////
// Production 3 space points seeds in ROI
///////////////////////////////////////////////////////////////////

 
void InDet::SiSpacePointsSeedMaker_ITK::production3SpTrigger
( std::list<InDet::SiSpacePointForSeedITK*>::iterator* rb ,
  std::list<InDet::SiSpacePointForSeedITK*>::iterator* rbe,
  std::list<InDet::SiSpacePointForSeedITK*>::iterator* rt ,
  std::list<InDet::SiSpacePointForSeedITK*>::iterator* rte,
  int NB, int NT, int& nseed) 
{
  std::list<InDet::SiSpacePointForSeedITK*>::iterator r0=rb[0],r;
  if (!m_endlist) {r0 = m_rMin; m_endlist = true;}

  constexpr float pi2 = 2.*M_PI;

  float ipt2K = m_ipt2K   ;
  float ipt2C = m_ipt2C   ;
  float COFK  = m_COFK    ;
  float imaxp = m_diver   ;
  float imaxs = m_diversss;

  m_CmSp.clear();

  // Loop through all trigger space points
  //
  for (; r0!=rbe[0]; ++r0) {

    m_nOneSeeds = 0;
    m_mapOneSeeds.clear();
	
    float R  = (*r0)->radius();

    const Trk::Surface* sur0 = (*r0)->sur();
    float               X    = (*r0)->x()  ;
    float               Y    = (*r0)->y()  ;
    float               Z    = (*r0)->z()  ;
    int                 Nb   = 0           ;

    // Bottom links production
    //
    for (int i=0; i!=NB; ++i) {

      for (r=rb[i]; r!=rbe[i]; ++r) {
	
	float Rb =(*r)->radius();

	float dR = R-Rb;
	if (dR < m_drmin || (m_iteration && (*r)->spacepoint->clusterList().second)) break;
	if (dR > m_drmax || (*r)->sur()==sur0) continue;

	// Comparison with  bottom and top Z 
	//
	float Tz = (Z-(*r)->z())/dR;
	float Zo = Z-R*Tz          ;
        if (Zo < m_zminB || Zo > m_zmaxB) continue;
	float Zu = Z+(550.-R)*Tz   ;
        if (Zu < m_zminU || Zu > m_zmaxU) continue;
	m_SP[Nb] = (*r);
        if (++Nb==m_maxsizeSP) goto breakb;
      }
    }
  breakb:
    if (!Nb || Nb==m_maxsizeSP) continue;
    int Nt = Nb;
    
    // Top   links production
    //
    for (int i=0; i!=NT; ++i) {
      
      for (r=rt[i]; r!=rte[i]; ++r) {
	
	float Rt =(*r)->radius();
	float dR = Rt-R;
	
	if (dR<m_drmin) {
          rt[i]=r;
          continue;
        }
	if (dR>m_drmax) break;

	if ( (*r)->sur()==sur0) continue;

	// Comparison with  bottom and top Z 
	//
	float Tz = ((*r)->z()-Z)/dR;
	float Zo = Z-R*Tz          ;
        if (Zo < m_zminB || Zo > m_zmaxB) continue;
	float Zu = Z+(550.-R)*Tz   ;
        if (Zu < m_zminU || Zu > m_zmaxU) continue;
  	m_SP[Nt] = (*r);
        if (++Nt==m_maxsizeSP) goto breakt;
      }
    }
    
  breakt:
    if (!(Nt-Nb)) continue;
    float covr0 = (*r0)->covr ();
    float covz0 = (*r0)->covz ();

    float ax   = X/R;
    float ay   = Y/R;
    
    for (int i=0; i!=Nt; ++i) {

      InDet::SiSpacePointForSeedITK* sp = m_SP[i];

      float dx  = sp->x()-X   ;
      float dy  = sp->y()-Y   ;
      float dz  = sp->z()-Z   ;
      float x   = dx*ax+dy*ay ;
      float y   = dy*ax-dx*ay ;
      float r2  = 1./(x*x+y*y);
      float dr  = sqrt(r2)    ;
      float tz  = dz*dr       ;
      if (i < Nb) tz = -tz;

      m_X [i]   = x                                             ;
      m_Y [i]   = y                                             ;
      m_Tz[i]   = tz                                            ;
      m_Zo[i]   = Z-R*tz                                        ;
      m_R [i]   = dr                                            ;
      m_U [i]   = x*r2                                          ;
      m_V [i]   = y*r2                                          ;
      m_Er[i]   = ((covz0+sp->covz())+(tz*tz)*(covr0+sp->covr()))*r2;
    }
    covr0      *= .5;
    covz0      *= 2.;
   
    // Three space points comparison
    //
    for (int b=0; b!=Nb; ++b) {
    
      float  Zob  = m_Zo[b]      ;
      float  Tzb  = m_Tz[b]      ;
      float  Rb2r = m_R [b]*covr0;
      float  Rb2z = m_R [b]*covz0;
      float  Erb  = m_Er[b]      ;
      float  Vb   = m_V [b]      ;
      float  Ub   = m_U [b]      ;
      float  Tzb2 = (1.+Tzb*Tzb) ;
      float  CSA  = Tzb2*COFK    ;
      float ICSA  = Tzb2*ipt2C   ;
      float imax  = imaxp        ;
      if (m_SP[b]->spacepoint->clusterList().second) imax = imaxs;
      
      for (int t=Nb;  t!=Nt; ++t) {

	float dT  = ((Tzb-m_Tz[t])*(Tzb-m_Tz[t])-m_R[t]*Rb2z-(Erb+m_Er[t]))-(m_R[t]*Rb2r)*((Tzb+m_Tz[t])*(Tzb+m_Tz[t]));
	if ( dT > ICSA) continue;
	float dU  = m_U[t]-Ub;
        if (dU == 0.) continue;
	float A   = (m_V[t]-Vb)/dU                   ;
	float S2  = 1.+A*A                           ;
	float B   = Vb-A*Ub                          ;
	float B2  = B*B                              ;
	if (B2  > ipt2K*S2 || dT*S2 > B2*CSA) continue;

	float Im  = fabs((A-B*R)*R)                  ;
	if (Im > imax) continue;

	// Azimuthal angle test
	//
	float y  = 1.;
	float x  = 2.*B*R-A;
	float df = fabs(atan2(ay*y-ax*x,ax*y+ay*x)-m_ftrig);
	if (df > M_PI) df=pi2-df;
	if (df > m_ftrigW) continue;
	m_CmSp.push_back(std::make_pair(B/sqrt(S2),m_SP[t]));
        m_SP[t]->setParam(Im);
      }
      if (!m_CmSp.empty()) {newOneSeedWithCurvaturesComparison(m_SP[b],(*r0),Zob);}
    }
    fillSeeds();
    nseed += m_fillOneSeeds;
    if (nseed>=m_maxsize) {
      m_endlist=false;
      ++r0;
      m_rMin = r0;
      return;
    } 
  }
}

///////////////////////////////////////////////////////////////////
// New 3 space points pro seeds 
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::newOneSeed
(InDet::SiSpacePointForSeedITK*& p1, InDet::SiSpacePointForSeedITK*& p2,
 InDet::SiSpacePointForSeedITK*& p3,float z,float q)
{
  if (m_nOneSeeds < m_maxOneSize) {

    m_OneSeeds[m_nOneSeeds].set(p1,p2,p3,z);
    m_mapOneSeeds.insert(std::make_pair(q, &(m_OneSeeds[m_nOneSeeds])));
    ++m_nOneSeeds;
  } else {
    std::multimap<float,InDet::SiSpacePointsProSeedITK*>::reverse_iterator 
      l = m_mapOneSeeds.rbegin();

    if ((*l).first <= q) return;
    
    InDet::SiSpacePointsProSeedITK* s = (*l).second;
    s->set(p1,p2,p3,z);

    std::multimap<float,InDet::SiSpacePointsProSeedITK*>::iterator 
      i = m_mapOneSeeds.insert(std::make_pair(q,s));
	
    for (++i; i!=m_mapOneSeeds.end(); ++i) {
      if ((*i).second==s) {
        m_mapOneSeeds.erase(i);
        return;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////
// New 3 space points pro seeds production
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::newOneSeedWithCurvaturesComparison
(SiSpacePointForSeedITK*& SPb,SiSpacePointForSeedITK*& SP0,float Zob)
{
  const float dC = .00003;

  bool  pixb = !SPb->spacepoint->clusterList().second;

  std::sort(m_CmSp.begin(),m_CmSp.end(),comCurvatureITK());
  std::vector<std::pair<float,InDet::SiSpacePointForSeedITK*>>::iterator j,jn,i = m_CmSp.begin(),ie = m_CmSp.end();
  jn=i;
      
  for (; i!=ie; ++i) {

    float u    = (*i).second->param();

    bool                pixt = !(*i).second->spacepoint->clusterList().second;
    
    if (pixt && fabs(SPb->z() -(*i).second->z()) > m_dzmaxPPP) continue;

    const Trk::Surface* Sui  = (*i).second->sur   ();
    float               Ri   = (*i).second->radius();
    float               Ci1  =(*i).first-dC         ;
    float               Ci2  =(*i).first+dC         ;
    float               Rmi  = 0.                   ;
    float               Rma  = 0.                   ;
    bool                in   = false;
    
    if      (!pixb) u-=400.;
    else if ( pixt) u-=200.;

    for (j=jn;  j!=ie; ++j) {
      
      if (       j == i           ) continue;
      if ( (*j).first < Ci1       ) {jn=j; ++jn; continue;}
      if ( (*j).first > Ci2       ) break;
      if ( (*j).second->sur()==Sui) continue;
      
      float Rj = (*j).second->radius();
      if (fabs(Rj-Ri) < m_drmin) continue;

      if (in) {
	if      (Rj > Rma) Rma = Rj;
	else if (Rj < Rmi) Rmi = Rj;
	else continue;
	if ( (Rma-Rmi) > 20.) {
          u-=200.;
          break;
        }
      } else {
	in=true;
        Rma=Rmi=Rj;
        u-=200.;
      }
    }
    if (u > m_umax) continue;

    newOneSeed(SPb,SP0,(*i).second,Zob,u);
  }
  m_CmSp.clear();
}

///////////////////////////////////////////////////////////////////
// Fill seeds
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::fillSeeds ()
{
  m_fillOneSeeds = 0;

  std::multimap<float,InDet::SiSpacePointsProSeedITK*>::iterator 
    lf = m_mapOneSeeds.begin(),
    l  = m_mapOneSeeds.begin(),
    le = m_mapOneSeeds.end  ();
  
  if (l==le) return;

  SiSpacePointsProSeedITK* s = nullptr;

  for (; l!=le; ++l) {

    float w = (*l).first ;
    s       = (*l).second;
    if (l!=lf && s->spacepoint0()->radius() < 43. && w > -200.) continue;
    if (!s->setQuality(w)) continue;
    
    if (m_i_seede!=m_l_seeds.end()) {
      s  = &(*m_i_seede++);
      *s = *(*l).second;
    } else {
      m_l_seeds.push_back(SiSpacePointsProSeedITK(*(*l).second));
      s = &(m_l_seeds.back());
      m_i_seede = m_l_seeds.end();
    }
    
    if      (s->spacepoint0()->spacepoint->clusterList().second) w-=3000.;
    else if (s->spacepoint1()->spacepoint->clusterList().second) w-=2000.;
    else if (s->spacepoint2()->spacepoint->clusterList().second) w-=1000.;

    m_seeds.insert(std::make_pair(w,s));
    ++m_fillOneSeeds;
  }
}

const InDet::SiSpacePointsSeed* InDet::SiSpacePointsSeedMaker_ITK::next()
{
  if (m_nspoint==3) {
    do {
      if (m_i_seed==m_i_seede) {
        findNext();
        if (m_i_seed==m_i_seede) return nullptr;
      }
      ++m_i_seed;
    } while (!(*m_seed++).second->set3(m_seedOutput));
    return &m_seedOutput;
  } else {
    if (m_i_seed==m_i_seede) {
      findNext();
      if (m_i_seed==m_i_seede) return nullptr;
    } 
    (*m_i_seed++).set2(m_seedOutput);
    return &m_seedOutput;
  }
  return nullptr;
}
  

bool InDet::SiSpacePointsSeedMaker_ITK::isZCompatible  
(float& Zv,float& R,float& T)
{
  if (Zv < m_zminU || Zv > m_zmaxU) return false;
  if (!m_isvertex) return true;

  std::set<float>::iterator v=m_l_vertex.begin(),ve=m_l_vertex.end();

  float dZmin = fabs((*v)-Zv);
  for (++v; v!=ve; ++v) {
    float dZ = fabs((*v)-Zv);
    if (dZ >= dZmin) break;
    dZmin=dZ;
  }
  return dZmin < (m_dzver+m_dzdrver*R)*sqrt(1.+T*T);
}

///////////////////////////////////////////////////////////////////
// New space point for seeds 
///////////////////////////////////////////////////////////////////

InDet::SiSpacePointForSeedITK* InDet::SiSpacePointsSeedMaker_ITK::newSpacePoint
(const Trk::SpacePoint*const& sp) 
{
  InDet::SiSpacePointForSeedITK* sps = nullptr;

  float r[15];
  convertToBeamFrameWork(sp, r);

  if (m_checketa) {
    float z = (fabs(r[2])+m_zmax);
    float x = r[0]*m_dzdrmin;
    float y = r[1]*m_dzdrmin;
    if ((z*z )<(x*x+y*y)) return sps;
  }

  if (m_i_spforseed!=m_l_spforseed.end()) {
    sps = &(*m_i_spforseed++);
    sps->set(sp, r);
  } else {
    m_l_spforseed.push_back(InDet::SiSpacePointForSeedITK(sp, r));
    sps = &(m_l_spforseed.back());
    m_i_spforseed = m_l_spforseed.end();
  }
      
  return sps;
}

///////////////////////////////////////////////////////////////////
// New 2 space points seeds 
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::newSeed
(InDet::SiSpacePointForSeedITK*& p1, InDet::SiSpacePointForSeedITK*& p2, float z) 
{
  InDet::SiSpacePointForSeedITK* p3 = nullptr;

  if (m_i_seede!=m_l_seeds.end()) {
    InDet::SiSpacePointsProSeedITK* s = &(*m_i_seede++);
    s->set(p1,p2,p3,z);
  } else {
    m_l_seeds.push_back(InDet::SiSpacePointsProSeedITK(p1, p2, p3, z));
    m_i_seede = m_l_seeds.end();
  }
}
 
