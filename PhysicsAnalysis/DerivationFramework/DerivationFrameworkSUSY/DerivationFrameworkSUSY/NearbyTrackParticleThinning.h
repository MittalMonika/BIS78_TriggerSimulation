/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORKCALO_CALOCLUSTERPARTICLETHINNING_H
#define DERIVATIONFRAMEWORKCALO_CALOCLUSTERPARTICLETHINNING_H

// System include(s):
#include <string>
#include <memory>
#include <vector>

// Framework include(s):
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaKernel/IThinningSvc.h"
#include "AthenaBaseComps/AthAlgTool.h"

// DF include(s):
#include "DerivationFrameworkInterfaces/IThinningTool.h"
#include "DerivationFrameworkInterfaces/ExpressionParserHelper.h"

// EDM include(s): 
#include "xAODTracking/TrackParticleContainerFwd.h"

namespace DerivationFramework {

   /// Tool selecting track particles near leptons to keep
   ///
   /// This tool is used in derivation jobs to set up the thinning such that
   /// track particles associated with lepton objects would be kept for a given
   /// output stream.
   ///
   /// @author Mike Hance <michael.hance@cern.ch>
   ///
   class NearbyTrackParticleThinning : public AthAlgTool, public IThinningTool {

   public:
      /// AlgTool constructor
      NearbyTrackParticleThinning( const std::string& type, const std::string& name,
				   const IInterface* parent );

      /// @name Function(s) implementing the @c IAlgTool interface
      /// @{

      /// Function initialising the tool
      StatusCode initialize() override;
      /// Function finalising the tool
      StatusCode finalize() override;

      /// @}

      /// @name Function(s) implementing the
      ///       @c DerivationFramework::IThinningTool interface
      /// @{

      /// Function performing the configured thinning operation
      StatusCode doThinning() const override;

      /// @}

   private:
      /// @name Function(s) implementing much of the logic
      /// @{

      StatusCode setTracksMask( std::vector< bool >& mask,
				const xAOD::IParticle* particle,
				const xAOD::TrackParticleContainer* tpc ) const;
      /// @}

      /// @name Tool properties
      /// @{

      /// SG key for the lepton container to use
      std::string m_sgKey;
      /// SG key of the calo cluster container to use
      std::string m_inDetSGKey = "InDetTrackParticles";
      /// Selection string to use with the expression evaluation
      std::string m_selectionString;
      /// Cone around lepton objects to keep clusters in
      float m_coneSize = -1.0;
      /// Flag for using @c IThinningSvc::Operator::And (instead of "or")
      bool m_and = false;

      /// @}

      /// Handle for accessing the thinning service
      ServiceHandle< IThinningSvc > m_thinningSvc;
      /// Variables keeping statistics information about the job
      mutable unsigned int m_ntot = 0, m_ntotTopo = 0, m_npass = 0,
                           m_npassTopo = 0;
      /// The expression evaluation helper object
      std::unique_ptr< ExpressionParserHelper > m_parser;

   }; // class NearbyTrackParticleThinning

} // namespace DerivationFramework

#endif // DERIVATIONFRAMEWORKSUSY_NEARBYTRACKPARTICLETHINNING_H
