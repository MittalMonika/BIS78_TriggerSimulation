/**
 *  Author Lennart Adam <lennart.adam@cern.ch> July 2018
 *  
 *	Instances of this class create a map (TH1D) describing the electric field profile along the pixeldepth.
 *	E field maps based on TCAD simulations available only for limited number of values of fluence and bias voltage.
 *	Interpolate inbetween these simulations to save computing time. 
 * 	
 *	The TCAD simulations are performed for the FEI4 pixel sensor with a pixeldepth of 200um (IBL) or 250um (B layers)
 */

#ifndef PIXELDIGITIZATION_EFIELDINTERPOLATOR_H
#define PIXELDIGITIZATION_EFIELDINTERPOLATOR_H 

#include "AthenaBaseComps/AthAlgTool.h"
#include "TH1.h"

namespace RadDam{

// Different options to retrieve E field
// default: interspline
enum interpolationMethod{
    TCAD,               //use Precomputed TCAD map 
    interspline,        //use interpolation based on cubic splines
    interinvdist,       //use inverse distance weighted estimate
    linearField         //linear field according to bias voltage
};

class EfieldInterpolator: public AthAlgTool { 
public: 

  EfieldInterpolator( const std::string& type, const std::string& name,const IInterface* parent);
  virtual ~EfieldInterpolator(); 
  void SetLayer(int layer);
  //Recommended constructor
  StatusCode loadTCADlist(std::string TCADfileListToLoad );  
  //defFct

  virtual StatusCode  initialize();
  virtual StatusCode  finalize();  

  static const InterfaceID& interfaceID() ;

  // Member Functions
  TString loadTCADfiles(TString targetList = "");
  TString createInterpolationFromTCADtree(TString fTCAD);//TTree* tTCAD); 
  bool initializeFromFile(TString fpath);
  bool initializeFromDirectory(const std::string fpath);
  //void initInterpolationTree(TString interpolationTreesFile = "");	
  Double_t estimateEfield(std::vector<Double_t> vvol, std::vector<Double_t> vflu, std::vector<std::vector<Double_t>> vfluvvol, Double_t aimFlu, Double_t aimVol, TString prepend="", bool debug =false);
  Double_t estimateEfieldInvDistance(std::vector<Double_t> vvol, std::vector<Double_t> vflu, std::vector<std::vector<Double_t>> vfluvvol, Double_t aimFlu, Double_t aimVol, Double_t measure = 1.);
  
  TH1D* createEfieldProfile(Double_t aimFluence, Double_t aimVoltage);
  TH1D* getEfield(Double_t aimFluence, Double_t aimVoltage);
  TH1D* loadEfieldFromDat(TString fname, bool fillEdges =  true);
  void scaleIntegralTo(TH1* hin, Double_t aimInt, int first = 1, int last = -1);
  //TFile* debugFile = new TFile("/afs/cern.ch/user/l/ladam/Allpix/allpix/share/debug_interpolation.root", "RECREATE");
  //TString deIbugPrefix = "/afs/cern.ch/user/l/ladam/NegativeEfieldInterpolated/";
  void reliabilityCheck(Double_t aimFluence, std::vector<Double_t> fluences, Double_t aimVoltage, std::vector<Double_t> voltages);

private:  
    // Member variables	
  bool m_initialized;
  bool m_saveDocu   ;
  bool m_useSpline  ;  
  int m_sensorDepth ;          //um - default is IBL layer
  interpolationMethod m_efieldOrigin;
  TH1D* m_efieldProfile;            //Final efield profile
  TString m_fInter;  //= "InterpolationTTree.root";    //path to .root file for saving interpolation TTree, i.e. ordered by pixeldepth z
	//TFile* defFile; 	//File containing ttrees TCAD and Interpolation
  std::vector<std::vector<TString>> list_files(TString fileList_TCADsamples);
  Double_t extrapolateLinear(Double_t x1, Double_t y1, Double_t x2, Double_t y2, Double_t xaim );
  int fillXYvectors(std::vector<Double_t> vLoop,int ifix, std::vector<std::vector<Double_t>> v2vsv1, std::vector<Double_t> &xx, std::vector<Double_t> &yy, bool regularOrder = true);
  void fillEdgeValues(TH1D* hin);
  bool isInterpolation(const std::vector<Double_t> &vval, Double_t aimval) 
      { return ( vval.front() <= aimval && aimval <= vval.back() ); };
  bool isInterpolation(std::vector<Double_t>* vval, Double_t aimval)
      {return ( vval->front() <= aimval && aimval <= vval->back()   );};
  Double_t relativeDistance(Double_t x1, Double_t x2);                            //difference between x1 x2 scaled to x1
  Double_t relativeDistance(Double_t x1, Double_t y1, Double_t x2, Double_t y2);
  Double_t estimateEfieldLinear(Double_t aimVoltage);
  void saveTGraph(std::vector<Double_t> vvol, std::vector<Double_t> vflu, std::vector<std::vector<Double_t>> vfluvvol, Double_t aimFlu, Double_t aimVol, TString prepend, bool skipNegative = true);
}; 

}
#endif //> !PIXELDIGITIZATION_EFIELDINTERPOLATOR_H
