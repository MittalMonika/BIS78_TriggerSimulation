/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
//  Header file for class SiTools_xk
/////////////////////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////////////////////
// Class for trajector elements in Pixels and SCT
/////////////////////////////////////////////////////////////////////////////////
// Version 1.0 3/10/2004 I.Gavrilenko
/////////////////////////////////////////////////////////////////////////////////

#ifndef SiTools_xk_H
#define SiTools_xk_H

#include "InDetConditionsSummaryService/IInDetConditionsTool.h"
#include "MagFieldInterfaces/IMagFieldSvc.h"
#include "TrkExInterfaces/IPatternParametersPropagator.h"
#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkToolInterfaces/IPatternParametersUpdator.h"
#include "TrkToolInterfaces/IPRD_AssociationTool.h"
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"

namespace InDet{

  class SiTools_xk
    {
      ///////////////////////////////////////////////////////////////////
      // Public methods:
      ///////////////////////////////////////////////////////////////////
      
    public:
      
      SiTools_xk();
      SiTools_xk(const SiTools_xk&) = default;
      ~SiTools_xk() = default;
      SiTools_xk& operator  = (const SiTools_xk&) = default;

      ///////////////////////////////////////////////////////////////////
      // Main methods
      ///////////////////////////////////////////////////////////////////
      
      const Trk::MagneticFieldProperties& fieldTool  () const {return *m_fieldtool  ;}

      const Trk::IPatternParametersPropagator*  propTool   () const {return m_proptool   ;}
      const Trk::IPatternParametersUpdator*     updatorTool() const {return m_updatortool;}
      const MagField::IMagFieldSvc*             magfield   () const {return m_fieldService;}  

      const Trk::IRIO_OnTrackCreator*           rioTool    () const {return m_riotool    ;}
      const Trk::IPRD_AssociationTool*          assoTool   () const {return m_assoTool   ;}
      const IInDetConditionsTool*               pixcond    () const {return m_pixcond    ;}
      const IInDetConditionsTool*               sctcond    () const {return m_sctcond    ;}
      const double&                       xi2max     () const {return m_xi2max     ;}
      const double&                       xi2maxBrem () const {return m_xi2maxBrem ;}
      const double&                       xi2maxNoAdd() const {return m_xi2maxNoAdd;}
      const double&                       xi2maxlink () const {return m_xi2maxlink ;}
      const double&                       xi2multi   () const {return m_xi2multi   ;}
      const double&                       pTmin      () const {return m_pTmin      ;}
      const int&                          maxholes   () const {return m_nholesmax  ;}
      const int&                          maxdholes  () const {return m_dholesmax  ;}
      const int&                          clustersmin() const {return m_nclusmin   ;}
      const bool&                         useassoTool() const {return m_useassoTool;}
      const bool&                         multiTrack () const {return m_multitrack ;}
      const bool&                         bremNoise  () const {return m_bremnoise  ;}
      const bool&                         electron   () const {return m_electron   ;}
      const bool&                         heavyion   () const {return m_heavyion   ;}

      void setTools
	(const Trk::IPatternParametersPropagator* ,
	 const Trk::IPatternParametersUpdator*    , 
	 const Trk::IRIO_OnTrackCreator*          , 
	 const Trk::IPRD_AssociationTool*         ,
	 MagField::IMagFieldSvc* 
	 );  
      
      void setTools(const Trk::MagneticFieldProperties*);
      void setTools(const IInDetConditionsTool*, const IInDetConditionsTool*);
      void setXi2pTmin(const double&,const double&,const double&,const double&);
      void setHolesClusters(const int&,const int&,const int&);
      void setAssociation(const int&);
      void setMultiTracks(const int,double);
      void setBremNoise  (bool,bool);
      void setHeavyIon   (bool);

    protected:
      
      ///////////////////////////////////////////////////////////////////
      // Protected Data
      ///////////////////////////////////////////////////////////////////

      const Trk::IPRD_AssociationTool* m_assoTool   ; // PRD-Track assosiation tool
      const Trk::MagneticFieldProperties* m_fieldtool; // Magnetic field properties
      MagField::IMagFieldSvc*        m_fieldService;  // Magnetic field service 
      const Trk::IPatternParametersPropagator* m_proptool; // Propagator tool
      const Trk::IPatternParametersUpdator* m_updatortool; // Updator    tool
      const Trk::IRIO_OnTrackCreator* m_riotool    ;  // RIOonTrack creator
      const IInDetConditionsTool*     m_pixcond    ;  // Condtionos for pixels 
      const IInDetConditionsTool*     m_sctcond    ;  // Conditions for sct

