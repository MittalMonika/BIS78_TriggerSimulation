#ifndef egammaValidation_ArtTest_H
#define egammaValidation_ArtTest_H

#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/Property.h"
#include "AthenaBaseComps/AthAlgorithm.h"

#include "ElectronPhotonSelectorTools/AsgElectronLikelihoodTool.h"

#include "TFile.h"
#include "TH1.h"

#include <string>

using namespace std;

class ArtTest : public AthAlgorithm
{
 public:

  // Output File
  TFile *myfile; //!

  // Histos
  
  // General Info
  TH1D *evtNmb; //!

  // Electron
  TH1D *pT_ElTrk_All; //!
  TH1D *pT_ElTrk_LLH; //! 
  TH1D *pT_ElTrk_MLH; //!
  TH1D *pT_ElTrk_TLH; //!

  ArtTest (const std::string& name, ISvcLocator* pSvcLocator);
  
  virtual StatusCode initialize ();
  virtual StatusCode beginInputFile();
  virtual StatusCode firstExecute();
  virtual StatusCode execute ();
  virtual StatusCode finalize ();

 private:

  AsgElectronLikelihoodTool* m_LooseLH;
  AsgElectronLikelihoodTool* m_MediumLH;
  AsgElectronLikelihoodTool* m_TightLH;

};

#endif
