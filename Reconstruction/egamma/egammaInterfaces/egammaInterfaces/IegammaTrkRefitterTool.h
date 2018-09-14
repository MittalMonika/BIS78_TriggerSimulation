/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAINTERFACES_IEGAMMATRKREFITTERTOOL_H
#define EGAMMAINTERFACES_IEGAMMATRKREFITTERTOOL_H
/** @brief
  @class IxAODEgammaTrackRefitterTool
          Iterface for track refitter
  @author A. Morley
*/

/********************************************************************

NAME:     IxAODEgammaTrkRefitterTool.h
PACKAGE:  offline/Reconstruction/xAODEgamma/xAODEgammaInterfaces

CREATED:  Jul 2008

********************************************************************/

#include "GaudiKernel/IAlgTool.h"

static const InterfaceID IID_egammaTrkRefitterTool("IegammaTrkRefitterTool", 1, 0);

#include "TrkParameters/TrackParameters.h"
// Forward declarations
#include "xAODEgamma/ElectronFwd.h"
#include "xAODTracking/TrackParticleFwd.h"
#include "TrkTrack/Track.h"
#include "TrkParameters/TrackParameters.h"
#include <memory>

class IegammaTrkRefitterTool : virtual public IAlgTool 
{
    public:
    /** Alg tool and IAlgTool interface method */
    static const InterfaceID& interfaceID() 
      { return IID_egammaTrkRefitterTool; };
  
    /** Struct Holding the result to return and intermediate objects
     *  Things are owned by the EDM or the unique_ptr*/
    struct Result {
      /** @brief Pointer to the refitted track*/  
      std::unique_ptr<Trk::Track>   refittedTrack; 
      /** @brief Pointer to the refitted MeasuredPerigee*/    
      const Trk::Perigee* refittedTrackPerigee;
      /** @brief Pointer to the original track*/  
      const Trk::Track*    originalTrack; 
      /** @brief Pointer to the original Perigee*/    
      const Trk::Perigee*  originalTrackPerigee;
     /** @brief pointer to the Electron input*/
      const xAOD::Electron* electron; 
      Result():refittedTrack(nullptr),
      refittedTrackPerigee(nullptr),
      originalTrack(nullptr),
      originalTrackPerigee(nullptr),
      electron(nullptr){
      }
    };

    /** Refit the track associated with an xAODEgamma object*/
    virtual StatusCode  refitElectronTrack(const xAOD::Electron* eg ,Result& result) const = 0;
    
    /** Refit the track associated with a track particle  object*/
    virtual StatusCode  refitTrackParticle(const xAOD::TrackParticle*, Result& result) const = 0;
    
    /** Refit the track*/
		virtual StatusCode  refitTrack(const Trk::Track*, Result& result) const = 0;

};
#endif
