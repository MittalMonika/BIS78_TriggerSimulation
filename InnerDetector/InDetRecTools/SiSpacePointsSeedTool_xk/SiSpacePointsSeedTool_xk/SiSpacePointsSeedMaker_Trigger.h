/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
//  Header file for class SiSpacePointsSeedMaker_Trigger
/////////////////////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////////////////////
// Class for track candidates generation using space points information
// for standard Atlas geometry
/////////////////////////////////////////////////////////////////////////////////
// Version 1.0 3/10/2004 I.Gavrilenko
/////////////////////////////////////////////////////////////////////////////////

#ifndef SiSpacePointsSeedMaker_Trigger_H
#define SiSpacePointsSeedMaker_Trigger_H

#include <list>
#include "GaudiKernel/ServiceHandle.h"
#include "MagFieldInterfaces/IMagFieldSvc.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetRecToolInterfaces/ISiSpacePointsSeedMaker.h"
#include "TrkSpacePoint/SpacePointContainer.h" 
#include "TrkSpacePoint/SpacePointOverlapCollection.h"
#include "InDetBeamSpotService/IBeamCondSvc.h"

#include "SiSpacePointsSeedTool_xk/SiSpacePointForSeed.h"

class MsgStream   ;
class IBeamCondSvc;

namespace InDet {


