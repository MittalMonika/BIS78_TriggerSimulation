// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file InDetD3PDMaker/src/TrackParticleFitQualityAssociationTool.h
 * @author remi zaidan <remi.zaidan@cern.ch>
 * @date Feb, 2010
 * @brief Associate from a TrackParticle to its FitQuality.
 */
#ifndef INDETD3PDMAKER_TRACKPARTICLEFITQUALITYATOOASSOCIATIONTOOL_H
#define INDETD3PDMAKER_TRACKPARTICLEFITQUALITYATOOASSOCIATIONTOOL_H

#include "D3PDMakerUtils/SingleAssociationTool.h"

namespace Trk {
  class FitQuality;
}

namespace Rec {
  class TrackParticle;
}

namespace D3PD {

/**
 * @brief Associate from a TrackParticle to its FitQuality.
 */
class TrackParticleFitQualityAssociationTool
  : public SingleAssociationTool<Rec::TrackParticle, Trk::FitQuality>
{
public:
  typedef SingleAssociationTool<Rec::TrackParticle, Trk::FitQuality> Base;

  /**
   * @brief Standard Gaudi tool constructor.
   * @param type The name of the tool type.
   * @param name The tool name.
   * @param parent The tool's Gaudi parent.
   */
  TrackParticleFitQualityAssociationTool (const std::string& type,
				       const std::string& name,
				       const IInterface* parent);


  /**
   * @brief Return the target object.
   * @param p The source object for the association.
   *
   * Return the target of the association, or 0.
   */
  virtual const Trk::FitQuality* get (const Rec::TrackParticle& p);
};


} // namespace D3PD



#endif // not INDETD3PDMAKER_TRACKPARTICLEFITQUALITYATOOASSOCIATIONTOOL_H