      double                          m_xi2max     ;  // Max Xi2 for updator 
      double                          m_xi2maxBrem ;  // Max Xi2 for updator (brem fit)  
      double                          m_xi2maxNoAdd;  // Max Xi2 outlayer 
      double                          m_xi2maxlink ;  // Max Xi2 for search
      double                          m_xi2multi   ;  // Max Xi2 for multi tracks
      double                          m_pTmin      ;  // min pT
      int                             m_nholesmax  ;  // Max number holes
      int                             m_dholesmax  ;  // Max holes gap
      int                             m_nclusmin   ;  // Min number clusters
      bool                            m_useassoTool;  // Use assosiation tool
      bool                            m_multitrack ;  // Do multi tracks
      bool                            m_bremnoise  ;  // Do brem noise
      bool                            m_electron   ;  // Do electron mode
      bool                            m_heavyion   ;  // Is it heavy ion event

      ///////////////////////////////////////////////////////////////////
      // Methods
      ///////////////////////////////////////////////////////////////////

    };
  
  /////////////////////////////////////////////////////////////////////////////////
  // Inline methods
  /////////////////////////////////////////////////////////////////////////////////

  inline SiTools_xk::SiTools_xk()
    {
      m_assoTool    = nullptr;
      m_fieldtool   = nullptr;
      m_fieldService= nullptr;
      m_proptool    = nullptr;
      m_updatortool = nullptr;
      m_riotool     = nullptr;  
      m_pixcond     = nullptr;
      m_sctcond     = nullptr;
      m_xi2max      = 9.  ;
      m_xi2maxBrem  = 15. ;
      m_xi2maxlink  = 200.;
      m_xi2multi    = 5.  ;
      m_xi2maxNoAdd = 20. ; 
      m_pTmin       = 500.;
      m_nholesmax   = 2   ;
      m_dholesmax   = 1   ;
      m_nclusmin    = 5   ;
      m_useassoTool = false;
      m_multitrack  = false; 
      m_bremnoise   = false;
      m_electron    = false;
      m_heavyion    = false;
    }

  inline void SiTools_xk::setTools
    (const Trk::IPatternParametersPropagator*  PR,
     const Trk::IPatternParametersUpdator*     UP, 
     const Trk::IRIO_OnTrackCreator*           RO,
     const Trk::IPRD_AssociationTool*          AS,
     MagField::IMagFieldSvc*             MS     
     )    
    {
      m_proptool    = PR;
      m_updatortool = UP;
      m_riotool     = RO;
      m_assoTool    = AS; 
      m_fieldService= MS;   
    }

  inline void SiTools_xk::setTools
    (const Trk::MagneticFieldProperties* MF)
    {
      m_fieldtool = MF;
    }

  inline void SiTools_xk::setTools
    (const IInDetConditionsTool* pix,
     const IInDetConditionsTool* sct)
    {
      m_pixcond = pix;
      m_sctcond = sct;
    } 
 
  inline void SiTools_xk::setXi2pTmin
    (const double& xi2m,const double& xi2mNoAdd,const double& xi2ml,const double& pT)    
    {
      m_xi2max      = fabs(xi2m     );
      m_xi2maxNoAdd = fabs(xi2mNoAdd);
      m_xi2maxlink  = fabs(xi2ml    );
      m_pTmin       = fabs(pT       );
    }

  inline void  SiTools_xk::setHolesClusters
    (const int& h,const int& dh,const int& cl)
    {
      m_nholesmax   = h ;
      m_dholesmax   = dh;
      m_nclusmin    = cl;
    }

  inline void  SiTools_xk::setAssociation(const int& A)
    {
      if( m_assoTool &&  A) m_useassoTool = true ; 
      else                  m_useassoTool = false;  
    }
  inline void SiTools_xk::setMultiTracks(const int M,double X)
    {
      M ?  m_multitrack = true : m_multitrack = false;  
      m_xi2multi = X; 
    }

  inline void SiTools_xk::setBremNoise(bool B,bool E)
    {
      m_bremnoise = B;
      m_electron  = E;
    }

  inline void SiTools_xk::setHeavyIon(bool HI)
  {
    m_heavyion = HI;
  }

} // end of name space

#endif // SiTools_xk

