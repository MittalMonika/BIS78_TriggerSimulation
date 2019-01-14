/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/
#include <algorithm>
#include <set>
#include "AthenaMonitoring/HistogramFiller.h"

using namespace Monitored;

HistogramFillerFactory::HistogramFillerFactory(const ServiceHandle<ITHistSvc>& histSvc,
                                               std::string groupName)
    : m_histSvc(histSvc), m_groupName(std::move(groupName)) { 
}
    
HistogramFillerFactory::~HistogramFillerFactory() {
}

HistogramFiller* HistogramFillerFactory::create(const HistogramDef& def) {
  TH1* histo(0);
  TH1* histo1D(0);
  TProfile* histoProfile(0);
  TH2* histo2D(0);
  TProfile2D* histo2DProfile(0);

  if (def.type == "TH1F") {
    histo1D = histo = create1D<TH1F>(def);
  } else if (def.type == "TH1D") {
    histo1D = histo = create1D<TH1D>(def);
  } else if (def.type == "TH1I") {
    histo1D = histo = create1D<TH1I>(def);
  } else if (def.type == "TProfile") {
    histo = create1DProfile<TProfile>(def);
    histoProfile = dynamic_cast<TProfile*>(histo);
  } else if (def.type == "TH2F") {
    histo = create2D<TH2F>(def);
    histo2D = dynamic_cast<TH2*>(histo);
  } else if (def.type == "TH2D") {
    histo = create2D<TH2D>(def);
    histo2D = dynamic_cast<TH2*>(histo);
  } else if (def.type == "TH2I") {
    histo = create2D<TH2I>(def);
    histo2D = dynamic_cast<TH2*>(histo);
  } else if (def.type == "TProfile2D") {
    histo = create2DProfile<TProfile2D>(def);
    histo2DProfile = dynamic_cast<TProfile2D*>(histo);
  }
  
  if (histo == 0) {
    throw HistogramFillerCreateException("Can not create yet histogram of type: >" + def.type + "<\n" +
                                         "Try one of: TH1[F,D,I], TH2[F,D,I], TProfile, TProfile2D");
  }
  histo->GetYaxis()->SetTitleOffset( 1.25 );// magic shift to make histograms readable even if no post-procesing is done

  setLabels(histo, def.labels);
  setOpts(histo, def.opt);

  HistogramFiller* result(0);

  if ( histo1D ){
    if ( def.opt.find("kCumulative") != std::string::npos ) {
      result = new CumulativeHistogramFiller1D(histo1D, def);
    } else if (def.opt.find("kVecUO") != std::string::npos) {
      result = new VecHistogramFiller1DWithOverflows(histo1D, def);
    } else if (def.opt.find("kVec") != std::string::npos) {
      result = new VecHistogramFiller1D(histo1D, def);
    } else {
      result = new HistogramFiller1D(histo1D, def);
    }
  } else if ( histoProfile ){
      result = new HistogramFillerProfile(histoProfile, def);
  } else if ( histo2DProfile ) {
      result = new HistogramFiller2DProfile(histo2DProfile, def);
  } else if ( histo2D ) {
      result = new HistogramFiller2D(histo2D, def);
  }
  
  return result;
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
HBASE* HistogramFillerFactory::create(const HistogramDef& def, Types&&... hargs) {    
  // invent path name
  // if def path contains any of: EXPERT, SHIFT, DEBUG, RUNSTAT, EXPRES this is online convention
  // this becomes the first element of the path followed by the group name
  // else if the def.path is DEFAULT then only the group name is used
  // if the path jet different is is concatenated with the group name
  //
  const static std::set<std::string> online( { "EXPERT", "SHIFT", "DEBUG", "RUNSTAT", "EXPRES" } );
  std::string path;
  if( online.count( def.path) != 0 )
      path =  "/" + def.path + "/" + m_groupName;
  else if ( def.path == "DEFAULT" )
    path = "/" + m_groupName;
  else
    path = "/" + m_groupName + "/"+def.path; 
  // remove duplicate //
  std::string fullName = path + "/" + def.alias;
  fullName.erase( std::unique( fullName.begin(), fullName.end(), 
			       [](const char a, const char b){ 
				 return a == b and a == '/'; 
			       } ), fullName.end() );
   
  // Check if histogram exists already
  HBASE* histo = nullptr;
  if ( m_histSvc->exists( fullName ) ) {
    if ( !m_histSvc->getHist( fullName, histo ) ) {
      throw HistogramFillerCreateException("Histogram >"+ fullName + "< seems to exist but can not be obtained from THistSvc");
    }    
    return histo;
  }

  // Create the histogram and register it
  H* h = new H(def.alias.c_str(), def.title.c_str(), std::forward<Types>(hargs)...);
  if ( !m_histSvc->regHist( fullName, static_cast<TH1*>(h) ) ) {
    delete h;
    throw HistogramFillerCreateException("Histogram >"+ fullName + "< can not be registered in THistSvc");

  }
  
  return h;
}

template<class H> 
TH1* HistogramFillerFactory::create1D(const HistogramDef& def) {
  return create<H,TH1>(def, def.xbins, def.xmin, def.xmax);
}

template<class H> 
TH1* HistogramFillerFactory::create1DProfile(const HistogramDef& def) {
  return create<H,TH1>(def, def.xbins, def.xmin, def.xmax, def.ymin, def.ymax);
}

template<class H> 
TH1* HistogramFillerFactory::create2D(const HistogramDef& def) {
  return create<H,TH2>(def, def.xbins, def.xmin, def.xmax, def.ybins, def.ymin, def.ymax);
}

template<class H> 
TH1* HistogramFillerFactory::create2DProfile(const HistogramDef& def) {
  return create<H,TH2>(def, def.xbins, def.xmin, def.xmax, 
                            def.ybins, def.ymin, def.ymax, def.zmin, def.zmax);
}

void HistogramFillerFactory::setOpts(TH1* hist, const string& opt) {
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

void HistogramFillerFactory::setLabels(TH1* hist, const std::vector<stdstring>& labels) {
  if (labels.empty()){
    return;
  }
  
  for ( int i = 0; i < std::min( (int)labels.size(), (int)hist->GetNbinsX() ); ++i ) {
    int bin = i+1;
    hist->GetXaxis()->SetBinLabel(bin, labels[i].c_str());
  }

  for ( int i = (int)hist->GetNbinsX(); i < std::min( (int)labels.size(), (int)hist->GetNbinsX()+(int)hist->GetNbinsY() ); ++i ) {
    int bin = i+1-(int)hist->GetNbinsX();
    hist->GetYaxis()->SetBinLabel(bin, labels[i].c_str());
  }
}


unsigned HistogramFiller1D::fill() {
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

unsigned CumulativeHistogramFiller1D::fill() {
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

unsigned VecHistogramFiller1D::fill() {
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

unsigned VecHistogramFiller1DWithOverflows::fill() {
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

unsigned HistogramFillerProfile::fill() {
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

unsigned HistogramFiller2DProfile::fill() {
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

unsigned HistogramFiller2D::fill() {
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