  class SiSpacePointsSeedMaker_Trigger : 
    virtual public ISiSpacePointsSeedMaker, public AthAlgTool
    {
      ///////////////////////////////////////////////////////////////////
      // Public methods:
      ///////////////////////////////////////////////////////////////////
      
    public:
      
      ///////////////////////////////////////////////////////////////////
      // Standard tool methods
      ///////////////////////////////////////////////////////////////////

      SiSpacePointsSeedMaker_Trigger
	(const std::string&,const std::string&,const IInterface*);
      virtual ~SiSpacePointsSeedMaker_Trigger();
      virtual StatusCode               initialize();
      virtual StatusCode               finalize  ();

      ///////////////////////////////////////////////////////////////////
      // Methods to initialize tool for new event or region
      ///////////////////////////////////////////////////////////////////

      void newEvent (int);
      void newRegion
	(const std::vector<IdentifierHash>&,const std::vector<IdentifierHash>&);
      void newRegion
	(const std::vector<IdentifierHash>&,const std::vector<IdentifierHash>&,const IRoiDescriptor&);
      
      ///////////////////////////////////////////////////////////////////
      // Methods to initilize different strategies of seeds production
      // with two space points with or without vertex constraint
      ///////////////////////////////////////////////////////////////////

      void find2Sp (const std::list<Trk::Vertex>&);

      ///////////////////////////////////////////////////////////////////
      // Methods to initilize different strategies of seeds production
      // with three space points with or without vertex constraint
      ///////////////////////////////////////////////////////////////////

      void find3Sp (const std::list<Trk::Vertex>&);
      void find3Sp (const std::list<Trk::Vertex>&,const double*);

      ///////////////////////////////////////////////////////////////////
      // Methods to initilize different strategies of seeds production
      // with variable number space points with or without vertex constraint
      // Variable means (2,3,4,....) any number space points
      ///////////////////////////////////////////////////////////////////
 
      void findVSp (const std::list<Trk::Vertex>&);
      
      ///////////////////////////////////////////////////////////////////
      // Iterator through seeds pseudo collection produced accordingly
      // methods find    
      ///////////////////////////////////////////////////////////////////
      
      const SiSpacePointsSeed* next();
      
      ///////////////////////////////////////////////////////////////////
      // Print internal tool parameters and status
      ///////////////////////////////////////////////////////////////////

      MsgStream&    dump          (MsgStream   & out) const;
      std::ostream& dump          (std::ostream& out) const;

    protected:
        /**    @name Disallow default instantiation, copy, assignment */
  //@{
  SiSpacePointsSeedMaker_Trigger() = delete;
  SiSpacePointsSeedMaker_Trigger(const SiSpacePointsSeedMaker_Trigger&) = delete;
  SiSpacePointsSeedMaker_Trigger &operator=(const SiSpacePointsSeedMaker_Trigger&) = delete;
  //@}

      ///////////////////////////////////////////////////////////////////
      // Protected data and methods
      ///////////////////////////////////////////////////////////////////
      
      ServiceHandle<MagField::IMagFieldSvc>  m_fieldServiceHandle ;
      MagField::IMagFieldSvc*                m_fieldService       ;

      IBeamCondSvc* p_beam                                        ;
      bool                        m_pixel                         ;
      bool                        m_sct                           ;
      bool                        m_endlist                       ;
      bool                        m_useOverlap                    ;
      bool                        m_trigger                       ;
      int                         m_outputlevel                   ;
      int                         m_nprint                        ;
      int                         m_state                         ;
      int                         m_nspoint                       ;
      int                         m_mode                          ;
      int                         m_nlist                         ;
      int                         m_maxsize                       ;
      unsigned int                m_maxNumberVertices             ;
      float                       m_r1min, m_r1minv               ;
      float                       m_r1max, m_r1maxv               ;
      float                       m_r2min, m_r2minv               ;
      float                       m_r2max, m_r2maxv               ;
      float                       m_r3min                         ;
      float                       m_r3max                         ;
      float                       m_drmin, m_drminv               ;
      float                       m_drmax                         ;
      float                       m_rapcut                        ;
      float                       m_dzdrmin                       ;
      float                       m_dzdrmax                       ;
      float                       m_zmin                          ;
      float                       m_zmax                          ;
      float                       m_zminU                         ;
      float                       m_zmaxU                         ;
      float                       m_zminB                         ;
      float                       m_zmaxB                         ;
      float                       m_ftrig                         ;
      float                       m_ftrigW                        ;
      float                       m_r_rmax                        ;
      float                       m_r_rstep                       ;
      float                       m_dzver                         ;
      float                       m_dzdrver                       ;
      float                       m_diver                         ;
      float                       m_diverpps                      ;
      float                       m_diversss                      ;
      float                       m_dazmax                        ;
      float                       m_ptmin                         ;
      float                       m_ipt                           ;
      float                       m_ipt2                          ;
      float                       m_COF                           ;
      float                       m_K                             ;
      int m_r_size                                                  ;
      int m_rf_size                                                 ;
      int m_rfz_size                                                ;
      std::list<InDet::SiSpacePointForSeed*>* m_r_Sorted            ;
      std::list<InDet::SiSpacePointForSeed*>  m_rfz_Sorted [   583] ;
      std::list<InDet::SiSpacePointForSeed*>  m_rfzv_Sorted[   300] ;
      std::list<InDet::SiSpacePointForSeed*>  m_l_spforseed         ;
      std::list<InDet::SiSpacePointForSeed*>::iterator m_i_spforseed; 
      std::list<InDet::SiSpacePointForSeed*>::iterator m_rMin     ;

      int m_ns,m_nsaz,m_nsazv                                     ;
      int m_fNmax,m_fvNmax                                        ;
      int m_fNmin,m_fvNmin                                        ;
      int m_zMin                                                  ;
      int  m_nr     ; int* m_r_index   ; int* m_r_map                 ;
      int  m_nrfz   , m_rfz_index  [583], m_rfz_map  [583]            ;
      int  m_nrfzv  , m_rfzv_index [300], m_rfzv_map [300]            ;
      int m_rfz_b[583],m_rfz_t[593],m_rfz_ib[583][9],m_rfz_it[583][9]     ;
      int m_rfzv_n[300],m_rfzv_i[300][6]                              ;
      float m_sF                                                  ;
      float m_sFv                                                 ;

      ///////////////////////////////////////////////////////////////////
      // Tables for 3 space points seeds search
      ///////////////////////////////////////////////////////////////////
     
      int    m_maxsizeSP                                          ;                    
      InDet::SiSpacePointForSeed** m_SP                           ;
      float               *  m_Zo                                 ; 
      float               *  m_Tz                                 ;
      float               *  m_R                                  ;
      float               *  m_U                                  ;
      float               *  m_V                                  ;
      float               *  m_Er                                 ;

      std::list<InDet::SiSpacePointsSeed*>           m_l_seeds    ;
      std::list<InDet::SiSpacePointsSeed*>::iterator m_i_seed     ; 
      std::list<InDet::SiSpacePointsSeed*>::iterator m_i_seede    ;

      std::multimap<float,InDet::SiSpacePointsSeed*> m_mapOneSeeds;
      std::multimap<float,InDet::SiSpacePointsSeed*> m_mapSeeds   ;
      std::multimap<float,InDet::SiSpacePointsSeed*>::iterator m_seed;
      std::multimap<float,InDet::SiSpacePointsSeed*>::iterator m_seede;

      InDet::SiSpacePointsSeed*                      m_OneSeeds   ;
      int                                            m_maxOneSize ;
      int                                            m_nOneSeeds  ;
      std::list<float>                               m_l_vertex   ;
 
      ///////////////////////////////////////////////////////////////////
      // Beam geometry
      ///////////////////////////////////////////////////////////////////
 
      float m_xbeam[4];    // x,ax,ay,az - center and x-axis direction
      float m_ybeam[4];    // y,ax,ay,az - center and y-axis direction
      float m_zbeam[4];    // z,ax,ay,az - center and z-axis direction

      ///////////////////////////////////////////////////////////////////
      // Space points container
      ///////////////////////////////////////////////////////////////////
      
//      std::string                        m_spacepointsSCTname     ;
//      std::string                        m_spacepointsPixelname   ;
//      std::string                        m_spacepointsOverlapname ; 
      std::string                        m_beamconditions         ;
      SG::ReadHandle<SpacePointContainer>         m_spacepointsSCT         ;
      SG::ReadHandle<SpacePointContainer>         m_spacepointsPixel       ;
      SG::ReadHandle<SpacePointOverlapCollection> m_spacepointsOverlap     ;

      ///////////////////////////////////////////////////////////////////
      // Protected methods
      ///////////////////////////////////////////////////////////////////

      MsgStream&    dumpConditions(MsgStream   & out) const;
      MsgStream&    dumpEvent     (MsgStream   & out) const;

      void buildFrameWork()                                       ;
      void buildBeamFrameWork()                                   ;

      SiSpacePointForSeed* newSpacePoint
	(const Trk::SpacePoint*const&)                            ;
      void newSeed
      (const Trk::SpacePoint*&,const Trk::SpacePoint*&,
       const float&)                                              ; 

      void newOneSeed
      (const Trk::SpacePoint*&,const Trk::SpacePoint*&,
       const Trk::SpacePoint*&,const float&,const float&)         ;
      void fillSeeds()                                            ;

      void fillLists     ()                                       ;
      void erase         ()                                       ;
      void production2Sp ()                                       ;
      void production3Sp ()                                       ;
      void production3Sp
	(std::list<InDet::SiSpacePointForSeed*>::iterator*,
	 std::list<InDet::SiSpacePointForSeed*>::iterator*,
	 std::list<InDet::SiSpacePointForSeed*>::iterator*,
	 std::list<InDet::SiSpacePointForSeed*>::iterator*,
	 int,int,int&);
      void production3SpTrigger
	(std::list<InDet::SiSpacePointForSeed*>::iterator*,
	 std::list<InDet::SiSpacePointForSeed*>::iterator*,
	 std::list<InDet::SiSpacePointForSeed*>::iterator*,
	 std::list<InDet::SiSpacePointForSeed*>::iterator*,
	 int,int,int&);
 
      bool newVertices(const std::list<Trk::Vertex>&)             ;
      void findNext()                                             ;
      bool isZCompatible     (float&,float&,float&)               ;
      void convertToBeamFrameWork(const Trk::SpacePoint*const&,float*)  ;
      float dZVertexMin(float&)                                   ;
   };

