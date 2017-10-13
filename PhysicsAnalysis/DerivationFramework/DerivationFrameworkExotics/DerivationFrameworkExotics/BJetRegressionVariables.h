/////////////////////////////////////////////////////////////////
// BJetRegressionVariables.h
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_BJetRegressionVariables_H
#define DERIVATIONFRAMEWORK_BJetRegressionVariables_H

/** @class BJetRegressionVariables
    @author Alexander Melzer (alexander.melzer@cern.ch)
*/

#include "AsgTools/ToolHandle.h"

#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"

#include "xAODJet/Jet.h"
#include "xAODJet/JetContainer.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"

#include <vector>
#include <string>


namespace DerivationFramework {
 
  class BJetRegressionVariables : public AthAlgTool, public IAugmentationTool {
  public:
    BJetRegressionVariables(const std::string& t, const std::string& n, const IInterface* p);

    ~BJetRegressionVariables();
     
    StatusCode initialize();
    StatusCode finalize();
    virtual StatusCode addBranches() const;
 
  private:
    std::string m_containerName; 
    std::string m_assocTracksName;
    std::vector<float> m_minTrackPt;

    struct TrackMomentStruct {float vecSumPtTrk; float scalSumPtTrk; };

    TrackMomentStruct getSumTrackPt( const float minTrackPt, const std::vector<const xAOD::TrackParticle*>& tracks) const;

    const std::string getMomentBaseName(const float minTrackPt) const;
  };
}

#endif // DERIVATIONFRAMEWORK_BJetRegressionVariables_H
