/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Dual-use tool interface for Rivet routine for classifying MC events according to the Higgs template cross section categories
 * Authors: Jim Lacey (Carleton University)
 * <james.lacey@cern.ch,jlacey@physics.carleton.ca>
 */

#ifndef TRUTHRIVETTOOLS_HIGGSTRUTHCATEGORYTOOL_H
#define TRUTHRIVETTOOLS_HIGGSTRUTHCATEGORYTOOL_H 1

#include "TLorentzVector.h"
#include "Rivet/AnalysisHandler.hh"
#include "TruthRivetTools/HiggsTemplateCrossSections.cc"

#include "AsgTools/AsgTool.h"
#include "TruthRivetTools/IHiggsTruthCategoryTool.h"


class HiggsTruthCategoryTool 
: public asg::AsgTool, 
  public virtual IHiggsTruthCategoryTool 
{ 
 public: 
   ASG_TOOL_CLASS( HiggsTruthCategoryTool , IHiggsTruthCategoryTool )
   HiggsTruthCategoryTool( const std::string& name );
   ~HiggsTruthCategoryTool() { };
 public:
   Rivet::AnalysisHandler *rivetAnaHandler; //!
   Rivet::HiggsTemplateCrossSections *higgsTemplateCrossSections; //!
   virtual StatusCode  initialize() override;
   StatusCode finalize () override;
   HTXS::HiggsClassification getHiggsTruthCategoryObject(const HepMC::GenEvent& HepMCEvent, const HTXS::HiggsProdMode prodMode) override;
 private:
   bool m_isInitialized;
   bool m_outHistos;
};

#endif //> !HIGGSTRUTHCLASSIFIER_HIGGSTRUTHCATEGORYTOOL_H