  MsgStream&    operator << (MsgStream&   ,const SiSpacePointsSeedMaker_Trigger&);
  std::ostream& operator << (std::ostream&,const SiSpacePointsSeedMaker_Trigger&); 

  ///////////////////////////////////////////////////////////////////
  // Inline methods
  ///////////////////////////////////////////////////////////////////

  inline const SiSpacePointsSeed* SiSpacePointsSeedMaker_Trigger::next()
    {
      if(m_i_seed==m_i_seede) {findNext(); if(m_i_seed==m_i_seede) return 0;} 
      if(m_mode==0 || m_mode==1) return(*m_i_seed++);
      ++m_i_seed;
      return (*m_seed++).second;
    }
    
  inline bool SiSpacePointsSeedMaker_Trigger::isZCompatible  
    (float& Zv,float& R,float& T)
    {
      if(Zv < m_zmin || Zv > m_zmax) return false;

      std::list<float>::iterator v=m_l_vertex.begin(),ve=m_l_vertex.end(); 
      if(v==ve) return true;      

      float dZmin = fabs((*v)-Zv); ++v;

      for(; v!=ve; ++v) {
	float dZ = fabs((*v)-Zv); if(dZ<dZmin) dZmin=dZ;
      }
      return dZmin < (m_dzver+m_dzdrver*R)*sqrt(1.+T*T);
    }

