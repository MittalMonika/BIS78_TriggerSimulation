/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// #include <iostream>
// #include <sstream>
#include <map>
#include <mutex>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>
#include <TProfile2D.h>

#include <boost/tokenizer.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "AthenaMonitoring/GenericMonitoringTool.h"

using namespace std;

const InterfaceID& GenericMonitoringTool::interfaceID() {
	static InterfaceID GenericMonitoringTool_ID("GenericMonitoringTool", 1, 0);

	return GenericMonitoringTool_ID;
}

GenericMonitoringTool::GenericMonitoringTool(const std::string & type, const std::string & name, const IInterface* parent)
  : AthAlgTool(type, name, parent), m_histSvc("THistSvc", name) { 
  declareProperty("Histograms", m_histograms, "Definitions of histograms");
  declareInterface<GenericMonitoringTool>(this);
}

GenericMonitoringTool::~GenericMonitoringTool() { }

StatusCode GenericMonitoringTool::initialize() {
  // ATH_CHECK(setProperties());
  ATH_CHECK(m_histSvc.retrieve());

  // const INamedInterface* parentAlg = dynamic_cast<const INamedInterface*>(parent());

  // if (parentAlg == nullptr) {
  //   ATH_MSG_ERROR("Cannot retrieve INamedInterface of parent algorithm");
  //   return StatusCode::FAILURE;
  // }
  
  string parentAlgName = "TODO"; // parentAlg->name();

  m_histogramCategory["EXPERT"]   = new MonGroup(this, parentAlgName, expert);
  m_histogramCategory["SHIFT"]    = new MonGroup(this, parentAlgName, shift);
  m_histogramCategory["DEBUG"]    = new MonGroup(this, parentAlgName, debug);
  m_histogramCategory["RUNSTAT"]  = new MonGroup(this, parentAlgName, runstat);
  m_histogramCategory["EXPRESS"]  = new MonGroup(this, parentAlgName, express);

  for (const string& item : m_histograms) {
    HistogramDef def = parseJobOptHistogram(item);

    if ( ! def.ok || createFiller(def).isFailure() ) {
      ATH_MSG_DEBUG("Demand to monitor variable(s): " << def.name  << " can't be satisfied");
    }
  }

  if ( m_fillers.empty() ) {
    ATH_MSG_ERROR("No variables to be monitored, detach this tool, it will save time");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

vector<GenericMonitoringTool::HistogramFiller*> GenericMonitoringTool::getHistogramsFillers(vector<reference_wrapper<Monitored::IMonitoredVariable>> monitoredVariables) {
  vector<GenericMonitoringTool::HistogramFiller*> result;

  // for (auto filler : m_fillers) {
  //   auto fillerVariables = filler->histogramVariablesNames();
  //   vector<reference_wrapper<Monitored::IMonitoredVariable>> variables;

  //   for (auto fillerVariable : fillerVariables) {

  //   }
  // }
//TODO
  return result;
}

string GenericMonitoringTool::level2string(Level l) {
  string result = "/UNSPECIFIED_NONE_OF_EXPERT_DEBUG_SHIFT_EXPRESS_RUNSUM/";

  switch (l){
  case debug:
    result = "/DEBUG/";
    break;
  case expert:
    result = "/EXPERT/";
    break;
  case shift:
    result = "/SHIFT/";
    break;
  case express:
    result = "/EXPRESS/";
    break;
  case runsum:
    result = "/RUNSUM/";
    break;
  }

  return result;
}

void GenericMonitoringTool::setOpts(TH1* hist, const std::string& opt) {
  // try to apply an option
  if ( opt.find("kCanRebin") != std::string::npos ) {
     hist->SetCanExtend(TH1::kAllAxes);
  } else {
     hist->SetCanExtend(TH1::kNoAxis);  
  }
  // try to apply option to make Sumw2 in histogram
  if ( opt.find("Sumw2") != std::string::npos ) {
    hist->Sumw2();
  }
}

void GenericMonitoringTool::setLabels(TH1* hist, const std::vector<std::string>& labels) {
  if (labels.empty()){
    return;
  }
  for ( int i = 0; i < std::min( (int)labels.size(), (int)hist->GetNbinsX() ); ++i ) {
    int bin = i+1;
    hist->GetXaxis()->SetBinLabel(bin, labels[i].c_str());
    ATH_MSG_DEBUG("setting label X" <<  labels[i] << " for bin " << bin);
  }

  for ( int i = (int)hist->GetNbinsX(); i < std::min( (int)labels.size(), (int)hist->GetNbinsX()+(int)hist->GetNbinsY() ); ++i ) {
    int bin = i+1-(int)hist->GetNbinsX();
    hist->GetYaxis()->SetBinLabel(bin, labels[i].c_str());
    ATH_MSG_DEBUG("setting label Y" <<  labels[i] << " for bin " << bin);
  }
}

/**
 * Create and register histogram
 * 
 * If histogram already exists under that name, re-use it
 * H      : actual type of histogram to be created
 * HBASE  : histogram base class (TH[1,2,3])
 * Args   : remaining arguments to TH constructor (except name, title)
 */
template<class H, class HBASE, typename... Types> 
HBASE* GenericMonitoringTool::create(const HistogramDef& def, Types&&... hargs) {
  // Check if histogram exists already
  HBASE* histo = m_histogramCategory[def.path]->template getHist<HBASE>(def.alias);

  if (histo) {
    ATH_MSG_DEBUG("Histogram " << def.alias << " already exists. Re-using it.");
    return histo;
  }

  // Create the histogram and register it
  H* h = new H(def.alias.c_str(), def.title.c_str(), std::forward<Types>(hargs)...);
  StatusCode sc = m_histogramCategory[def.path]->regHist(static_cast<TH1*>(h));

  if (sc.isFailure()) {
    delete h;
    h = nullptr;
  }
  
  return h;
}

template<class H> 
TH1* GenericMonitoringTool::create1D( TH1*& histo, const HistogramDef& def ) {
  histo = create<H,TH1>(def, def.xbins, def.xmin, def.xmax);
  return histo;
}

template<class H> 
TH1* GenericMonitoringTool::createProfile( TProfile*& histo, const HistogramDef& def ) {
  TH1* h = create<H,TH1>(def, def.xbins, def.xmin, def.xmax, def.ymin, def.ymax);
  histo = dynamic_cast<TProfile*>(h);
  return h;
}

template<class H> 
TH1* GenericMonitoringTool::create2D( TH2*& histo, const HistogramDef& def ) {
  histo = create<H,TH2>(def, def.xbins, def.xmin, def.xmax, def.ybins, def.ymin, def.ymax);
  return histo;
}

template<class H> 
TH1* GenericMonitoringTool::create2DProfile( TProfile2D*& histo, const HistogramDef& def ) {
  TH1* h = create<H,TH2>(def, def.xbins, def.xmin, def.xmax, 
                         def.ybins, def.ymin, def.ymax, def.zmin, def.zmax);
  histo = dynamic_cast<TProfile2D*>(h);
  return histo;
}

StatusCode GenericMonitoringTool::createFiller(const HistogramDef& def) {
  TH1* histo(0);
  TH1* histo1D(0);
  TProfile* histoProfile(0);
  TH2* histo2D(0);
  TProfile2D* histo2DProfile(0);

  // create 1D
  if (def.type == "TH1F") 
    histo = create1D<TH1F>(histo1D, def);

  else if (def.type == "TH1D") 
    histo = create1D<TH1D>(histo1D, def);
  
  else if (def.type == "TH1I") 
    histo = create1D<TH1I>(histo1D, def);

  else if (def.type == "TProfile")
    histo = createProfile<TProfile>(histoProfile, def);
  
  //create 2D
  else if (def.type == "TH2F")
    histo = create2D<TH2F>(histo2D, def);

  else if (def.type == "TH2D")
    histo = create2D<TH2D>(histo2D, def);

  else if (def.type == "TH2I") 
    histo = create2D<TH2I>(histo2D, def);
  
  else if (def.type == "TProfile2D")
    histo = create2DProfile<TProfile2D>(histo2DProfile, def);
  
  if (histo == 0 ) {
    ATH_MSG_WARNING("Can not create yet histogram of type: " << def.type);
    ATH_MSG_WARNING("Try one of: TH1[F,D,I], TH2[F,D,I], TProfile, TProfile2D");
    return StatusCode::FAILURE;
  }

  setLabels(histo, def.labels);
  setOpts(histo, def.opt);

  std::string m_parentName = "TODO";

  if ( histo1D && !histoProfile){
    if ( def.opt.find("kCumulative") != std::string::npos ) {
      ATH_MSG_DEBUG("Variable: " << def.name[0] << " from parent algorithm: "
                    << m_parentName << " will be histogrammed in Cummulative histogram");

      m_fillers.push_back(new CumulativeHistogramFiller1D(histo1D, def));
    } else if (def.opt.find("kVecUO") != std::string::npos) {
      ATH_MSG_DEBUG("Variable: " << def.name << " from parent algorithm: "
                    << m_parentName << " will be added to histogram");
      // if (g && (unsigned(histo1D->GetNbinsX()+2) != g->size())) {
      //   ATH_MSG_WARNING("Variable: " << def.name << " from parent algorithm: " << m_parentName 
      //                   << " has different dimension: " << g->size() 
      //                   << " than histogram: " << histo1D->GetNbinsX()
      //                   << " booked for it and kVecUO options is requested (variable has to accomodate Under/Overflows too)");
      // }

      m_fillers.push_back(new VecHistogramFiller1DWithOverflows(histo1D, def));
    } else if (def.opt.find("kVec") != std::string::npos) {
      ATH_MSG_DEBUG("Variable: " << def.name << " from parent algorithm: "
                    << m_parentName << " will be added to histogram");
      // if (g && (unsigned(histo1D->GetNbinsX()) != varIt->second->size())) {
      //   ATH_MSG_WARNING("Variable: " << def.name << " from parent algorithm: " << m_parentName 
      //                   << " has different dimension: " << g->size() 
      //                   << " than histogram: " << histo1D->GetNbinsX()
      //                   << " booked for it and kVec options is requested");
      // }

      m_fillers.push_back(new VecHistogramFiller1D(histo1D, def));
    } else {
      ATH_MSG_DEBUG("Variable: " << def.name << " from parent algorithm: " << m_parentName
                    <<  " will be histogrammed");

      m_fillers.push_back(new HistogramFiller1D(histo1D, def));
    }
  } else if ( histo2DProfile ) {
      ATH_MSG_DEBUG("Variables: " << def.name[0] << "," << def.name[1] << "," << def.name[2]
                    << " from parent algorithm: " << m_parentName
                    << " will be histogrammed in 2D Profile histogram");

      m_fillers.push_back(new HistogramFiller2DProfile(histo2DProfile, def));
  } else if ( histo2D || histoProfile ){
      ATH_MSG_DEBUG("Variables: " << def.name[0] << "," << def.name[1]
                    << " from parent algorithm: " << m_parentName
                    << " will be histogrammed in 2D histogram");

      if (histo2D) {
        m_fillers.push_back(new HistogramFiller2D(histo2D, def));
      } else {       
        m_fillers.push_back(new HistogramFillerProfile(histoProfile, def));
      }
  }
  
  return StatusCode::SUCCESS;
}

const GenericMonitoringTool::HistogramDef GenericMonitoringTool::parseJobOptHistogram(const std::string& histDef) {
  /* Parse histogram defintion
     Example:
     1D: "EXPERT, TH1I, Name, Title;Alias, nBins, xmin, xmax, BinLabel1:BinLabel2:BinLabel3, kCumulative"
  */

  ATH_MSG_DEBUG("parseJobOptHistogram(\"" << histDef << "\")");
  
  // convert histogram definition string to an array of strings
  list<string> histProperty;
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer_t;
  boost::char_separator<char> sep(","); 
  tokenizer_t tokens(histDef, sep);
  for (tokenizer_t::iterator itr = tokens.begin(); itr != tokens.end(); ++itr) {
    string word = *itr;
    boost::trim(word);
    histProperty.push_back(word); 
  } 
  // return value
  HistogramDef histPar;
  list<string>::iterator itr = histProperty.begin();
  
  // stream
  string word = (*itr);
  boost::to_upper(word);
  if (word == "EXPERT" || word == "SHIFT" || word == "DEBUG" || word == "RUNSTAT" ) {
    histPar.path = word;
    itr = histProperty.erase(itr);
  }
  else
    histPar.path = "EXPERT";
  
  const char* warning = " NOT booked: ";
  histPar.ok   = false;
  histPar.ycut = false;
  histPar.zcut = false;
  
  if (histProperty.size() < 5) return histPar;   
  
  histPar.type = (*itr);
  itr = histProperty.erase(itr);

  histPar.name.push_back(*itr);
  itr = histProperty.erase(itr);
  if (histPar.type.find("TH2") == 0 || histPar.type == "TProfile") {
    histPar.name.push_back(*itr);
    itr = histProperty.erase(itr);
  }
  else if (histPar.type == "TProfile2D") {
    //twice more as there are three variables
    histPar.name.push_back(*itr);
    itr = histProperty.erase(itr);
    histPar.name.push_back(*itr);
    itr = histProperty.erase(itr);
  }

  // find if names contain alias 
  string::size_type semicolon_pos = histPar.name.back().find(';');
  if (  semicolon_pos != string::npos ) { // remove what is after the ; and set alias
    string actual_name = histPar.name.back().substr(0, semicolon_pos);    
    histPar.alias = histPar.name.back().substr(semicolon_pos+1);
    histPar.name.back() = actual_name;
  } else {
    histPar.alias = histPar.name[0];
    if ( histPar.name.size() == 2 ) {
      histPar.alias += "_vs_"+histPar.name[1];
    }
    else if (histPar.name.size() == 3 ){
      histPar.alias += "_vs_"+histPar.name[1]+"_vs_"+histPar.name[2];
    }
  }

  histPar.title = (*itr);
  itr = histProperty.erase(itr);
  
  if (histProperty.size() < 2) {
    ATH_MSG_WARNING(histPar.alias << warning << "NOT enough parameters for defining 1-D histogram");
    return histPar;   
  }
  
  try {
    histPar.xbins = boost::lexical_cast<int>(*itr);
    itr = histProperty.erase(itr);
  }
  catch (boost::bad_lexical_cast&) {
    ATH_MSG_WARNING(histPar.alias << warning << "int expected for xbins while got"  << histProperty);
    return histPar;
  }
  
  if (histProperty.size() < 2) {
    ATH_MSG_WARNING(histPar.name[0] << warning << "xmin and xmax expected");
    return histPar;
  }
  
  try {
    histPar.xmin = boost::lexical_cast<double>(*itr);
    itr = histProperty.erase(itr);
  }
  catch (boost::bad_lexical_cast&) {
    ATH_MSG_WARNING(histPar.name[0] << warning << "double expected for xmin");
    return histPar;
  }
  
  try {
    histPar.xmax = boost::lexical_cast<double>(*itr);
    itr = histProperty.erase(itr);
  }
  catch (boost::bad_lexical_cast&) {
    ATH_MSG_WARNING(histPar.name[0] << warning << "double expected for xmax");
    return histPar;
  }

  if (histPar.type.find("TH2") == 0) {
    if (histProperty.size() < 2) {
      ATH_MSG_WARNING(histPar.name[0] << warning << "y-axis definition expected for TH2");
      return histPar;
    }
    
    try {
      histPar.ybins = boost::lexical_cast<int>(*itr);
      itr = histProperty.erase(itr);
    }
    catch (boost::bad_lexical_cast&) {
      ATH_MSG_WARNING(histPar.name[0] << warning << "int expected for ybins");
      return histPar;
    }
    
    if (histProperty.size() < 2) {
      ATH_MSG_WARNING(histPar.name[0] << warning << "ymin and ymax expected");
      return histPar;
    }
    
    try {
      histPar.ymin = boost::lexical_cast<double>(*itr);
      itr = histProperty.erase(itr);
    }
    catch (boost::bad_lexical_cast&) {
      ATH_MSG_WARNING(histPar.name[0] << warning << "double expected for ymin");
      return histPar;
    }
    
    try {
      histPar.ymax = boost::lexical_cast<double>(*itr);
      itr = histProperty.erase(itr);
    }
    catch (boost::bad_lexical_cast&) {
      ATH_MSG_WARNING(histPar.name[0] << warning << "double expected for ymax");
      return histPar;
    }
  } //-end of TH2
  //TProfile
  else if (histPar.type == "TProfile" && histProperty.size() >= 2) {
    // limited y-range
    try {
      histPar.ymin = boost::lexical_cast<double>(*itr);
      itr = histProperty.erase(itr);
    }
    catch (boost::bad_lexical_cast&) {
      ATH_MSG_WARNING(histPar.name[0] << warning << "double expected for ymin of TProfile");
      return histPar;
    }
    
    try {
      histPar.ymax = boost::lexical_cast<double>(*itr);
      itr = histProperty.erase(itr);
    }
    catch (boost::bad_lexical_cast&) {
      ATH_MSG_WARNING(histPar.name[0] << warning << "double expected for ymax of TProfile");
      return histPar;
    }
    histPar.ybins = 0; // not used
    histPar.ycut = true;
  }
  //TProfile2D
  else if (histPar.type == "TProfile2D"){

    if (histProperty.size() < 2) {
      ATH_MSG_WARNING(histPar.name[0] << warning << "y-axis definition expected for TProfile2D");
      return histPar;
    }
    
    try {
      histPar.ybins = boost::lexical_cast<int>(*itr);
      itr = histProperty.erase(itr);
    }
    catch (boost::bad_lexical_cast&) {
      ATH_MSG_WARNING(histPar.name[0] << warning << "int expected for ybins");
      return histPar;
    }
    
    if (histProperty.size() < 2) {
      ATH_MSG_WARNING(histPar.name[0] << warning << "ymin and ymax expected");
      return histPar;
    }
    
    try {
      histPar.ymin = boost::lexical_cast<double>(*itr);
      itr = histProperty.erase(itr);
    }
    catch (boost::bad_lexical_cast&) {
      ATH_MSG_WARNING(histPar.name[0] << warning << "double expected for ymin");
      return histPar;
    }
    
    try {
      histPar.ymax = boost::lexical_cast<double>(*itr);
      itr = histProperty.erase(itr);
    }
    catch (boost::bad_lexical_cast&) {
      ATH_MSG_WARNING(histPar.name[0] << warning << "double expected for ymax");
      return histPar;
    }
    //For limited z range
    if(histProperty.size() >= 2){
      try {
        histPar.zmin = boost::lexical_cast<double>(*itr);
        itr = histProperty.erase(itr);
      }
      catch (boost::bad_lexical_cast&) {
        ATH_MSG_WARNING(histPar.name[0] << warning << "double expected for zmin of TProfile2D");
        return histPar;
      }
    
      try {
        histPar.zmax = boost::lexical_cast<double>(*itr);
        itr = histProperty.erase(itr);
      }
      catch (boost::bad_lexical_cast&) {
        ATH_MSG_WARNING(histPar.name[0] << warning << "double expected for zmax of TProfile2D");
        return histPar;
      }

      histPar.zcut = true;
    }
  }

  if (itr->find(":") != std::string::npos ) { // it means that last paramater has format str1:str2:str3:str4 which means this are bins labels
    // breate it 
    boost::char_separator<char> colon(":");
    tokenizer_t labels(*itr, colon);
    for ( tokenizer_t::iterator l = labels.begin(); l != labels.end(); ++l ) {
      histPar.labels.push_back(*l);
    }
    itr = histProperty.erase(itr);
  }
  
  // opt
  if (itr != histProperty.end()) {
    histPar.opt = (*itr);
  }
    
  histPar.ok = true;
  return histPar;
}

/////////////////////////////////////////////////////////////////////////////
// fillers
/////////////////////////////////////////////////////////////////////////////

unsigned GenericMonitoringTool::HistogramFiller1D::fill() {
  if (m_monVariables.size() != 1) {
    return 0;
  }

  unsigned i(0);
  auto valuesVector = m_monVariables[0].get().getVectorRepresentation();
  std::lock_guard<std::mutex> lock(*(this->m_mutex));

  for (auto value : valuesVector) {
    histogram()->Fill(value);
    ++i;
  }

  return i;
} 

unsigned GenericMonitoringTool::CumulativeHistogramFiller1D::fill() {
  if (m_monVariables.size() != 1) {
    return 0;
  }

  unsigned i(0);
  auto hist = histogram();
  auto valuesVector = m_monVariables[0].get().getVectorRepresentation();
  std::lock_guard<std::mutex> lock(*(this->m_mutex));

  for (auto value : valuesVector) {
    unsigned bin = hist->FindBin(value);

    for (unsigned j = bin; j > 0; --j) {
      hist->AddBinContent(j);
    }

    ++i;
  }

  return i;  
}

unsigned GenericMonitoringTool::VecHistogramFiller1D::fill() {
  if (m_monVariables.size() != 1) {
    return 0;
  }

  unsigned i(0);
  auto hist = histogram();
  auto valuesVector = m_monVariables[0].get().getVectorRepresentation();
  std::lock_guard<std::mutex> lock(*(this->m_mutex));

  for (auto value : valuesVector) {
    hist->AddBinContent(i+1, value);
    hist->SetEntries(hist->GetEntries() + value);

    ++i;
  }

  return i;
}

unsigned GenericMonitoringTool::VecHistogramFiller1DWithOverflows::fill() {
  if (m_monVariables.size() != 1) {
    return 0;
  }

  unsigned i(0);
  auto hist = histogram();
  auto valuesVector = m_monVariables[0].get().getVectorRepresentation();
  std::lock_guard<std::mutex> lock(*(this->m_mutex));

  for (auto value : valuesVector) {
    hist->AddBinContent(i, value);
    hist->SetEntries(hist->GetEntries() + value);

    ++i;
  }

  return i;
}

unsigned GenericMonitoringTool::HistogramFillerProfile::fill() {
  if (m_monVariables.size() != 2) {
    return 0;
  }

  unsigned i(0);
  auto hist = histogram();
  auto valuesVector1 = m_monVariables[0].get().getVectorRepresentation();
  auto valuesVector2 = m_monVariables[1].get().getVectorRepresentation();
  std::lock_guard<std::mutex> lock(*(this->m_mutex));

  if (valuesVector1.size() != valuesVector2.size()) {
    if (valuesVector1.size() == 1) {
      // first variable is scalar -- loop over second
      for (auto value2 : valuesVector2) {
        hist->Fill(valuesVector1[0], value2);
        ++i;
      }
    } else if (valuesVector2.size() == 1)  {
      // second varaible is scalar -- loop over first
      for (auto value1 : valuesVector1) {
        hist->Fill(value1, valuesVector2[0]); 
        ++i;
      } 
    }
  } else {
    for (i = 0; i < valuesVector1.size(); ++i) {
      hist->Fill(valuesVector1[i], valuesVector2[i]);
    }
  }
  
  return i;
}

unsigned GenericMonitoringTool::HistogramFiller2DProfile::fill() {
  if (m_monVariables.size() != 3) {
    return 0;
  }

  unsigned i(0);
  auto hist = histogram();
  auto valuesVector1 = m_monVariables[0].get().getVectorRepresentation();
  auto valuesVector2 = m_monVariables[1].get().getVectorRepresentation();
  auto valuesVector3 = m_monVariables[2].get().getVectorRepresentation();
  std::lock_guard<std::mutex> lock(*(this->m_mutex));
  /*HERE NEED TO INCLUDE CASE IN WHICH SOME VARIABLES ARE SCALAR AND SOME VARIABLES ARE VECTORS
  unsigned i(0);
  if (m_variable1->size() != m_variable2->size() || m_variable1->size() != m_variable3->size() || m_variable2->size() != m_variable3->size() ) {

  }*/

  //For now lets just consider the case in which all variables are of the same length
  for (i = 0; i < valuesVector1.size(); ++i) {
    hist->Fill(valuesVector1[i], valuesVector2[i], valuesVector3[i]);
  }
  
  return i;
}

unsigned GenericMonitoringTool::HistogramFiller2D::fill() {
  if (m_monVariables.size() != 2) {
    return 0;
  }

  unsigned i(0);
  auto hist = histogram();
  auto valuesVector1 = m_monVariables[0].get().getVectorRepresentation();
  auto valuesVector2 = m_monVariables[1].get().getVectorRepresentation();
  std::lock_guard<std::mutex> lock(*(this->m_mutex));

  if (valuesVector1.size() != valuesVector2.size()) {
    if (valuesVector1.size() == 1) {
      // first variable is scalar -- loop over second
      for (auto value2 : valuesVector2) {
        hist->Fill(valuesVector1[0], value2);
        ++i;
      }
    } else if (valuesVector2.size() == 1)  {
      // second varaible is scalar -- loop over first
      for (auto value1 : valuesVector1) {
        hist->Fill(value1, valuesVector2[0]); 
        ++i;
      } 
    }
  } else {
    for (i = 0; i < valuesVector1.size(); ++i) {
      hist->Fill(valuesVector1[i], valuesVector2[i]);
    }
  }
  
  return i;
}

/////////////////////////////////////////////////////////////////////////
// code form monitoring group
/////////////////////////////////////////////////////////////////////////
GenericMonitoringTool::MonGroup::MonGroup(GenericMonitoringTool* tool, const std::string& algo, Level l) 
  : m_tool(tool), m_algo(algo), m_level(l) {}

StatusCode GenericMonitoringTool::MonGroup::regHist(TH1* h) {
  return m_tool->m_histSvc->regHist(m_tool->level2string(m_level)+m_algo+"/"+h->GetName(), h);
}

StatusCode GenericMonitoringTool::MonGroup::deregHist(TH1* h) {
  return m_tool->m_histSvc->deReg(h);
}
