/*
  Copyright (C) 2020 CERN for the benefit of the ATLAS collaboration
*/

/**
*  ____________________________________________________________________________
*  @file postProcessIDPVMHistos.cxx
*  @author: Max Goblirsch
*  Based on code by Liza Mijovic, Soeren Prell
*
*  Goal: Update resolutions extracted from 2D histograms 
*        after merging several output files (typically after grid running) 
*   ____________________________________________________________________________
*/


#include <iostream>
#include <algorithm>
#include <utility>

#include "InDetPhysValMonitoring/ResolutionHelper.h"

#include <boost/algorithm/string.hpp>

#include "TFile.h"
#include "TSystem.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TProfile.h"
#include "TEfficiency.h"
#include "TMath.h"
#include "TROOT.h"
#include "TKey.h"
#include "TClass.h"
#include "TObject.h"
#include "TObjString.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"

using namespace std;


bool file_exists(const string p_name) {
  return (gSystem->AccessPathName(p_name.c_str(), kFileExists))?
    false : true;    
}

// check if the name of an object matches what we expect from a resolution helper 
bool isResolutionHelper(TObject* entry){
  const std::string objName{entry->GetName()}; 
  return ((objName.find("resHelper") == 0 || objName.find("pullHelper") == 0) && dynamic_cast<TH1*>(entry));
}

// get the x axis observable and resolution (y axis) from the name of a 2D histo. 
// Relies on the conventions within IDPVM
std::pair<std::string, std::string> getObservableAndReso(const TObject* resHelper){
    const std::string name{resHelper->GetName()};
    const std::string keyWord {"Helper"}; 
    const size_t offset = keyWord.size();
    auto start = name.find(keyWord)+offset;
    auto sep = name.find("_");
    return {name.substr(start, sep-start), name.substr(sep+1)}; 

}

// get the resolution type (pull or res) - taken from the prefix of the 2D histo name 
std::string getResoType(const TObject* resHelper){
    std::string aux{resHelper->GetName()};
    return aux.substr(0, aux.find("Helper")); 
}

// clone an existing histogram of a known name 
TH1* cloneExisting(const std::string & name){
    auto h = gDirectory->Get(name.c_str()); 
    if (!h){ 
        std::cerr << "Could not find existing histogram "<<name<<" - will not postprocess "<<std::endl; 
        return nullptr;
    }
    return dynamic_cast<TH1*>(h->Clone(name.c_str()));
}

// so far, we only support the 2D --> 1D case. 3D --> 2D is not used in IDPVM to date. 
int postProcessHistos(TObject* resHelper, IDPVM::ResolutionHelper & theHelper){
    // here we have to rely on the naming conventions of IDPVM to identify what we are looking at 
    auto vars = getObservableAndReso(resHelper); 
    auto type = getResoType(resHelper); 
    // cast to TH2
    TH2* resHelper2D = dynamic_cast<TH2*>(resHelper); 
    if (!resHelper2D){
        std::cerr <<"Unable to reduce the histogram "<<resHelper->GetName()<<" to a TH2 - this histo can not yet be postprocessed! " <<std::endl; 
        return 1; 
    }
    // get the corresponding 1D histos by cloning the existing ones in the same folder 
    TH1* h_width = cloneExisting(type + "width_vs_"+vars.first+"_"+vars.second); 
    TH1* h_mean = cloneExisting(type + "mean_vs_"+vars.first+"_"+vars.second); 
    // then call the resolution helper as done in "online" IDPVM
    theHelper.makeResolutions(resHelper2D, h_width, h_mean); 
    // update our 1D histos 
    h_width->Write();
    h_mean->Write();
    // and we are done
    return 0;
}

// recursively parse a directory tree, post-processing any histos seen along the way 
int postProcessDir(TDirectory* dir, IDPVM::ResolutionHelper & theHelper){

  int outcome = 0; 
  auto theCWD = gDirectory; 
  // walk through all keys in this directory 
  dir->cd();
  auto keys = dir->GetListOfKeys();
  for (const auto & key : *keys){
    TObject* gotIt = dir->Get(key->GetName()); 
    
    // if we encounter a directory, descend into it and repeat the process
    TDirectory* theDir = dynamic_cast<TDirectory*>(gotIt);
    if (theDir){
      outcome |= postProcessDir(theDir, theHelper); 
    }
    
    // if we encounter a histogram that could be a resolution input, post-process it 
    if (isResolutionHelper(gotIt)){
      outcome |= postProcessHistos(gotIt, theHelper); 
    }
  }
  theCWD->cd();
  return outcome;
}

//____________________________________________________________________________________
// function driving the postprocessing for this file
int pproc_file(const std::string & p_infile) {

    IDPVM::ResolutionHelper theHelper; 

    TFile* infile = TFile::Open(p_infile.c_str(),"UPDATE"); 
    if (!infile ) {
    std::cerr << "could not open input file "<<p_infile<<" for updating "<< std::endl;
    return 1;
    }

    int res = postProcessDir(gDirectory,theHelper); // recursively post-process the directory tree, starting from the root.  
    infile->Close();
    return res;
}
//____________________________________________________________________________________


//____________________________________________________________________________________
int main(int argc, char* argv[]) {
  if (argc !=2){
    std::cerr<<" Usage: postProcessIDPVMHistos <File to post-process>"<<std::endl;
    std::cerr<< "    where the file is typically obtained by hadding" << std::endl;
    std::cerr<< "    outputs of several independent IDPVM runs." << std::endl;
    return 1; 
  }
  const string infile {argv[1]};

  if (!file_exists(infile)) {
    std::cout << "Error: invalid input file: " << infile << std::endl;
    return 1;
  }
  std::cout << " Post-processing file " << infile << "\n" << std::endl;
  return pproc_file(infile);
}
//____________________________________________________________________________________      