  inline float SiSpacePointsSeedMaker_Trigger::dZVertexMin(float& Z)
    {
      std::list<float>::iterator v=m_l_vertex.begin(),ve=m_l_vertex.end();
      if(v==ve) return 0.;

      float dZm = 1.E10;
      for(; v!=ve; ++v) {float dZ = fabs((*v)-Z); if(dZ<dZm) dZm = dZ;}
      return dZm;
    }

  ///////////////////////////////////////////////////////////////////
  // New space point for seeds 
  ///////////////////////////////////////////////////////////////////

  inline SiSpacePointForSeed* SiSpacePointsSeedMaker_Trigger::newSpacePoint
    (const Trk::SpacePoint*const& sp) 
    {
      SiSpacePointForSeed* sps;

      float r[3]; convertToBeamFrameWork(sp,r);

      if(m_i_spforseed!=m_l_spforseed.end()) {
	sps = (*m_i_spforseed++); sps->set(sp,r); 
      }
      else                               {
	m_l_spforseed.push_back((sps=new SiSpacePointForSeed(sp,r)));
	m_i_spforseed = m_l_spforseed.end();	
      }
      
      return sps;
    }

  ///////////////////////////////////////////////////////////////////
  // New 2 space points seeds 
  ///////////////////////////////////////////////////////////////////

  inline void SiSpacePointsSeedMaker_Trigger::newSeed
      (const Trk::SpacePoint*& p1,const Trk::SpacePoint*& p2, 
       const float& z) 
    {
      if(m_i_seede!=m_l_seeds.end()) {
	SiSpacePointsSeed* s = (*m_i_seede++);
	s->erase     (  ); 
	s->add       (p1); 
	s->add       (p2); 
	s->setZVertex(double(z));
      }
      else                  {
	m_l_seeds.push_back(new SiSpacePointsSeed(p1,p2,z));
	m_i_seede = m_l_seeds.end(); 
      }
    }

  ///////////////////////////////////////////////////////////////////
  // Fill seeds
  ///////////////////////////////////////////////////////////////////

  inline void SiSpacePointsSeedMaker_Trigger::fillSeeds ()
    {
      std::multimap<float,InDet::SiSpacePointsSeed*>::iterator 
	l  = m_mapOneSeeds.begin(),
	le = m_mapOneSeeds.end  ();

      for(; l!=le; ++l) {

	float                     q  = (*l).first;
	InDet::SiSpacePointsSeed* s0 = (*l).second;

	if((*s0->spacePoints().rbegin())->clusterList().second) {
	  (*s0->spacePoints().begin())->clusterList().second ? q+=1000. : q+=10000.;
	}

	if(m_i_seede!=m_l_seeds.end()) {
	  InDet::SiSpacePointsSeed* s = (*m_i_seede++);
	  *s = *s0;
	  m_mapSeeds.insert(std::make_pair(q,s));
	}
	else                  {

	  InDet::SiSpacePointsSeed* s = new InDet::SiSpacePointsSeed(*s0);
	  m_l_seeds.push_back(s);
	  m_i_seede = m_l_seeds.end(); 
	  m_mapSeeds.insert(std::make_pair(q,s));
	}
      }
    }
  
} // end of name space

#endif // SiSpacePointsSeedMaker_Trigger_H

