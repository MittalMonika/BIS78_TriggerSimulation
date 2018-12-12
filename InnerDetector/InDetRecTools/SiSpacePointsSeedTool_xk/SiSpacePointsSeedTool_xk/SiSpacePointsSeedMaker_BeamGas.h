/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
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


#include "GaudiKernel/ServiceHandle.h"
#include "MagFieldInterfaces/IMagFieldSvc.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetRecToolInterfaces/ISiSpacePointsSeedMaker.h"
#include "TrkSpacePoint/SpacePointContainer.h" 
#include "TrkSpacePoint/SpacePointOverlapCollection.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "SiSpacePointsSeedTool_xk/SiSpacePointForSeed.h"
#include <list>
#include <map>
#include <vector>
#include <iosfwd>

class MsgStream   ;

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
	(const std::string&,const std::string&,const IInterface*);
      virtual ~SiSpacePointsSeedMaker_BeamGas();
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
  SiSpacePointsSeedMaker_BeamGas() = delete;
  SiSpacePointsSeedMaker_BeamGas(const SiSpacePointsSeedMaker_BeamGas&) = delete;
  SiSpacePointsSeedMaker_BeamGas &operator=(const SiSpacePointsSeedMaker_BeamGas&) = delete;
  //@}
      ///////////////////////////////////////////////////////////////////
      // Protected data and methods
      ///////////////////////////////////////////////////////////////////
      
      ServiceHandle<MagField::IMagFieldSvc>  m_fieldServiceHandle ;
      MagField::IMagFieldSvc*                m_fieldService{}       ;

      ToolHandle<Trk::IPRD_AssociationTool>  m_assoTool           ;
      SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };

      bool                        m_pixel {}                        ;
      bool                        m_sct  {}                         ;
      bool                        m_endlist  {}                     ;
      bool                        m_useOverlap{}                    ;
      bool                        m_useassoTool {}                  ;
      int                         m_outputlevel{}                   ;
      int                         m_nprint {}                       ;
      int                         m_state {}                        ;
      int                         m_nspoint{}                       ;
      int                         m_mode {}                         ;
      int                         m_nlist{}                         ;
      int                         m_maxsize {}                      ;
      float                       m_r1min{}                         ;
      float                       m_r1max {}                        ;
      float                       m_r2min {}                        ;
      float                       m_r2max {}                        ;
      float                       m_r3min {}                        ;
      float                       m_r3max {}                        ;
      float                       m_drmin  {}                       ;
      float                       m_drmax {}                        ;
      float                       m_rapcut {}                       ;
      float                       m_dzdrmin{}                       ;
      float                       m_dzdrmax{}                       ;
      float                       m_zmin  {}                        ;
      float                       m_zmax {}                         ;
      float                       m_r_rmax{}                        ;
      float                       m_r_rstep {}                      ;
      float                       m_dzver {}                        ;
      float                       m_dzdrver {}                      ;
      float                       m_diver {}                        ;
      float                       m_diverpps {}                     ;
      float                       m_dazmax{}                        ;
      float                       m_ptmin {}                        ;
      float                       m_ipt  {}                         ;
      float                       m_ipt2 {}                         ;
      float                       m_COF {}                          ;
      float                       m_K  {}                           ;

      int m_r_size {}                                               ;
      int m_rf_size  {}                                             ;
      int m_rfz_size {}                                             ;
      std::list<InDet::SiSpacePointForSeed*>* m_r_Sorted          ;
      std::list<InDet::SiSpacePointForSeed*>  m_rf_Sorted  [    53] ; 
      std::list<InDet::SiSpacePointForSeed*>  m_rfz_Sorted [   583] ;
      std::list<InDet::SiSpacePointForSeed*>  m_l_spforseed       ;
      std::list<InDet::SiSpacePointForSeed*>::iterator m_i_spforseed; 
      std::list<InDet::SiSpacePointForSeed*>::iterator m_rMin     ;

      int m_ns{},m_nsaz {}                                            ;
      int m_fNmax {}                                                ;
      int m_fNmin {}                                                ;
      int m_zMin {}                                                 ;
      int  m_nr{}     ; int* m_r_index{}   ; int* m_r_map {}            ;
      int  m_nrf {}   , m_rf_index   [ 53], m_rf_map   [ 53]        ;
      int  m_nrfz {}  , m_rfz_index  [583], m_rfz_map  [583]        ;
      int m_rfz_b[583],m_rfz_t[593],m_rfz_ib[583][12],m_rfz_it[583][12]   ;
      float m_sF {}                                                 ;

      ///////////////////////////////////////////////////////////////////
      // Tables for 3 space points seeds search
      ///////////////////////////////////////////////////////////////////
     
      int                     m_maxsizeSP{}                         ;                    
      InDet::SiSpacePointForSeed** m_SP {}                          ;
      float               *  m_R {}                                 ;
      float               *  m_Tz {}                                ;
      float               *  m_Er {}                                ;
      float               *  m_U {}                                 ;
      float               *  m_V {}                                 ;
      float               *  m_Zo{}                                 ; 

      std::list<InDet::SiSpacePointsSeed*>           m_l_seeds    ;
      std::list<InDet::SiSpacePointsSeed*>::iterator m_i_seed     ; 
      std::list<InDet::SiSpacePointsSeed*>::iterator m_i_seede    ;

      std::multimap<float,InDet::SiSpacePointsSeed*> m_mapOneSeeds;
      InDet::SiSpacePointsSeed*                      m_OneSeeds {}  ;
      int                                            m_maxOneSize{} ;
      int                                            m_nOneSeeds{}  ;

      ///////////////////////////////////////////////////////////////////
      // Beam geometry
      ///////////////////////////////////////////////////////////////////
 
      float m_xbeam[4];    // x,ax,ay,az - center and x-axis direction
      float m_ybeam[4];    // y,ax,ay,az - center and y-axis direction
      float m_zbeam[4];    // z,ax,ay,az - center and z-axis direction

      ///////////////////////////////////////////////////////////////////
      // Space points container
      ///////////////////////////////////////////////////////////////////
      

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
      void newSeed
      (const Trk::SpacePoint*&,const Trk::SpacePoint*&,
       const Trk::SpacePoint*&,const float&)                      ;

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
	 int,int,int&)                                            ;
			  
      void findNext()                                             ;
      bool isZCompatible     (float&)                             ;
      void convertToBeamFrameWork(const Trk::SpacePoint*const&,float*)  ;
      bool isUsed(const Trk::SpacePoint*)                         ; 
   };

  MsgStream&    operator << (MsgStream&   ,const SiSpacePointsSeedMaker_BeamGas&);
  std::ostream& operator << (std::ostream&,const SiSpacePointsSeedMaker_BeamGas&); 

  ///////////////////////////////////////////////////////////////////
  // Inline methods
  ///////////////////////////////////////////////////////////////////

  inline const SiSpacePointsSeed* SiSpacePointsSeedMaker_BeamGas::next()
    {
      if(m_i_seed==m_i_seede) {
	findNext(); if(m_i_seed==m_i_seede) return 0;
      } 
      return(*m_i_seed++);
    }
    
  inline bool SiSpacePointsSeedMaker_BeamGas::isZCompatible  
    (float& Zv)
    {
      return  Zv > m_zmin && Zv < m_zmax;
    }
  
  ///////////////////////////////////////////////////////////////////
  // New space point for seeds 
  ///////////////////////////////////////////////////////////////////

  inline SiSpacePointForSeed* SiSpacePointsSeedMaker_BeamGas::newSpacePoint
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

  inline void SiSpacePointsSeedMaker_BeamGas::newSeed
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
  // New 3 space points seeds 
  ///////////////////////////////////////////////////////////////////

  inline void SiSpacePointsSeedMaker_BeamGas::newSeed
      (const Trk::SpacePoint*& p1,const Trk::SpacePoint*& p2, 
       const Trk::SpacePoint*& p3,const float& z) 
    {
      if(m_i_seede!=m_l_seeds.end()) {
	SiSpacePointsSeed* s = (*m_i_seede++);
	s->erase     (  ); 
	s->add       (p1); 
	s->add       (p2); 
	s->add       (p3); 
	s->setZVertex(double(z));
      }
      else                  {
	m_l_seeds.push_back(new SiSpacePointsSeed(p1,p2,p3,z));
	m_i_seede = m_l_seeds.end(); 
      }
    }

  ///////////////////////////////////////////////////////////////////
  // Fill seeds
  ///////////////////////////////////////////////////////////////////

  inline void SiSpacePointsSeedMaker_BeamGas::fillSeeds ()
    {
      std::multimap<float,InDet::SiSpacePointsSeed*>::iterator 
	l  = m_mapOneSeeds.begin(),
	le = m_mapOneSeeds.end  ();

      for(; l!=le; ++l) {

	if(m_i_seede!=m_l_seeds.end()) {
	  SiSpacePointsSeed* s = (*m_i_seede++);
	  *s = *(*l).second;
	}
	else                  {
	  m_l_seeds.push_back(new SiSpacePointsSeed(*(*l).second));
	  m_i_seede = m_l_seeds.end(); 
	}
      }
    }
  
} // end of name space

#endif // SiSpacePointsSeedMaker_BeamGas_H

