/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "TopEventSelectionTools/JetFlavorPlots.h"

#include <cmath>
#include <array>
#include <string>

#include "TopEvent/EventTools.h"
#include "TopConfiguration/TopConfig.h"
#include "TopCorrections/ScaleFactorRetriever.h"

#include "TH1.h"
#include "TH2D.h"
#include "TFile.h"

// Systematic include(s):
#include "PATInterfaces/SystematicSet.h"

namespace top {

const double JetFlavorPlots::toGeV = 0.001;


JetFlavorPlots::JetFlavorPlots(const std::string& name,
                               TFile* outputFile, const std::string& params, std::shared_ptr<top::TopConfig> config, EL::Worker* wk) :
    m_nominalHashValue(0),
    m_detailed(false),
    m_doNominal(false),
    m_doRadHigh(false),
    m_doRadLow(false),
    m_isweightsread(false),
    m_w_Nominal_pos(-1),
    // default pT and eta binning, and default max number of Jets
    m_ptBins ("15:20:30:45:60:80:110:160:210:260:310:400:500:600:800:1000:1200:1500:1800:2500"),
    m_etaBins ("0.:0.3:0.8:1.2:2.1:2.8:3.6:4.5"),
    m_nJetsMax(15),
    m_jetCollection(""),
    m_config (config) {

    CP::SystematicSet nominal;
    m_nominalHashValue = nominal.hash();

    // retrieve jet collection and remove the "Jets" at the end of it
    m_jetCollection = m_config->sgKeyJets();
    m_jetCollection.erase(m_jetCollection.length() - 4);

    //decoding arguments
    std::istringstream stream(params);
    std::string s;
    while (stream>>s) {
      if (s=="detailed")
        m_detailed = true;
      else if (s.substr(0,3)=="pt:" || s.substr(0,3)=="pT:" )
        m_ptBins = s.substr(3,s.size()-3);
      else if (s.substr(0,7)=="abseta:" )
        m_etaBins = s.substr(7,s.size()-7);
      else if (s.substr(0,9)=="nJetsMax:" )
        m_nJetsMax = std::atoi(s.substr(9,s.size()-9).c_str());
      else if (s=="nominal" || s=="NOMINAL" || s=="Nominal")
        m_doNominal= true;
      else if (s=="radiationhigh" || s=="RADIATIONHIGH" || s=="RadiationHigh" || s=="radiationHigh")
        m_doRadHigh= true;
      else if (s=="radiationlow"  || s=="RADIATIONLOW"  || s=="RadiationLow"  || s=="radiationLow")
        m_doRadLow= true;
      else {
        std::cout << "ERROR: Can't understand argument " << s << " for JETFLAVORPLOTS." << std::endl;
        exit(1);
      }
    }
    //If neither nominal or radiation has been selected, assume it's nominal
    if((m_doNominal+m_doRadHigh+m_doRadLow)==false){
       m_doNominal = true;
       m_w_Nominal_pos = 0;
    }

    if((m_doRadHigh+m_doRadLow)&&(!m_config->doMCGeneratorWeights())){
       std::cout << "ERROR: You have requested Radiation Variation JETFLAVORPLOTS, but MCGeneratorWeights is set to False! Please enable it and run again!";
       exit(1);
    }
    // create the JetFlavorPlots and JetFlavorPlots_Loose directories only if needed
    if (m_config->doTightEvents()){
       if(m_doNominal) m_hists = std::make_shared<PlotManager>(name+"/JetFlavorPlots", outputFile, wk);
       if(m_doRadHigh) m_hists_RadHigh = std::make_shared<PlotManager>(name+"/JetFlavorPlots_RadHigh", outputFile, wk);
       if(m_doRadLow)  m_hists_RadLow  = std::make_shared<PlotManager>(name+"/JetFlavorPlots_RadLow", outputFile, wk);
    }
    if (m_config->doLooseEvents() && m_doNominal) m_hists_Loose = std::make_shared<PlotManager>(name+"/JetFlavorPlots_Loose", outputFile, wk);

    //handle binning
    std::vector<double> ptBinning;
    std::vector<double> etaBinning;
    formatBinning(m_ptBins, ptBinning);
    formatBinning(m_etaBins, etaBinning);
    std::cout << "INFO: Here is the binning used for JETFLAVORPLOTS in selection "<<name<<":" << std::endl;
    std::cout<<"pt: ";
    for (auto pt:ptBinning) std::cout<<pt<<" ";
    std::cout<<std::endl;
    std::cout<<"abseta: ";
    for (auto eta:etaBinning) std::cout<<eta<<" ";
    std::cout<<std::endl;

    if (m_config->doTightEvents()){
      if(m_doNominal) BookHistograms(m_hists, ptBinning, etaBinning);
      if(m_doRadHigh) BookHistograms(m_hists_RadHigh, ptBinning, etaBinning);
      if(m_doRadLow)  BookHistograms(m_hists_RadLow, ptBinning, etaBinning);
    }
    if (m_config->doLooseEvents()) BookHistograms(m_hists_Loose, ptBinning, etaBinning);
}


void JetFlavorPlots::BookHistograms(std::shared_ptr<PlotManager> h_ptr, std::vector<double> ptBins, std::vector<double> etaBins){
   if(m_detailed){
     for (std::string flavour:{"gluon","lightquark","cquark","bquark","other"}){
        // vs. nJets and flavour
        for (int i = 0; i <= m_nJetsMax; ++i){
           h_ptr->addHist(flavour + "_jets_njet" + std::to_string(i) + "_" + m_jetCollection,";pT [GeV];|#eta^{jet}|;Events",
                            ptBins.size()-1, ptBins.data(), etaBins.size()-1, etaBins.data());
          }
          // vs. flavour only
          h_ptr->addHist(flavour + "_jets_" + m_jetCollection,";pT [GeV];|#eta^{jet}|;Events",
                            ptBins.size()-1, ptBins.data(), etaBins.size()-1, etaBins.data());
      }
      // if "detailed", also making the quark_jets one
        h_ptr->addHist("quark_jets_" + m_jetCollection, ";pT [GeV];|#eta^{jet}|;Events",
                        ptBins.size()-1, ptBins.data(), etaBins.size()-1, etaBins.data());
    }
    else {
      // the simplest case, one for gluon, one for quarks
        h_ptr->addHist("quark_jets_" + m_jetCollection, ";pT [GeV];|#eta^{jet}|;Events",
                        ptBins.size()-1, ptBins.data(), etaBins.size()-1, etaBins.data());
        h_ptr->addHist("gluon_jets_" + m_jetCollection, ";pT [GeV];|#eta^{jet}|;Events",
                        ptBins.size()-1, ptBins.data(), etaBins.size()-1, etaBins.data());
    }
}

bool JetFlavorPlots::apply(const top::Event& event) const {
    //only MC
    if (!top::isSimulation(event)) return true;
    // only nominal
    if (event.m_hashValue != m_nominalHashValue) return true;
    // do loose or tight events only if requested
    if (event.m_isLoose && !m_config->doLooseEvents()) return true;
    if (!event.m_isLoose && !m_config->doTightEvents()) return true;

    //Only the first time the apply method is called, retrieve the weights positions. This is not done in constructor since the names are not known before reading the first event
    if((m_doRadHigh+m_doRadLow)&&(!m_isweightsread)){
       m_isweightsread=true;
       std::vector<std::string> w_names = m_config->getLHE3Names();
       for(unsigned int n=0; n<w_names.size(); ++n){
          if(w_names.at(n)=="nominal")       m_w_Nominal_pos = n;
          if(w_names.at(n)=="muR=05,muF=05") m_w_RadHigh_pos.push_back(n);
          if(w_names.at(n)=="muR=20,muF=20") m_w_RadLow_pos.push_back(n);
          if(w_names.at(n)=="Var3cUp")       m_w_RadHigh_pos.push_back(n);
          if(w_names.at(n)=="Var3cDown")     m_w_RadLow_pos.push_back(n);
       }
    }
    double eventWeight = 1.;
//       eventWeight = event.m_info->mcEventWeight();
//      eventWeight = event.m_truthEvent->at(0)->weights()[0];// FIXME temporary bugfix
    if(event.m_isLoose){
       eventWeight = event.m_truthEvent->at(0)->weights()[m_w_Nominal_pos];
       FillHistograms(m_hists_Loose, eventWeight, event);
    }
    else{
       if(m_doNominal){
          eventWeight = event.m_truthEvent->at(0)->weights()[m_w_Nominal_pos];
          FillHistograms(m_hists, eventWeight, event);
       }
       if(m_doRadHigh){
          eventWeight = event.m_truthEvent->at(0)->weights()[m_w_RadHigh_pos.at(0)] * event.m_truthEvent->at(0)->weights()[m_w_RadHigh_pos.at(1)] / event.m_truthEvent->at(0)->weights()[m_w_Nominal_pos];
          FillHistograms(m_hists_RadHigh, eventWeight, event);
       }
       if(m_doRadLow){
          eventWeight = event.m_truthEvent->at(0)->weights()[m_w_RadLow_pos.at(0)] * event.m_truthEvent->at(0)->weights()[m_w_RadLow_pos.at(1)] / event.m_truthEvent->at(0)->weights()[m_w_Nominal_pos];
          FillHistograms(m_hists_RadLow, eventWeight, event);
       }
    }

    return true;
}

void JetFlavorPlots::FillHistograms(std::shared_ptr<PlotManager> h_ptr, double w_event, const top::Event& event) const{
   bool throwWarning=false;
    for (const auto* const jetPtr : event.m_jets) {
      int jet_flavor = -99;
      bool status = jetPtr->getAttribute<int>("PartonTruthLabelID", jet_flavor);
      if (!status) continue;

      if(m_detailed){
        if((unsigned int) m_nJetsMax>=event.m_jets.size()){
          // vs. nJets and flavour
          char name[200];
          // PDG ID for gluon is 21
          if (jet_flavor == 21) sprintf(name,"gluon_jets_njet%lu_%s", event.m_jets.size(), m_jetCollection.c_str());
          // PDG ID for d,u,s is 1,2,3
          else if (jet_flavor >= 1 && jet_flavor <=3 ) sprintf(name,"lightquark_jets_njet%lu_%s", event.m_jets.size(), m_jetCollection.c_str());
          // PDG ID for c is 4
          else if (jet_flavor == 4) sprintf(name,"cquark_jets_njet%lu_%s", event.m_jets.size(), m_jetCollection.c_str());
          // PDG ID for b is 5
          else if (jet_flavor == 5) sprintf(name,"bquark_jets_njet%lu_%s",  event.m_jets.size(), m_jetCollection.c_str());
          else sprintf(name,"other_jets_njet%lu_%s",  event.m_jets.size(), m_jetCollection.c_str());
          
          static_cast<TH2D*>(h_ptr->hist(name))->Fill(jetPtr->pt()*toGeV,
                                                           jetPtr->eta(),
                                                                w_event);
          // if "detailed", also making the quark_jets one
          if (jet_flavor >= 1 && jet_flavor <=4 ) {
            sprintf(name,"quark_jets_%s", m_jetCollection.c_str());
            static_cast<TH2D*>(h_ptr->hist(name))->Fill(jetPtr->pt()*toGeV,
                                                                jetPtr->eta(),
                                                                w_event);
          }
        }
        else
          throwWarning=true;
        // vs. flavour only
        char shortname[200];
        // PDG ID for gluon is 21
        if (jet_flavor == 21) sprintf(shortname,"gluon_jets_%s", m_jetCollection.c_str());
        // PDG ID for d,u,s is 1,2,3
        else if (jet_flavor >= 1 && jet_flavor <=3 ) sprintf(shortname,"lightquark_jets_%s", m_jetCollection.c_str());
        // PDG ID for c is 4
        else if (jet_flavor == 4) sprintf(shortname,"cquark_jets_%s", m_jetCollection.c_str());
        // PDG ID for b is 5
        else if (jet_flavor == 5) sprintf(shortname,"bquark_jets_%s", m_jetCollection.c_str());
        else sprintf(shortname,"other_jets_%s", m_jetCollection.c_str());

        static_cast<TH2D*>(h_ptr->hist(shortname))->Fill(jetPtr->pt()*toGeV,
                                                              jetPtr->eta(),
                                                                   w_event);
      }
      else {
        // the simplest case, one for gluon, one for quarks
        char name[200];
        // PDG ID for gluon is 21
        if (jet_flavor == 21) sprintf(name,"gluon_jets_%s", m_jetCollection.c_str());
        // PDG ID for d,u,s is 1,2,3
        else if (jet_flavor >= 1 && jet_flavor <=4 ) sprintf(name,"quark_jets_%s", m_jetCollection.c_str());
        
        static_cast<TH2D*>(h_ptr->hist(name))->Fill(jetPtr->pt()*toGeV,
                                                         jetPtr->eta(),
                                                              w_event);
      }
    }

    if (throwWarning)
       std::cout<<"Warning! Number of jets in the event is "<<event.m_jets.size()<<", but histograms have been booked up to "<<m_nJetsMax<<". Exclusive histograms in number of jets have not been filled.\n";
}

// function to translate the binnings into vector of bin edges
void JetFlavorPlots::formatBinning(const std::string& str, std::vector<double>& binEdges ) {
  std::stringstream ss(str);
  std::string tok;
  char separator = ':';
  while(std::getline(ss, tok, separator)) {
    binEdges.push_back(std::atof(tok.c_str()));
  }
}

std::string JetFlavorPlots::name() const {
    return "JETFLAVORPLOTS";
}

}  // namespace top
