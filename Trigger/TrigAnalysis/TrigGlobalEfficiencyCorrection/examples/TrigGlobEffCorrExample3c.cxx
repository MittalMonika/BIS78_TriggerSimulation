/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/// Based on CPToolTester.cxx (A. Kraznahorkay) 
/// and ut_ath_checkTrigger_test.cxx (W. Buttinger)
/// Contact: jmaurer@cern.ch
/*
 *    The set of examples 3a - 3e illustrates the use of lepton selection tags
 * for various scenarios:
 * 
 * - Example 3a: trigger = 2e12_lhloose_L12EM10VH, selecting events containing
 *               >=2 loose electrons, the leading-pT electron always satisfying
 *               in addition tight PID+isolation requirements.
 * 
 * - Example 3b: trigger = e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose
 *                         || 2e12_lhloose_L12EM10VH
 *               selecting events with >=2 loose electrons where the leading-pT
 *               electron also satisfies medium PID requirements.
 *               Only the latter is allowed to fire the single-electron trigger.
 * 
 * - Example 3c: trigger = e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose
 *                         || 2e12_lhloose_L12EM10VH
 *               selecting events with >=2 loose electrons. Any electron also
 *               satisfying medium PID requirements is allowed to fire the
 *               single-electron trigger.
 * 
 * - Example 3d: trigger = 2e17_lhvloose_nod0 || e7_lhmedium_nod0_mu24
 *       || e26_lhtight_nod0_ivarloose_OR_e60_lhmedium_nod0_OR_e140_lhloose_nod0
 *               same type of selection as example 3c but with 3 PID working
 *               points, using two distinct decorations.
 * 
 * - Example 3e: same scenario as example 3d, but using an alternative
 *               implementation that requires only one decoration.
 * 
 */
/*
 *    For this example (3c), any electron might be tagged as 'Signal' if it
 * passes MediumLH selection (here emulated with a random number). Electron tools
 * providing the efficiency/SF for the e24_xxx leg are configured with the
 * MediumLH working point. For the tools providing efficiency/SF for the e12_xxx
 * leg however, one SHOULD NOT set up one version for 'Medium'-tagged electrons
 * and another version for untagged electrons. This is because for the latter
 * one would need to configure the tools for a LooseLH-but-not-MediumLH working
 * point, which is not provided by egamma. Instead, we use a single version,
 * configured with the LooseLH working point, and instruct the TrigGlob tool to
 * use that version for both 'Medium'-tagged and untagged electrons. In this way,
 * no bias is introduced. ",
 *    The configuration of the TrigGlobalEfficiencyCorrectionTool is otherwise
 * essentially the same as for the example 3b, please refer to that example for
 * more details.
 *     
 */

// ROOT include(s):
#include <TFile.h>
#include <TError.h>

// Infrastructure include(s):
#ifdef XAOD_STANDALONE
    #include "xAODRootAccess/Init.h"
    #include "xAODRootAccess/TEvent.h"
    #include "xAODRootAccess/TStore.h"
#else
    #include "AthAnalysisBaseComps/AthAnalysisHelper.h"
    #include "POOLRootAccess/TEvent.h"
#endif

// EDM include(s):
#include "AsgTools/AnaToolHandle.h"
#include "EgammaAnalysisInterfaces/IAsgElectronEfficiencyCorrectionTool.h"
#include "MuonAnalysisInterfaces/IMuonTriggerScaleFactors.h"
#include "TriggerAnalysisInterfaces/ITrigGlobalEfficiencyCorrectionTool.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "PATCore/PATCoreEnums.h"

// stdlib include(s):
#include <sstream>
#include <random>
#include <vector>
#include <array>
using std::vector;
using std::string;

/// Helper function to split comma-delimited strings
namespace { vector<string> split_comma_delimited(const std::string&); }

#define MSGSOURCE "Example 3c"

