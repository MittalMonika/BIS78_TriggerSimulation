/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Navigator.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKEXTOOLS_NAVIGATOR_H
#define TRKEXTOOLS_NAVIGATOR_H

// Gaudi
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
// Trk
#include "TrkExInterfaces/INavigator.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkVolumes/BoundarySurface.h"
#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkParameters/TrackParameters.h"
// STD
#include <cstring>
#include <exception>
#include <atomic>


namespace Trk {

  class ITrackingGeometrySvc;

  /** Exception to be thrown when TrackingGeometry not found */
  class NavigatorException : public std::exception
  {
     virtual const char* what() const throw()
     { return "Problem with TrackingGeometry loading"; } 
  };

  class IGeometryBuilder;
  class IPropagator;
  class Surface;
  class Track;
  class TrackingVolume;
  class TrackingGeometry;

  typedef std::pair<const NavigationCell*,const NavigationCell*> NavigationPair;   

 /** 
     @class Navigator
      
     Main AlgTool for Navigation in the TrkExtrapolation realm :
     It retrieves the TrackingGeometry from the DetectorStore 
     as the reference Geometry.            

     There's an experimental possibility to use a straightLineApproximation for the 
     Navigation. This is unstable due to wrong cylinder intersections.

     @author Andreas.Salzburger@cern.ch
     */

  class Navigator : public AthAlgTool,
                    virtual public INavigator {
    public:
      /** Constructor */
      Navigator(const std::string&,const std::string&,const IInterface*);
      /** Destructor */
      virtual ~Navigator();
       
      /** AlgTool initailize method.*/
      StatusCode initialize();
      /** AlgTool finalize method */
      StatusCode finalize();
      
      /** INavigator interface method - returns the TrackingGeometry used for navigation */
      virtual const TrackingGeometry* trackingGeometry() const override;   
      
      /** INavigator interface methods - global search for the Volume one is in */
      virtual const TrackingVolume*               volume(const Amg::Vector3D& gp) const override;
            
      /** INavigator interface method - forward hightes TrackingVolume */
      virtual const TrackingVolume*               highestVolume() const override;
        
      /** INavigator interface methods - getting the next BoundarySurface not knowing the Volume*/
      virtual const BoundarySurface<TrackingVolume>* nextBoundarySurface( const IPropagator& prop,
                                                       const TrackParameters& parms,
                                                       PropDirection dir) const override;  
      
      /** INavigator interface methods - getting the next BoundarySurface when knowing the Volume*/
      virtual const BoundarySurface<TrackingVolume>* nextBoundarySurface( const IPropagator& prop,
                                                       const TrackParameters& parms,
                                                       PropDirection dir,
                                                       const TrackingVolume& vol  ) const override;

      /** INavigator interface method - getting the next Volume and the parameter for the next Navigation*/
      virtual const NavigationCell nextTrackingVolume( const IPropagator& prop,
                                               const TrackParameters& parms,
                                               PropDirection dir,
                                               const TrackingVolume& vol) const override;

      /** INavigator interface method - getting the next Volume and the parameter for the next Navigation
        - contains full loop over volume boundaries
      */
      virtual const NavigationCell nextDenseTrackingVolume( const IPropagator& prop,
						    const TrackParameters& parms,
						    const Surface* destination,
						    PropDirection dir, 
						    ParticleHypothesis particle, 
						    const TrackingVolume& vol,
						    double& path) const override;
     
      /** INavigator interface method - getting the closest TrackParameters from a Track to a Surface*/
      virtual const TrackParameters*      closestParameters( const Track& trk,
                                                     const Surface& sf,
                                                     const IPropagator* prop = 0) const override;

      /** INavigator method to resolve navigation at boundary */
      virtual bool atVolumeBoundary( const Trk::TrackParameters* parms, 
							const Trk::TrackingVolume* vol,  
							Trk::PropDirection dir, 
							const Trk::TrackingVolume*& nextVol, 
							double tol) const override;
    
     /** Validation Action:
        Can be implemented optionally, outside access to internal validation steps */
      virtual void validationAction() const override{

      }
      
    private:
      /* 
      * Methods to be overriden by the NavigatorValidation
      */
      virtual void validationInitialize() {
      }
      virtual void validationFill(const Trk::TrackParameters* trackPar) const{
        static int numPrint{0};
        if (m_validationMode && numPrint<10&&trackPar){
         ATH_MSG_INFO("No Validation implemented. Use an instance of NavigatorValidation");
         ++numPrint;
       }
      } 
    
      bool                                               m_validationMode; //!<This becomes a dummy option for now    
      /* 
       ****************************************************************
       * According to Goetz Gaycken this needs special attention marking as
       * @TODO replace by conditions handle.
       */
      StatusCode                               updateTrackingGeometry() const; 
      mutable const TrackingGeometry*           m_trackingGeometry;          //!< the tracking geometry owned by the navigator
      ServiceHandle<Trk::ITrackingGeometrySvc>  m_trackingGeometrySvc;       //!< ToolHandle to the TrackingGeometrySvc
      std::string                               m_trackingGeometryName;      //!< Name of the TrackingGeometry as given in Detector Store
      /******************************************************************/
      
      double                                    m_insideVolumeTolerance;     //!< Tolerance for inside() method of Volumes
      double                                    m_isOnSurfaceTolerance;      //!< Tolerance for isOnSurface() method of BoundarySurfaces 
      bool                                      m_useStraightLineApproximation; //!< use the straight line approximation for the next boundary sf
      bool                                      m_searchWithDistance;        //!< search with new distanceToSurface() method
      
      //------------ Magnetic field properties
      bool                                      m_fastField;
      Trk::MagneticFieldProperties              m_fieldProperties;

      // ------ PERFORMANCE STATISTICS -------------------------------- //
      /* All performance stat counters are atomic (the simplest solution perhaps not the most performant one)*/
      mutable std::atomic<int>                               m_forwardCalls;              //!< counter for forward nextBounday calls
      mutable std::atomic<int>                               m_forwardFirstBoundSwitch;   //!< counter for failed first forward nextBounday calls
      mutable std::atomic<int>                               m_forwardSecondBoundSwitch;  //!< counter for failed second forward nextBounday calls
      mutable std::atomic<int>                               m_forwardThirdBoundSwitch;   //!< counter for failed third forward nextBounday calls
                                                
      mutable std::atomic<int>                               m_backwardCalls;             //!< counter for backward nextBounday calls
      mutable std::atomic<int>                               m_backwardFirstBoundSwitch;  //!< counter for failed first backward nextBounday calls
      mutable std::atomic<int>                               m_backwardSecondBoundSwitch; //!< counter for failed second backward nextBounday calls
      mutable std::atomic<int>                               m_backwardThirdBoundSwitch;  //!< counter for failed third backward nextBounday calls
                                                
      mutable std::atomic<int>                               m_outsideVolumeCase;         //!< counter for navigation-break in outside volume cases (ovc)
      mutable std::atomic<int>                               m_sucessfulBackPropagation;  //!< counter for sucessful recovery of navigation-break in ovc 
      
    };

} // end of namespace


#endif // TRKEXTOOLS_NAVIGATOR_H

