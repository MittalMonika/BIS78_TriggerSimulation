/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/**
 * ----------------------------------------------------------------
 * The default behavior of this tool is to use beta = 1.0, but multiple
 * values of beta can be used simultaneously. The property BetaList
 * should be passed a list of floats. Values of < 0 or > 10 may result
 * in problematic output variable names and all values will be rounded
 * to the nearest 0.1. No suffix will be added to the outputs for beta = 1.0
 * and for other values a suffix of _BetaN will be added where N = int(10*beta).
 *
 * The DoDichroic option adds dichroic energy correlator ratios described
 * on page 120 in https://arxiv.org/abs/1803.07977
 * ----------------------------------------------------------------
 */

#ifndef jetsubstructuremomenttools_energycorrelatorgeneralizedratiostool_header
#define jetsubstructuremomenttools_energycorrelatorgeneralizedratiostool_header

#include "JetSubStructureMomentTools/JetSubStructureMomentToolsBase.h"
#include "JetSubStructureMomentTools/ECFHelper.h"

class EnergyCorrelatorGeneralizedRatiosTool :
  public JetSubStructureMomentToolsBase {
    ASG_TOOL_CLASS(EnergyCorrelatorGeneralizedRatiosTool, IJetModifier)
    
    public:
      // Constructor and destructor
      EnergyCorrelatorGeneralizedRatiosTool(std::string name);

      StatusCode initialize();

      int modifyJet(xAOD::Jet &jet) const;

    private:
      bool m_doN3;
      bool m_doLSeries;
      std::vector<float> m_rawBetaVals; /// Vector of input values before cleaning
      std::vector<float> m_betaVals; /// Local vector for cleaned up inputs
      bool m_doDichroic;

};

#endif