int main(int argc, char* argv[])
{
    const char* filename = nullptr;
    bool debug = false, cmdline_error = false, toys = false;
    for(int i=1;i<argc;++i)
    {
        if(string(argv[i]) == "--debug") debug = true;
        else if(string(argv[i]) == "--toys") toys = true;
        else if(!filename && *argv[i]!='-') filename = argv[i];
        else cmdline_error = true;
    }
    if(!filename || cmdline_error)
    {
        Error(MSGSOURCE, "No file name received!");
        Error(MSGSOURCE, "  Usage: %s [--debug] [--toys] [DxAOD file name]", argv[0]);
        return 1;
    }
    #ifdef XAOD_STANDALONE
        xAOD::Init(MSGSOURCE).ignore();
        TFile* file = TFile::Open(filename, "READ");
        if(!file)
        {
            Error(MSGSOURCE, "Unable to open file!");
            return 2;
        }
        xAOD::TEvent event(xAOD::TEvent::kClassAccess);
        xAOD::TStore store;
        StatusCode::enableFailure();
    #else
       IAppMgrUI* app = POOL::Init();
       POOL::TEvent event(POOL::TEvent::kClassAccess);
       TString file(filename);
    #endif
    event.readFrom(file).ignore();
    Long64_t entries = event.getEntries();
    Info(MSGSOURCE, "Number of events in the file: %lli", entries);

    /* ********************************************************************** */
    
    Info(MSGSOURCE, "Configuring the electron CP tools");
    /// For property 'ElectronEfficiencyTools':
    ToolHandleArray<IAsgElectronEfficiencyCorrectionTool> electronEffTools;
    /// For property 'ElectronScaleFactorTools':
    ToolHandleArray<IAsgElectronEfficiencyCorrectionTool> electronSFTools; 
        /// For property 'ListOfLegsPerTool':
    std::map<std::string,std::string> legsPerTool;
    /// For property 'ListOfTagsPerTool':
    std::map<std::string,std::string> tagsPerTool;
    /// For property 'ElectronLegsPerTag':
    std::map<std::string,std::string> legsPerTag;
    /// To tag electron(s) as 'Signal'
    SG::AuxElement::Decorator<char> dec_signal("Signal");
    /// To emulate PID selection (90% loose-to-medium efficiency)
    std::bernoulli_distribution bernoulliPdf(0.9);

    /// RAII on-the-fly creation of electron CP tools:
    vector<asg::AnaToolHandle<IAsgElectronEfficiencyCorrectionTool>> factory;
    enum{ cLEGS, cTAG, cKEY, cPID, cISO };
    std::vector<std::array<std::string,5> > toolConfigs = {
        /// <list of legs>, <list of tags>, <key in map file>, <PID WP>, <iso WP>
        /// Single electron trigger: electrons tagged 'Signal'
        {"e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose", "Signal", "SINGLE_E_2015_e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose_2016_e26_lhtight_nod0_ivarloose_OR_e60_lhmedium_nod0_OR_e140_lhloose_nod0", "Medium", ""}, 
        /// Dielectron trigger: all electrons (tagged or not)
        {"e12_lhloose_L1EM10VH", "*,Signal", "DI_E_2015_e12_lhloose_L1EM10VH_2016_e17_lhvloose_nod0", "LooseBLayer", ""}
     };

    const char* mapPath = "ElectronEfficiencyCorrection/2015_2016/"
            "rel20.7/Moriond_February2017_v3/map1.txt";
    for(auto& cfg : toolConfigs) /// one instance per trigger leg x working point
    for(int j=0;j<2;++j) /// two instances: 0 -> MC efficiencies, 1 -> SFs
    {
        string name = "AsgElectronEfficiencyCorrectionTool/"
                + ((j? "ElTrigEff_" : "ElTrigSF_")
                + std::to_string(factory.size()/2));
        auto t = factory.emplace(factory.end(), name);
        t->setProperty("MapFilePath", mapPath).ignore();
        t->setProperty("TriggerKey", string(j?"":"Eff_") + cfg[cKEY]).ignore();
        t->setProperty("IdKey", cfg[cPID]).ignore();
        t->setProperty("IsoKey", cfg[cISO]).ignore();

        t->setProperty("CorrelationModel", "TOTAL").ignore();
        t->setProperty("ForceDataType", (int)PATCore::ParticleDataType::Full).ignore();
        if(t->initialize() != StatusCode::SUCCESS)
        {
            Error(MSGSOURCE, "Unable to initialize the electron CP tool <%s>!",
                    t->name().c_str());
            return 3;
        }
        auto& handles = (j? electronSFTools : electronEffTools);
        handles.push_back(t->getHandle());
        /// Safer to retrieve the name from the final ToolHandle, it might be
        /// prefixed (by the parent tool name) when the handle is copied    
        name = handles[handles.size()-1].name();
        legsPerTool[name] = cfg[cLEGS];
        tagsPerTool[name] = cfg[cTAG];
        if(!j)
        {
            for(auto& tag : ::split_comma_delimited(cfg[cTAG]))
            {
                if(legsPerTag[tag]=="") legsPerTag[tag] = cfg[cLEGS];
                else legsPerTag[tag] += "," + cfg[cLEGS];
            }
        }

    }

    /* ********************************************************************** */
    
    Info(MSGSOURCE, "Configuring the global trigger SF tool");
    asg::AnaToolHandle<ITrigGlobalEfficiencyCorrectionTool> myTool("TrigGlobalEfficiencyCorrectionTool/TrigGlobal");
    myTool.setProperty("ElectronEfficiencyTools", electronEffTools).ignore();
    myTool.setProperty("ElectronScaleFactorTools", electronSFTools).ignore();
    const char* triggers2015 = 
        "e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose"
        "|| 2e12_lhloose_L12EM10VH";
    myTool.setProperty("TriggerCombination2015", triggers2015).ignore();
    myTool.setProperty("LeptonTagDecorations", "Signal").ignore();
    myTool.setProperty("ListOfLegsPerTool", legsPerTool).ignore();
    myTool.setProperty("ListOfTagsPerTool", tagsPerTool).ignore();
    myTool.setProperty("ListOfLegsPerTag", legsPerTag).ignore();

    if(debug) myTool.setProperty("OutputLevel", MSG::DEBUG).ignore();
    if(toys) myTool.setProperty("NumberOfToys", 1000).ignore();
    if(myTool.initialize() != StatusCode::SUCCESS)
    {
        Error(MSGSOURCE, "Unable to initialize the TrigGlob tool!");
        return 3;
    }
    
    /// Uniform random run number generation spanning the target dataset.
    /// In real life, use the PileupReweightingTool instead!
    const unsigned periodRuns[] = {
        276073, 278727, 279932, 280423, 281130, 282625 /// 2015 periods D-H, J
    };
    std::uniform_int_distribution<unsigned> uniformPdf(0,
            sizeof(periodRuns)/sizeof(*periodRuns) - 1);
    std::default_random_engine randomEngine;
    
    /* ********************************************************************** */
    
    Info(MSGSOURCE, "Starting the event loop");
    unsigned errors = 0;
    double nSuitableEvents = 0., sumW = 0.;
    for(Long64_t entry = 0; entry < entries; ++entry)
    {
        event.getEntry(entry);
        
        /// Get a random run number, and decorate the event info
        const xAOD::EventInfo* eventInfo = nullptr;
        event.retrieve(eventInfo,"EventInfo").ignore();
        unsigned runNumber = periodRuns[uniformPdf(randomEngine)];
        eventInfo->auxdecor<unsigned>("RandomRunNumber") = runNumber;
        vector<const xAOD::Electron*> myTriggeringElectrons;
        const xAOD::ElectronContainer* electrons = nullptr;
        event.retrieve(electrons,"Electrons").ignore();
        for(auto electron : *electrons)
        {
            if(!electron->caloCluster()) continue;
            float eta = fabs(electron->caloCluster()->etaBE(2));
            float pt = electron->pt();
            if(pt<10e3f || eta>=2.47) continue;
            int t = electron->auxdata<int>("truthType");
            int o = electron->auxdata<int>("truthOrigin");
            if(t!=2 || !(o==10 || (o>=12 && o<=22) || o==43)) continue;
            /// electron must be above softest trigger threshold (e12 here)
            if(pt < 13e3f) continue;

            myTriggeringElectrons.push_back(electron);
        }

        vector<const xAOD::Muon*> myTriggeringMuons;

        /// Add 'Signal' decorations to random electrons
        /// also count 'Signal' electrons above e24_xxx threshold
        unsigned nTrig1L = 0;
        for(auto electron : myTriggeringElectrons)
        {
            bool signal = bernoulliPdf(randomEngine);
            dec_signal(*electron) = signal? 1 : 0;
            if(signal && electron->pt()>25e3f) ++nTrig1L;
        }
        
        /// Events must contain enough leptons to trigger
        if(nTrig1L < 1 /// single-electron trigger
            && myTriggeringElectrons.size() < 2) /// dielectron
        {
            continue;
        }


        /// Finally retrieve the global trigger scale factor
        double sf = 1.;
        auto cc = myTool->getEfficiencyScaleFactor(myTriggeringElectrons,
            myTriggeringMuons, sf);
        if(cc==CP::CorrectionCode::Ok)
        {
            nSuitableEvents += 1;
            sumW += sf;
        }
        else
        {
            Warning(MSGSOURCE, "Scale factor evaluation failed");
            ++errors;
        }
        if(errors>10)
        {
            Error(MSGSOURCE, "Too many errors reported!");
            break;
        }
    }
    Info(MSGSOURCE, "Average scale factor: %f (over %ld events)",
            sumW / nSuitableEvents, long(nSuitableEvents));
    #ifndef XAOD_STANDALONE
        app->finalize();
    #endif
    return errors? 4 : 0;
}

/// Split comma-delimited string
namespace
{
    inline vector<string> split_comma_delimited(const string& s)
    {
        std::stringstream ss(s);
        std::vector<std::string> tokens;
        std::string token;
        while(std::getline(ss, token, ','))
        {
            if(token.length()) tokens.push_back(token);
        }
        return tokens;
    }
}
