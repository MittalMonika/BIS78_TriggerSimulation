/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// ********************************************************************
//
// NAME:     TileCellMonTool.cxx
// PACKAGE:  TileMonitoring  
//
// AUTHORS:   Alexander Solodkov
//            Luca Fiorini (Luca.Fiorini@cern.ch) 
//
// ********************************************************************

#include "TileMonitoring/TileCellMonTool.h"
#include "TileMonitoring/PairBuilder.h"

#include "xAODEventInfo/EventInfo.h"

#include "CaloEvent/CaloCellContainer.h"

#include "TileCalibBlobObjs/TileCalibUtils.h"
#include "TileIdentifier/TileHWID.h"
#include "TileConditions/TileCablingService.h"
#include "TileConditions/ITileBadChanTool.h"
#include "TileEvent/TileCell.h"

#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TProfile2D.h"
#include "TString.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include <sstream>
#include <iomanip>
#include <map>

using CLHEP::GeV;
using CLHEP::ns;


// Make a TileCell namespace so this doesn't cause problems else where.
namespace TC{
template<class T>
std::string to_string(T in){
  std::ostringstream strm;
  strm << in;
  return strm.str();
}
}

/*---------------------------------------------------------*/
TileCellMonTool::TileCellMonTool(const std::string & type, const std::string & name, const IInterface* parent)
  : TileFatherMonTool(type, name, parent)
  , m_tileBadChanTool("TileBadChanTool")
  , m_TileCellTrig(0U)
  , m_delta_lumiblock(0U)
  , m_TileBadCell(0)
/*---------------------------------------------------------*/
{
  declareInterface<IMonitorToolBase>(this);

  declareProperty("energyThreshold"        , m_Threshold=300.); //Threshold in MeV
  declareProperty("energyThresholdForTime" , m_ThresholdForTime=500.); //Threshold in MeV
  declareProperty("negEnergyThreshold"     , m_NegThreshold=-2000.); //Threshold in MeV
  declareProperty("cellsContainerName"     , m_cellsContName="AllCalo"); //SG Cell Container
  declareProperty("energyBalanceThreshold" , m_EneBalThreshold=3.); //Threshold is in the energy ratio
  declareProperty("TimeBalanceThreshold"   , m_TimBalThreshold=25.); //Threshold in ns
  declareProperty("doOnline"               , m_doOnline=false); //online mode
  declareProperty("TileBadChanTool"        , m_tileBadChanTool);
  
  m_path = "/Tile/Cell";

  std::ostringstream ss;
  std::string ebcell[48] = {"E3","E4","D4","D4","C10","C10","A12","A12","B11","B11","A13","A13",
                            "E1","E2","B12","B12","D5","D5","E3*","E4*","A14","A14","B13","B13",
                            "","","","","","","B14","A15","A15","","","B14",
                            "B15","D6","D6","B15","A16","A16","","","","","",""};

  std::string lbcell[48] = {"D0","A1","B1","B1","A1","A2","B2","B2","A2","A3","A3","B3",
                            "B3","D1","D1","A4","B4","B4","A4","A5","A5","B5","B5","A6",
                            "D2","D2","A6","B6","B6","A7","","","A7","B7","B7","A8",
                            "A9","A9","A8","B8","B8","D3","B9","","D3","A10","A10","B9"};

  m_PartNames[PartEBA] = "EBA";
  m_PartNames[PartLBA] = "LBA";
  m_PartNames[PartLBC] = "LBC";
  m_PartNames[PartEBC] = "EBC";

  for (int p = 0; p < NumPart; p++) {
    for (int m = 1; m < 65; m++) {
      ss.str("");
      if (m % 2 == 1) {
        ss << m_PartNames[p] << std::setfill('0') << std::setw(2) << m << std::setfill(' '); //EBA01, EBA03,...
      } else {
        ss.str(" ");
      }
      m_moduleLabel[p].push_back(ss.str());
    }

    for (int ch = 0; ch < 48; ch++) {
      ss.str("");
      if ((p < PartLBA) || (p > PartLBC))
        ss << ebcell[ch];
      else
        ss << lbcell[ch];
      if ((ss.str()).length() > 0) ss << "_";
      ss << "ch" << ch; //D0_ch1, A1_ch2, .., ch44, ...
      m_cellchLabel[p].push_back(ss.str());
    }
  }
  m_old_lumiblock= -1;
  m_isFirstEv = true;

}

/*---------------------------------------------------------*/
TileCellMonTool::~TileCellMonTool() {
/*---------------------------------------------------------*/

}

/*---------------------------------------------------------*/
StatusCode TileCellMonTool:: initialize() {
/*---------------------------------------------------------*/

  ATH_MSG_INFO( "in initialize()" );

  //=== get TileBadChanTool
  CHECK( m_tileBadChanTool.retrieve() );

  memset(m_nEventsProcessed, 0, sizeof(m_nEventsProcessed));

  return TileFatherMonTool::initialize();
}

/*---------------------------------------------------------*/
void TileCellMonTool::ShiftLumiHist(TProfile2D* Histo, int32_t delta_LB) {
/*---------------------------------------------------------*/

  for(int32_t binx = 20 - delta_LB; binx >= 1; binx--) {
    for(int32_t biny = 1; biny <= 8; biny++) {  //move bins

      // Looking at old bin to get moved
      double xCenter = Histo->GetXaxis()->GetBinCenter(binx);
      double yCenter = Histo->GetYaxis()->GetBinCenter(biny);
      int32_t globalBin = Histo->FindBin(xCenter,yCenter);
      
      // Getting values from old bin
      double oldContent=Histo->GetBinContent(globalBin);
      double oldEntries=Histo->GetBinEffectiveEntries(globalBin);
      
      // Looking at new bin to get filled
      xCenter = Histo->GetXaxis()->GetBinCenter(binx + delta_LB);
      yCenter = Histo->GetYaxis()->GetBinCenter(biny);
      globalBin = Histo->FindBin(xCenter,yCenter);
      
      // Filling in new bin with old values
      Histo->SetBinEntries(globalBin,oldEntries);
      Histo->SetBinContent(globalBin,oldContent*oldEntries);
    }
  }

  int32_t n0 = delta_LB;
  if (delta_LB > 20) {
    n0 = 20;
  }

  for (int32_t binx = 1; binx <= n0; binx++) { //set rest to zero
    for (int32_t biny = 1; biny <= 8; biny++) {
      // Looking at bin to get set to zero
      double xCenter = Histo->GetXaxis()->GetBinCenter(binx);
      double yCenter = Histo->GetYaxis()->GetBinCenter(biny);
      int32_t globalBin = Histo->FindBin(xCenter, yCenter);

      // Setting the bin to zero
      Histo->SetBinContent(globalBin, 0);
      Histo->SetBinEntries(globalBin, 0);
    }
  }
}
     

/*---------------------------------------------------------*/
StatusCode TileCellMonTool::bookHistTrigPart( int trig , int part ) {
/*---------------------------------------------------------*/


  int element =  m_activeTrigs[trig]; // taking the vector element from the trigger mapping array

  std::ostringstream sene; sene.str("");
  sene << m_Threshold;

  std::string runNumStr = getRunNumStr();
  /// Book Sampling Histos
  /// there are 1d histograms showing the energy and time balance for all cells together
  /// the 2d plots count the number of times the balance is outside the allowed range. 
  /// these are shown for as partition vs module number and for each partition module number vs cell number
  for (int sample = 0; sample < TotalSamp; sample++) {

    if (sample == 3) { //Only make Samp E
       m_TileCellEvEneSamp[part][ sample ].push_back( book1F(m_TrigNames[trig]+"/"+m_PartNames[part],
                                                             "tileCellEvEne"+m_SampStrNames[sample] + m_PartNames[part] + m_TrigNames[trig],
                                                             "Run "+runNumStr+" Trigger "+m_TrigNames[trig]+": TileCal Event "+m_SampStrNames[sample]+" Energy",
                                                             120,-2000.,10000., run, ATTRIB_MANAGED,
                                                             "", "mergeRebinned") );
       m_TileCellEvEneSamp[part][ sample ][ element ]->GetXaxis()->SetTitle("Event Energy (MeV)");
      //m_TileCellEvEneSamp[part][ sample ][ element ]->SetBit(TH1::kCanRebin);
    }

    if (sample != 3) { //Don't make Samp E 
      m_TileCellEneDiffSamp[part][ sample ].push_back( book1F(m_TrigNames[trig]+"/"+m_PartNames[part],
                                                              "tileCellEneDiff"+m_SampStrNames[sample] + m_PartNames[part] + m_TrigNames[trig],
                                                              "Run "+runNumStr+" Trigger "+m_TrigNames[trig]+": TileCell "+m_SampStrNames[sample]+" Energy difference (MeV) between PMTs",
                                                              50,-1000., 1000.) );
      m_TileCellEneDiffSamp[part][ sample ][ element ]->GetXaxis()->SetTitle("Energy diff (MeV)");
    }  

    
    if (sample != 3) { //Don't make samp E  
      m_TileCellTimeDiffSamp[part][ sample ].push_back( book1F(m_TrigNames[trig]+"/"+m_PartNames[part],
                                                               "tileCellTimeDiff"+m_SampStrNames[sample] + m_PartNames[part] + m_TrigNames[trig],
                                                               "Run "+runNumStr+" Trigger "+m_TrigNames[trig]+": TileCell "+m_SampStrNames[sample]+" Time difference (ns) between PMTs. Collision Events, either E_{ch} > "+TC::to_string(m_ThresholdForTime)+" MeV",
                                                               50,-10., 10.) );
      m_TileCellTimeDiffSamp[part][ sample ][ element ]->GetXaxis()->SetTitle("time (ns)");
    }
  }
  //END of the booking per sample




  if (m_doOnline) {
    m_TilenCellsLB[ part ].push_back( bookProfile(m_TrigNames[trig]+"/"+m_PartNames[part],"tilenCellsLB" + m_PartNames[part] + m_TrigNames[trig],
                                                  "Trigger "+m_TrigNames[trig]+": TileCal Cell number per LumiBlock",100, -99.5, 0.5) );
    m_TilenCellsLB[ part ][ element ]->GetXaxis()->SetTitle("Last LumiBlocks");
  } else {
    m_TilenCellsLB[ part ].push_back( bookProfile(m_TrigNames[trig]+"/"+m_PartNames[part],"tilenCellsLB" + m_PartNames[part] + m_TrigNames[trig],
                                                  "Trigger "+m_TrigNames[trig]+": TileCal Cell number per LumiBlock", 1500, -0.5, 1499.5) );
    m_TilenCellsLB[ part ][ element ]->GetXaxis()->SetTitle("LumiBlock");
  }

  m_TilenCellsLB[ part ][ element ]->GetYaxis()->SetTitle("Number of reconstructed cells");

  m_TileCellModuleCorr[ part ].push_back( book2F(m_TrigNames[trig]+"/"+m_PartNames[part],"tileCellModuleCorr" + m_PartNames[part] + m_TrigNames[trig],
                                                 "Run "+runNumStr+" Trigger "+m_TrigNames[trig]+": TileCal Cell Module correlation",64,-0.5, 63.5,64,-0.5,63.5) );
  m_TileCellModuleCorr[ part ][ element ]->GetXaxis()->SetTitle("cell Module");
  m_TileCellModuleCorr[ part ][ element ]->GetYaxis()->SetTitle("cell Module");
 
    
  if ( part != NumPart ){ //Don't make AllPart
    m_TileCellEvEneTim[ part ].push_back( bookProfile(m_TrigNames[trig]+"/"+m_PartNames[part],"tileCellEvEneTim"+m_PartNames[part] + m_TrigNames[trig],"Run "+runNumStr+" Trigger "+m_TrigNames[trig]+": Partition "+m_PartNames[part]+": Event Energy as a function of the event number",25,0., 100.,-2000.,200000.,  run, ATTRIB_MANAGED, "", "mergeRebinned") );
    m_TileCellEvEneTim[ part ][ element ]->SetYTitle("Average Event Energy (MeV)");
    m_TileCellEvEneTim[ part ][ element ]->SetXTitle("Event Number");
    //m_TileCellEvEneTim[ part ][ element ]->SetBit(TH1::kCanRebin);
  }

  if ( part != NumPart ){ //Don't Make allpart
    m_TileCellEvEneLumi[ part ].push_back( bookProfile(m_TrigNames[trig]+"/"+m_PartNames[part],"tileCellEvEneLumi"+m_PartNames[part] + m_TrigNames[trig],"Run "+runNumStr+" Trigger "+m_TrigNames[trig]+": Partition "+m_PartNames[part]+": Event Energy as a function of the LumiBlock",10,0., 20.,-5.e6,5.e6, run, ATTRIB_MANAGED, "", "mergeRebinned") );
    m_TileCellEvEneLumi[ part ][ element ]->SetYTitle("Average Event Energy (MeV)");
    m_TileCellEvEneLumi[ part ][ element ]->SetXTitle("LumiBlock");
    //m_TileCellEvEneLumi[ part ][ element ]->SetBit(TH1::kCanRebin);
  }


  m_TileCellOccOvThrBCID[ part ].push_back( bookProfile(m_TrigNames[trig]+"/"+m_PartNames[part],"tileCellOccOvThrBCID" + m_PartNames[part] + m_TrigNames[trig],
                                                        "Run "+runNumStr+" Trigger "+m_TrigNames[trig]+": TileCal Cell Occupancy over Threshold "+sene.str()+" MeV in "+m_PartNames[part]+" as a function of BCID",3565,0.,3565.,0.,10000.));

  m_TileCellOccOvThrBCID[ part ][ element ]->SetXTitle("BCID");
  m_TileCellOccOvThrBCID[ part ][ element ]->SetYTitle("Average number of cells over threshold");

  if (part < NumPart) {
    // last element of array exists, but it is not used. We don't book it, in order to avoid wasting memory and time


      
    m_TileCellDetailOccMapOvThr[ part ].push_back( book2F(m_TrigNames[trig]+"/"+m_PartNames[part],
                                                          "tileCellDetailOccMapOvThr_"+m_PartNames[part] + m_TrigNames[trig],
                                                          "Run "+runNumStr+" Trigger "+m_TrigNames[trig]+" Partition "+m_PartNames[part]+
                                                          ":  Hi Gain Occupancy Map Over Threshold "+sene.str()+" MeV",
                                                          64,0.5, 64.5,48,-0.5,47.5) ) ;

    SetBinLabel(m_TileCellDetailOccMapOvThr[ part ][ element ]->GetYaxis(),m_cellchLabel[part]);
    SetBinLabel(m_TileCellDetailOccMapOvThr[ part ][ element ]->GetXaxis(),m_moduleLabel[part]);

    m_TileCellDetailOccMapLowGainOvThr[ part ].push_back( book2F(m_TrigNames[trig]+"/"+m_PartNames[part],
                                                          "tileCellDetailOccMapLowGainOvThr_"+m_PartNames[part] + m_TrigNames[trig],
                                                          "Run "+runNumStr+" Trigger "+m_TrigNames[trig]+" Partition "+m_PartNames[part]+
                                                          ": Low Gain Occupancy Map Over Threshold "+sene.str()+" MeV",
                                                          64,0.5, 64.5,48,-0.5,47.5) ) ;

    SetBinLabel(m_TileCellDetailOccMapLowGainOvThr[ part ][ element ]->GetYaxis(),m_cellchLabel[part]);
    SetBinLabel(m_TileCellDetailOccMapLowGainOvThr[ part ][ element ]->GetXaxis(),m_moduleLabel[part]);

      
    m_TileCellDetailOccMapOvThr30GeV[ part ].push_back( book2F(m_TrigNames[trig]+"/"+m_PartNames[part],
                                                          "tileCellDetailOccMapOvThr30GeV_"+m_PartNames[part] + m_TrigNames[trig],
                                                          "Run "+runNumStr+" Trigger "+m_TrigNames[trig]+" Partition "+m_PartNames[part]+
                                                          ": Occupancy Map Over Threshold 30 GeV",
                                                          64,0.5, 64.5,48,-0.5,47.5) ) ;

    SetBinLabel(m_TileCellDetailOccMapOvThr30GeV[ part ][ element ]->GetYaxis(),m_cellchLabel[part]);
    SetBinLabel(m_TileCellDetailOccMapOvThr30GeV[ part ][ element ]->GetXaxis(),m_moduleLabel[part]);
      
    m_TileCellDetailOccMapOvThr300GeV[ part ].push_back( book2F(m_TrigNames[trig]+"/"+m_PartNames[part],
                                                          "tileCellDetailOccMapOvThr300GeV_"+m_PartNames[part] + m_TrigNames[trig],
                                                          "Run "+runNumStr+" Trigger "+m_TrigNames[trig]+" Partition "+m_PartNames[part]+
                                                          ": Occupancy Map Over Threshold 300 GeV",
                                                          64,0.5, 64.5,48,-0.5,47.5) ) ;

    SetBinLabel(m_TileCellDetailOccMapOvThr300GeV[ part ][ element ]->GetYaxis(),m_cellchLabel[part]);
    SetBinLabel(m_TileCellDetailOccMapOvThr300GeV[ part ][ element ]->GetXaxis(),m_moduleLabel[part]);



    m_TileCellDetailOccMap[ part ].push_back( bookProfile2D(m_TrigNames[trig]+"/"+m_PartNames[part],
                                                            "tileCellDetailOccMap_"+m_PartNames[part] + m_TrigNames[trig],
                                                            "Run "+runNumStr+" Trigger "+m_TrigNames[trig]+" Partition "+m_PartNames[part]+
                                                            ": Occupancy Map (MeV) (entries = events)",
                                                            64,0.5, 64.5,48,-0.5,47.5,-2.e6,2.e6) ) ;
      
    SetBinLabel(m_TileCellDetailOccMap[ part ][ element ]->GetYaxis(),m_cellchLabel[part]);
    SetBinLabel(m_TileCellDetailOccMap[ part ][ element ]->GetXaxis(),m_moduleLabel[part]);

    m_TileCellEneDiffChanMod[ part ].push_back( bookProfile2D(m_TrigNames[trig]+"/"+m_PartNames[part],
                                                              "tileCellEneDiffChanMod_"+m_PartNames[part] + m_TrigNames[trig],
                                                              "Run "+runNumStr+" Trigger "+m_TrigNames[trig]+" Partition "+m_PartNames[part]+
                                                              ": Energy difference betwean pmts (MeV)",
                                                              64,0.5, 64.5,48,-0.5,47.5,-2.e6,2.e6) ) ;
      
    SetBinLabel(m_TileCellEneDiffChanMod[ part ][ element ]->GetYaxis(),m_cellchLabel[part]);
    SetBinLabel(m_TileCellEneDiffChanMod[ part ][ element ]->GetXaxis(),m_moduleLabel[part]);


    // Channel timing plots below.
    std::string histDir, histName, histTitle, xTitle;
    TObject *obj;
    // Tile Cell average time temperature plot for each partition.
    histDir = m_TrigNames[trig]+"/"+m_PartNames[part];
    histName = "tileChanPartTime_"+m_PartNames[part] + m_TrigNames[trig];
    histTitle = "Run "+runNumStr+" Trigger "+m_TrigNames[trig]+" Partition "+
                m_PartNames[part]+": TileCal Average Channel Time (ns). "+
                "Collision Events, E_{ch} > "+TC::to_string(m_ThresholdForTime)+
                " MeV";
    obj = bookProfile2D(histDir,histName,histTitle, 64,0.5,64.5,48,-0.5,47.5,-80,80);
    m_TileChanPartTime[part].push_back(static_cast<TProfile2D*>(obj));
    SetBinLabel(m_TileChanPartTime[part][element]->GetXaxis(),  m_moduleLabel[part]);
    SetBinLabel(m_TileChanPartTime[part][element]->GetYaxis(), m_cellchLabel[part]);
    m_TileChanPartTime[part][element]->SetZTitle("Average Channel Time (ns)");

    // Tile Cell average digi time temperature plot for each partition.
    histName = "tileDigiPartTime_"+m_PartNames[part] + m_TrigNames[trig];
    histTitle = "Run "+runNumStr+" Trigger "+m_TrigNames[trig]+" Partition "+
                m_PartNames[part]+": TileCal Average Digitizer Time (ns). "+
               "Collision Events, E_{ch} > "+TC::to_string(m_ThresholdForTime)+
               " MeV";
    obj = bookProfile2D(histDir,histName,histTitle, 64,0.5,64.5,8,0.5,8.5,-80,80);
    m_TileDigiPartTime[part].push_back(static_cast<TProfile2D*>(obj));
    SetBinLabel(m_TileDigiPartTime[part][element]->GetXaxis(), m_moduleLabel[part]);
    m_TileDigiPartTime[part][element]->SetYTitle("Digitizer");
    m_TileDigiPartTime[part][element]->SetZTitle("Average Digitizer Time (ns)");

    // per module plots
    for (int mod=0; mod<64; mod++) {
       
    //Tile Cell average digitizer time per lumiblock for each module
        histName = "tileDigiTimeLB_" + m_PartNames[part] + "_" + TC::to_string(mod+1) + m_TrigNames[trig];
       histTitle = "Run " +runNumStr+" Trigger "+m_TrigNames[trig]+
                   " Partition "+m_PartNames[part]+" Module "+TC::to_string(mod+1)+
                   ": TileCal Average Digitizer Time (ns) vs. Lumiblock " +
                   "Collision Events, E_{ch} > "+TC::to_string(m_ThresholdForTime)+
                   " MeV";
       if (m_doOnline) {
         obj = bookProfile2D(histDir,histName,histTitle,20,-0.5,19.5,8,0.5,8.5,-100,100);
         m_TileDigiTimeLB[part][mod].push_back(static_cast<TProfile2D*>(obj));
         m_TileDigiTimeLB[part][mod][element]->SetXTitle("Last 20 Lumiblocks (left = most recent)");
         m_TileDigiTimeLB[part][mod][element]->SetYTitle("Digitizer");
         m_TileDigiTimeLB[part][mod][element]->SetZTitle("Average Digitizer Time (ns)");
       }
       else {
         obj = bookProfile2D(histDir,histName,histTitle,150,-0.5,1499.5,8,0.5,8.5,-100,100);
         m_TileDigiTimeLB[part][mod].push_back(static_cast<TProfile2D*>(obj));
         m_TileDigiTimeLB[part][mod][element]->SetXTitle("Lumiblock (10 per bin)");
         m_TileDigiTimeLB[part][mod][element]->SetYTitle("Digitizer");
         m_TileDigiTimeLB[part][mod][element]->SetZTitle("Average Digitizer Time (ns)");
       }
    
       //Tile Cell average digitizer energy per lumiblock for each module
       histName = "tileDigiEnergyLB_" + m_PartNames[part] + "_" + TC::to_string(mod+1) + m_TrigNames[trig];
       histTitle = "Run " +runNumStr+" Trigger "+m_TrigNames[trig]+
                   " Partition"+m_PartNames[part]+" Module "+TC::to_string(mod+1)+
                   ": TileCal Average Digitizer Energy (MeV) vs. Lumiblock" +
                   "Collsion Events";
       if (m_doOnline) {
         obj = bookProfile2D(histDir,histName,histTitle,20,-0.5,19.5,8,0.5,8.5,-3000,3000);
         m_TileDigiEnergyLB[part][mod].push_back(static_cast<TProfile2D*>(obj));
         m_TileDigiEnergyLB[part][mod][element]->SetXTitle("Last 20 Lumiblocks (left = most recent)");
         m_TileDigiEnergyLB[part][mod][element]->SetYTitle("Digitizer");
         m_TileDigiEnergyLB[part][mod][element]->SetZTitle("Average Digitizer Energy (MeV)");
       }
       else {
         obj = bookProfile2D(histDir,histName,histTitle,150,-0.5,1499.5,8,0.5,8.5,-3000,3000);
         m_TileDigiEnergyLB[part][mod].push_back(static_cast<TProfile2D*>(obj));
         m_TileDigiEnergyLB[part][mod][element]->SetXTitle("Lumiblock (10 per bin)");
         m_TileDigiEnergyLB[part][mod][element]->SetYTitle("Digitizer");
         m_TileDigiEnergyLB[part][mod][element]->SetZTitle("Average Digitizer Energy (MeV)");
       }
    
    }

  }

  return StatusCode::SUCCESS;
}

/*---------------------------------------------------------*/
StatusCode TileCellMonTool::bookHistTrig( int trig ) {
/*---------------------------------------------------------*/

  
  m_activeTrigs[ trig ] = m_TileCellTrig; 
  int element = m_activeTrigs[ trig ];

  std::ostringstream ss; ss.str("");
  ss << m_EneBalThreshold;

  std::string runNumStr = getRunNumStr();

  m_TileCellTrig++; //increment booking index

  // occupancy plots of the energy and time balance
  m_TileCellEneBalModPart.push_back( book2S(m_TrigNames[trig],"tileCellEneBalModPart" + m_TrigNames[trig],"Run "+runNumStr+" Trigger "+m_TrigNames[trig]+": Tile Cell Energy Ratio > "+ss.str(),64,0.5,64.5, 4,-0.5,3.5) );
  m_TileCellEneBalModPart[ element ]->GetXaxis()->SetTitle("Module Number");
  m_TileCellEneBalModPart[ element ]->GetYaxis()->SetTitle("Partition Number");

  ss.str("");
  ss << m_TimBalThreshold;
  m_TileCellTimBalModPart.push_back( book2S(m_TrigNames[trig],"tileCellTimBalModPart" + m_TrigNames[trig],"Run "+runNumStr+" Trigger "+m_TrigNames[trig]+" Partition ALL: Tile Cell Timing Difference above "+ss.str()+" ns. Collision Events, either E_{ch} > "+TC::to_string(m_ThresholdForTime)+" MeV",64,0.5,64.5, 4,-0.5,3.5) );
  m_TileCellTimBalModPart[ element ]->GetXaxis()->SetTitle("Module Number");
  m_TileCellTimBalModPart[ element ]->GetYaxis()->SetTitle("Partition Number");

  SetBinLabel(m_TileCellEneBalModPart[ element ]->GetYaxis(), m_PartNames,4);
  SetBinLabel(m_TileCellTimBalModPart[ element ]->GetYaxis(), m_PartNames,4);

  std::ostringstream sene; sene.str("");
  sene << m_Threshold;
      
  std::string histDir, histName, histTitle, xTitle;

  /// Book Sampling Histos
  for (int sample=0; sample<TotalSamp; sample++) {
    m_TileCellEtaPhiOvThrSamp[sample].push_back( book2I(m_TrigNames[trig],"tileCellEtaPhiOvThr"+m_SampStrNames[sample] + m_TrigNames[trig],"Run "+runNumStr+" Trigger "+m_TrigNames[trig]+": Tile Cell "+m_SampStrNames[sample]+" Position of cells over threshold "+sene.str()+" MeV",21,-2.025, 2.025,64,-3.15,3.15) );
    m_TileCellEtaPhiOvThrSamp[sample][ element ]->GetXaxis()->SetTitle("#eta");
    m_TileCellEtaPhiOvThrSamp[sample][ element ]->GetYaxis()->SetTitle("#phi");

    m_TileCellEneEtaPhiSamp[sample].push_back( bookProfile2D(m_TrigNames[trig],"tileCellEneEtaPhi"+m_SampStrNames[sample] + m_TrigNames[trig],"Run "+runNumStr+" Trigger "+m_TrigNames[trig]+": Tile 2D Cell "+m_SampStrNames[sample]+" Energy Average depostion (MeV) (entries = events)",21,-2.025, 2.025,64,-3.15,3.15,-2.e6, 2.e6) );
    m_TileCellEneEtaPhiSamp[sample][ element ]->GetXaxis()->SetTitle("#eta");
    m_TileCellEneEtaPhiSamp[sample][ element ]->GetYaxis()->SetTitle("#phi");

  }


  m_TileCellSynch.push_back( book1F(m_TrigNames[trig],"tileCellSynch" + m_TrigNames[trig],"Run "+runNumStr+" Trigger "+m_TrigNames[trig]+": Tile Time of Flight - Time measured ",50,-100., 100.) );
  m_TileCellSynch[ element ]->GetXaxis()->SetTitle("Time of Flight - Time Measured (ns)");  

  for (int p = 0; p < NPartHisto; p++) {
    if ( bookHistTrigPart( trig,  p ).isFailure() ) {
      ATH_MSG_WARNING( "Error booking TileCell histograms for trigger " << m_TrigNames[trig]
                        << ", partition " << m_PartNames[p] );
    }
  }

  return StatusCode::SUCCESS;
}

/// BookHistogram method is called at every event block ,lumi block and run.
/// At the moment we need to book only every run
/// It calls bookHistTrig for the 'AnyTrig' trigger type
/// All concrete trigger type histograms will only be booked when needed
/*---------------------------------------------------------*/
StatusCode TileCellMonTool::bookHistograms() {
/*---------------------------------------------------------*/

  ATH_MSG_INFO( "in bookHistograms()" );
  ATH_MSG_INFO( "Using base path " << m_path );

  cleanHistVec(); //necessary to avoid problems at the run, evblock, lumi blocks boundaries
  m_isFirstEv = true;

  return StatusCode::SUCCESS;
}

/*---------------------------------------------------------*/
void  TileCellMonTool::cleanHistVec() {
/*---------------------------------------------------------*/


  for (int i = 0; i < 9; i++) {
    m_activeTrigs[i] = -1;
  }

  memset(m_nEventsProcessed, 0, sizeof(m_nEventsProcessed));

  //m_TileCellEtaPhiOvThr.clear() ;
  //m_TileCellEtaOvThr.clear() ;
  //m_TileCellPhiOvThr.clear() ;
  //m_TileCellEneEtaPhi.clear() ;
  //m_TileCellEneEta.clear() ;
  //m_TileCellEnePhi.clear() ;

  //m_TileCellOccModPart.clear() ;
  //m_TileCellOccModPartOvThr.clear() ;
  m_TileCellEneBalModPart.clear() ;
  m_TileCellTimBalModPart.clear() ;

  m_TileCellTrig = 0;  

  m_TileCellSynch.clear() ;

  //m_TileEventsPerBCID.clear();

  for (int part = 0; part < NPartHisto; part++) {
    m_TileCellDetailOccMapOvThr[part].clear();
    m_TileCellDetailOccMapLowGainOvThr[part].clear();
    m_TileCellDetailOccMapOvThr30GeV[part].clear();
    m_TileCellDetailOccMapOvThr300GeV[part].clear();
    m_TileCellDetailOccMap[part].clear();
    m_TileCellEneDiffChanMod[part].clear();
 
   //m_TilenCells[part].clear() ;
    m_TilenCellsLB[part].clear() ;
    m_TileCellModuleCorr[part].clear() ;
    //m_TileCellTime[part].clear() ;
    //m_TileCellEnergy[part].clear() ;
    //m_TileCellEnergyRebin[part].clear();

    if (part != NumPart) {m_TileCellEvEneTim[part].clear() ;} // Don't make AllPart
    if (part != NumPart) {m_TileCellEvEneLumi[part].clear() ;} // Don't make AllPart
    //m_TileCellEvEneRebin[part].clear() ;

    m_TileCellOccOvThrBCID[part].clear(); //last element not used
    //m_TileCellOccEneBCID[part].clear();

    for (int sample = 0; sample < TotalSamp; sample++) {
      if (sample != 3) {
        m_TileCellEneDiffSamp[part][sample].clear();
      } //Don't make samp E
      //m_TileCellEneRatSamp[part][sample].clear() ;
      if (sample != 3) {
        m_TileCellTimeDiffSamp[part][sample].clear();
      } //Don't make samp E
      if (sample == 3) {
        m_TileCellEvEneSamp[part][sample].clear();
      } //only make samp E for this plot
    }

  }

  for (int sample = 0; sample < TotalSamp; sample++) {
    m_TileCellEtaPhiOvThrSamp[sample].clear();
    m_TileCellEneEtaPhiSamp[sample].clear();
    //m_TileCellOccModPartSamp[sample].clear();
    //m_TileCellOccModPartOvThrSamp[sample].clear();
  }

  //m_TileChanDetTime.clear();
  for (int part = 0; part < NumPart; part++) {
    m_TileChanPartTime[part].clear();
    m_TileDigiPartTime[part].clear();
    //m_TileChanModTime[part].clear();
    for (int mod = 0; mod < 64; mod++) {
      //m_TileChanChTime[part][mod].clear();
      m_TileDigiTimeLB[part][mod].clear();
      m_TileDigiEnergyLB[part][mod].clear();
    }
  }

}
/*---------------------------------------------------------*/
StatusCode TileCellMonTool::fillHistograms() {
/*---------------------------------------------------------*/

  
  // conversion from channel number to digitizer number
  static const int ch2digi[48] = {
                        8, 8, 8,  8, 8, 8,
                        7, 7, 7,  7, 7, 7,
                        6, 6, 6,  6, 6, 6,
                        5, 5, 5,  5, 5, 5,
                        4, 4, 4,  4, 4, 4,
                        3, 3, 3,  3, 3, 3,
                        2, 2, 2,  2, 2, 2,
                        1, 1, 1,  1, 1, 1};

  // weights for E-cells, only towers 10..15
  static const double Eweight[6] = {1./3., 1./6., 1./11., 1./11., 1./28., 1./28.};

  // get Lvl1 Trigger word
  fillEvtInfo();
  if (msgLvl(MSG::DEBUG)) {
    msg(MSG::DEBUG) << "Run = "        << m_runNum
                    << " LB = "        << m_lumiBlock
                    << " Evt = "       << m_evtNum
                    << " BCID = "      << m_evtBCID
                    << " lvl1 = 0x"    << std::hex << m_lvl1info << std::dec;

    const DataHandle<xAOD::EventInfo> eventInfo;
    if (evtStore()->retrieve(eventInfo).isSuccess()) {
      const std::vector<xAOD::EventInfo::StreamTag>& evtStreamTags = eventInfo->streamTags();
      if (!evtStreamTags.empty()) {
        msg(MSG::DEBUG) << " stream name/type:";
        for (const auto& evtStreamTag : evtStreamTags) {
          msg(MSG::DEBUG) << " " << evtStreamTag.name() << "/" << evtStreamTag.type();
        }
      }
    }

    msg(MSG::DEBUG) << endmsg;
  }

  uint32_t lvl1info = getL1info();
  uint32_t evtNum = getEvtNum();
  uint32_t lumi = getLumiBlock();
  unsigned int bcid = TileFatherMonTool::getEvtBCID();
  get_eventTrigs(lvl1info); //fill  m_eventTrigs; it always contains at least one element: AnyPhysTrig or Calib.
  
  if (m_isFirstEv) {
    FirstEvInit();
  }

  //check if trigger type is new and therefore some histograms must be booked
  for (unsigned int i = 0; i < m_eventTrigs.size(); i++) {
    if ( m_activeTrigs[  m_eventTrigs[i] ] < 0 ) {
      // book histograms
      if ( bookHistTrig(  m_eventTrigs[i] ).isFailure() ) {
        ATH_MSG_WARNING( "Error booking TileCell histograms for trigger " << m_TrigNames[m_eventTrigs[i]] );
      }
    }
  }

  // Pointer to a Tile cell container
  const CaloCellContainer* cell_container;

  //Retrieve Cell collection from SG
  CHECK( evtStore()->retrieve(cell_container, m_cellsContName) );

  m_muonCells.clear();
  PairBuilder modulecorr[NPartHisto];

  double eTileSamp[TotalSamp][TotalSamp] = { { 0. } };
  int nCells[NPartHisto] = { 0 };
  int nBadCells[NPartHisto] = { 0 };

  //arrays for BCID plots
  unsigned int cellcnt[NPartHisto] = { 0 };

  //number of channels masked on the fly
  unsigned int badonfly[NPartHisto] = { 0 };

  CaloCellContainer::const_iterator iCell = cell_container->begin();
  CaloCellContainer::const_iterator lastCell  = cell_container->end();
  
  for ( ; iCell != lastCell; ++iCell) {

    const CaloCell* cell_ptr = *iCell;     // pointer to cell object
    Identifier id = cell_ptr->ID();

    if ( m_tileID->is_tile(id) ) {

      const TileCell* tile_cell = dynamic_cast<const TileCell*>(cell_ptr);
      if (tile_cell == 0) continue;

      int drw = 0; // drawer number, range 0-63, the same for both channels
      short ch1 = -1, ch2 = -1; // channel number, range 0-47 or -1 for unknown
      int partition = NumPart;
      int partition2 = NumPart;

      int ros1(-1);
      int ros2(-1);

      const CaloDetDescrElement * caloDDE = tile_cell->caloDDE();

      IdentifierHash hash1 = caloDDE->onl1();
      if (hash1 != TileHWID::NOT_VALID_HASH) {
          HWIdentifier hw1 = m_tileHWID->channel_id(hash1);
          ch1 = m_tileHWID->channel(hw1);
          drw = m_tileHWID->drawer(hw1);
          ros1 = m_tileHWID->ros(hw1);
          partition = m_ros2partition[ros1];
      }
  
      IdentifierHash hash2 = caloDDE->onl2();
      if (hash2 != TileHWID::NOT_VALID_HASH) {
          HWIdentifier hw2 = m_tileHWID->channel_id(hash2);
          ch2 = m_tileHWID->channel(hw2);
          drw = m_tileHWID->drawer(hw2);
          ros2 = m_tileHWID->ros(hw2);
          partition2 = m_ros2partition[ros2];
      }

      // just to avoid potential problems with disconnected cells
      if (partition >= NumPart) partition = getPartition(cell_ptr);
      if (partition2 >= NumPart) partition2 = partition;
      
      // something is wrong - go to next cell
      if (partition >= NumPart) continue;

      double fpartition = (double) partition;
      // note that drawer from HWID and module from ID are different for E3 cells near MBTS
      double drawer = (double)drw + 1.0; // range from 1-64
      double module = (double)m_tileID->module(id) + 1.0; // range from 1-64
  
      int samp = std::min(m_tileID->sample(id),(int)AllSamp);

      bool single_PMT_scin = (samp == TileID::SAMP_E);
      bool single_PMT_C10 = (m_tileID->section(id) == TileID::GAPDET
                              && samp == TileID::SAMP_C
                              && (! m_cabling->C10_connected(m_tileID->module(id))) );

      // distinguish cells with one or two PMTs
      bool single_PMT = single_PMT_C10 || single_PMT_scin;

      // distinguish normal cells and fantoms (e.g. non-existing D4 in EBA15, EBC18
      // or non-existing E3/E4 - they might appear in CaloCellContainer)
      bool real_cell  = single_PMT_C10 || m_cabling->TileGap_connected(id);

      double weight = 1.;
      if (single_PMT_scin) {
        int twr = m_tileID->tower(id);
        if (twr>9 && twr<16) weight = Eweight[twr-10];
      }

      // note that in single PMT cell both badch1() and badch2() are changed together
      bool badch1 = (tile_cell->badch1());
      bool badch2 = (tile_cell->badch2());

      // 0 = both PMTs are good; 1= 1 PMT is bad; 2= both PMTs are bad, or PMT is bad for single PMT cell
      int cell_isbad = (int)badch1 + (int)badch2;

      int gn1 = tile_cell->gain1(); // gain of first PMT
      int gn2 = tile_cell->gain2(); // gain of second PMT

      bool ch1Ok = (ch1>-1 && gn1 != CaloGain::INVALIDGAIN);
      bool ch2Ok = (ch2>-1 && gn2 != CaloGain::INVALIDGAIN);


      // get the cell energy, time and position info
      double energy = cell_ptr->energy();
      double time = cell_ptr->time();
      // double quality = cell_ptr->quality();
      double eta = cell_ptr->eta();
      double phi = cell_ptr->phi();
      double ene1 = tile_cell->ene1();
      double ene2 = tile_cell->ene2();
      double ediff = (single_PMT) ? 0.0 : tile_cell->eneDiff();
      double eratio = (energy!=0.0) ? ediff/energy : 0.0;
      double t1 = tile_cell->time1();
      double t2 = tile_cell->time2();
      double tdiff = (single_PMT) ? 0.0 : 2.*tile_cell->timeDiff(); // attention! factor of 2 is needed here

      //Variables for lumiblock based plots
      int32_t current_lumiblock = getLumiBlock();
      if(m_old_lumiblock == -1) {
        m_old_lumiblock = current_lumiblock;
        for(int32_t parts = 0; parts < 4; parts++) { //initialize old lumiblock tracker array
          for(int32_t mods = 0; mods < 64; mods++) {
            for(int32_t vecInds = 0; vecInds < 4; vecInds++){
              m_OldLumiArray1[parts][mods][vecInds] = current_lumiblock;
              m_OldLumiArray2[parts][mods][vecInds] = current_lumiblock;
            }
          }
        }
      }

      if (msgLvl(MSG::VERBOSE)) {
          //tile_cell->print();
          //log << MSG::VERBOSE << "HWIdentifier: " << tile_cell->adc_HWID() << endreq;
          
        msg(MSG::VERBOSE) << "Identifier: " << id << " " << m_tileID->to_string(id,-2) << endmsg;
        msg(MSG::VERBOSE) << "  region: " << m_tileID->region(id)
                          << "  system: " << m_tileID->system(id)
                          << "  section: "<< m_tileID->section(id)
                          << "  side: "   << m_tileID->side(id)
                          << "  module: " << m_tileID->module(id)
                          << "  tower: "  << m_tileID->tower(id)
                          << "  sample: " << m_tileID->sample(id)  <<  endmsg;

        msg(MSG::VERBOSE) << "1st PMT partition: " << partition
                          << " drawer: " << drw
                          << " ch1: " << ch1
                          << " gain1: " << gn1
                          << " qual1: " << (int)tile_cell->qual1()
                          << " qbit1: " << (int)tile_cell->qbit1()
                          << ((drw != m_tileID->module(id))?"   drawer != module":" ") << endmsg;

        msg(MSG::VERBOSE) << "2nd PMT partitio2: " << partition2
                          << " drawer: " << drw
                          << " ch2: " << ch2
                          << " gain2: " << gn2
                          << " qual2: " << (int)tile_cell->qual2()
                          << " qbit2: " << (int)tile_cell->qbit2()
                          << ((partition2 != partition) ? "   partiton2 != partition" : " ") << endmsg;

        msg(MSG::VERBOSE) << "Bad status: " << cell_isbad
                          << " bad1: " << badch1
                          << " bad2: " << badch2
                          << ((single_PMT_scin && badch1
                              && m_TileCellStatFromDB[partition][0]->GetBinContent(drw+1, ch1+1)+m_TileCellStatFromDB[partition][1]->GetBinContent(drw+1, ch1+1) == 0)
                              ? "  NEW BAD E-cell" : " ") << endmsg ;

        msg(MSG::VERBOSE) << "  real_cell: " << ((real_cell) ? "true" : "false")
                          << "  E-cell: " << ((single_PMT_scin) ? "true" : "false")
                          << "  specC10: " << ((single_PMT_C10) ? "true" : "false") << endmsg;

        msg(MSG::VERBOSE) << "Energy= " << energy << " = " << ene1 << " + " << ene2
                          << "  ediff= " << ediff
                          << "  eratio= " << eratio
                          << "\t Energy/(GeV)= " << energy/(GeV) << endmsg ;

        msg(MSG::VERBOSE) << "Time= " << time << " = (" << t1 << " + " << t2 << ")/2  "
                          << "tdiff= " << tdiff
                          << "\t time/(ns)= " << time/(ns) << endmsg ;
      }


      if (real_cell) {
          
        // masking on the fly: check if channel is bad in event, while it is good in DB

        // when only one channel is bad, it might be that gain of masked channel is not correct
        // if two qualities are not identical, then gains are not the same (new feature introduced in rel 17.2)
        int qual1 = (int)tile_cell->qual1();
        int qual2 = (int)tile_cell->qual2();
        if (badch1 != badch2 && qual1 != qual2 && qual1 < 255 && qual2 < 255) {
          if (badch1 && ch1Ok) gn1 = 1 - gn1;
          if (badch2 && ch2Ok) gn2 = 1 - gn2;
        }

        if (badch1 && ch1Ok) {
          if (m_TileCellStatFromDB[partition][gn1]->GetBinContent(drw + 1, ch1 + 1) == 0) {
            ++badonfly[partition];
            m_TileCellStatOnFly[partition]->Fill(drawer, ch1);
          }
        }
    
        if (badch2 && ch2Ok) {
          if (m_TileCellStatFromDB[partition2][gn2]->GetBinContent(drw + 1, ch2 + 1) == 0) {
            ++badonfly[partition2];
            m_TileCellStatOnFly[partition2]->Fill(drawer, ch2);
          }
        }
    
        if ((energy > 300.) && (energy < 2000.)
            && (time > -60.) && (time < 60.) && (time != 0)
            && (cell_isbad < 2)) {

          m_muonCells.push_back(cell_ptr);
        }
    
        //if (m_nEvents<2) 
        //  if (cell_isbad>1) m_TileBadCell->Fill(partition);
    
        // check if at least 1 cell's pmt is good
        if (cell_isbad < 2) {

          for (unsigned int i = 0; i < m_eventTrigs.size(); i++) {
            int vecInd = vecIndx(i);
    
            //m_TileCellEnergy[ partition ][ vecInd ]->Fill(energy, 1.);
            //m_TileCellEnergy[ NumPart   ][ vecInd ]->Fill(energy, 1.);
              
            //m_TileCellEnergyRebin[ partition ][ vecInd ]->Fill(energy, 1.);
            //m_TileCellEnergyRebin[ NumPart   ][ vecInd ]->Fill(energy, 1.);
              
            //m_TileCellEneEta[ vecInd ]->Fill(eta, energy);
            //m_TileCellEnePhi[ vecInd ]->Fill(phi, energy);
            //m_TileCellEneEtaPhi[ vecInd ]->Fill(eta, phi, energy);
            m_TileCellEneEtaPhiSamp[ AllSamp ][ vecInd ]->Fill(eta, phi, energy);
    
            m_TileCellEneEtaPhiSamp[  samp ][ vecInd ]->Fill(eta, phi, energy);
            //m_TileCellOccModPartSamp[ samp ][ vecInd ]->Fill(module, fpartition, energy);
    
            // Fill occupancy histograms
            //m_TileCellOccModPart[ vecInd ]->Fill(module, fpartition, energy);
            //m_TileCellOccModPartSamp[ AllSamp ][ vecInd ]->Fill(module, fpartition, energy);
    
            if (ch1Ok) {
              m_TileCellDetailOccMap[ partition ][ vecInd ]->Fill(drawer, ch1, ene1 * weight);
            }        
            if (ch2Ok) {
              m_TileCellDetailOccMap[ partition2 ][ vecInd ]->Fill(drawer, ch2, ene2);
            }
          }
        } else {
          ++nBadCells[partition];
        }
        
        // check if energy is below negative threshold
        if (ch1Ok && ene1 < m_NegThreshold && (!badch1) ) {
          m_TileCellDetailNegOccMap[ partition ]->Fill(drawer, ch1, 1.0);
        }
        if (ch2Ok && ene2 < m_NegThreshold && (!badch2) ) {
          m_TileCellDetailNegOccMap[ partition2 ]->Fill(drawer, ch2, 1.0);
        }

        // check if energy is over threshold
        if  (energy>m_Threshold) {
          
          for (unsigned int i=0; i<m_eventTrigs.size(); i++) {
            int vecInd = vecIndx(i);
    
            //m_TileCellEtaPhiOvThr[ vecInd ]->Fill(eta, phi, 1.);
            m_TileCellEtaPhiOvThrSamp[AllSamp][ vecInd ]->Fill(eta, phi, 1.);
            //m_TileCellEtaOvThr[ vecInd ]->Fill(eta, 1.);
            //m_TileCellPhiOvThr[ vecInd ]->Fill(phi, 1.);
    
            m_TileCellEtaPhiOvThrSamp[ samp ][ vecInd ]->Fill(eta, phi, 1.);
            //m_TileCellOccModPartOvThrSamp[ samp ][ vecInd ]->Fill(module, fpartition, 1.);
            
            //if ((ene1 > m_ThresholdForTime || ene2 > m_ThresholdForTime) && 
            //   eratio<0.2 && time!=0.0 && 
            //   (m_iscoll || m_eventTrigs[i]==Trig_b7)) {//new cut-either pmt over threshold and ediff/ene<0.2 
              // Here we don't take into account negative energies on purpose.
              // Negative energies channels should be monitored independently
              //m_TileCellTime[ partition ][ vecInd ]->Fill(time, 1.);
            //}
    
            // Fill channel timing histograms.
            if ((m_iscoll || m_eventTrigs[i]==Trig_b7)) {
    
              if (ch1Ok && ene1 > m_ThresholdForTime) {
                //m_TileChanDetTime [ vecInd ]->Fill(t1,1.);
                m_TileChanPartTime[ partition ][ vecInd ]->Fill(drawer, ch1, t1, 1.);
                m_TileDigiPartTime[ partition ][ vecInd ]->Fill(drawer, ch2digi[ch1], t1, 1.);
                //m_TileChanModTime [ partition ][ vecInd ]->Fill(drawer, t1, 1.);
                //m_TileChanChTime  [ partition ][ drw ][ vecInd ]->Fill(ch1, t1, 1.);

                 if(m_doOnline) {
                  m_delta_lumiblock = current_lumiblock - m_OldLumiArray2[partition][drw][vecInd];

                  if(m_delta_lumiblock != 0) {//move bins
                    ShiftLumiHist(m_TileDigiEnergyLB[partition][drw][vecInd], m_delta_lumiblock);
                    m_OldLumiArray2[partition][drw][vecInd] = current_lumiblock;
                  }

                  m_TileDigiEnergyLB[partition][drw][vecInd]->Fill(0,ch2digi[ch1],ene1,1.);
                  m_delta_lumiblock = current_lumiblock - m_OldLumiArray1[partition][drw][vecInd];

                  if(m_delta_lumiblock != 0) {//move bins
                    ShiftLumiHist(m_TileDigiTimeLB[partition][drw][vecInd], m_delta_lumiblock);
                    m_OldLumiArray1[partition][drw][vecInd] = current_lumiblock;
                  }

                  m_TileDigiTimeLB[partition][drw][vecInd]->Fill(0,ch2digi[ch1],t1,1.);

                } else {// End of Online

                  m_TileDigiEnergyLB[partition][drw][vecInd]->Fill(current_lumiblock,ch2digi[ch1],ene1,1.);
                  m_TileDigiTimeLB[partition][drw][vecInd]->Fill(current_lumiblock,ch2digi[ch1],t1,1.);
                }

              }
    
              if (ch2Ok && ene2 > m_ThresholdForTime) {
                //m_TileChanDetTime [ vecInd ]->Fill(t2, 1.);
                m_TileChanPartTime[ partition2 ][ vecInd ]->Fill(drawer, ch2, t2, 1.);
                m_TileDigiPartTime[ partition2 ][ vecInd ]->Fill(drawer, ch2digi[ch2], t2, 1.);
                //m_TileChanModTime [ partition2 ][ vecInd ]->Fill(drawer, t2, 1.);
                //m_TileChanChTime  [ partition2 ][ drw ][ vecInd ]->Fill(ch2, t2, 1.);

                if(m_doOnline) {
                  m_delta_lumiblock = current_lumiblock - m_OldLumiArray2[partition][drw][vecInd];

                  if(m_delta_lumiblock != 0) {//move bins
                    ShiftLumiHist(m_TileDigiEnergyLB[partition][drw][vecInd], m_delta_lumiblock);
                    m_OldLumiArray2[partition][drw][vecInd] = current_lumiblock;
                  }

                  m_TileDigiEnergyLB[partition][drw][vecInd]->Fill(0,ch2digi[ch2],ene2,1.);
                  m_delta_lumiblock = current_lumiblock - m_OldLumiArray1[partition][drw][vecInd];

                  if(m_delta_lumiblock != 0) {//move bins
                    ShiftLumiHist(m_TileDigiTimeLB[partition][drw][vecInd], m_delta_lumiblock);
                    m_OldLumiArray1[partition][drw][vecInd] = current_lumiblock;
                  }

                  m_TileDigiTimeLB[partition][drw][vecInd]->Fill(0,ch2digi[ch2],t2,1.);

                } else {// End of Online

                  m_TileDigiEnergyLB[partition][drw][vecInd]->Fill(current_lumiblock,ch2digi[ch1],ene2,1.);
                  m_TileDigiTimeLB[partition][drw][vecInd]->Fill(current_lumiblock,ch2digi[ch1],t2,1.);
                }

              }
            }
    
            // store the occupancy for number hits over threshold
            //m_TileCellOccModPartOvThr[ vecInd ]->Fill(module, fpartition, 1.);// we fill only events in a precise energy region
            //m_TileCellOccModPartOvThrSamp[ AllSamp ][ vecInd ]->Fill(module, fpartition, 1.);
    

            if (ch1Ok && ene1 > m_Threshold && (!badch1)) {
          
              if (gn1 == 1) m_TileCellDetailOccMapOvThr[partition][vecInd]->Fill(drawer, ch1, weight);
              else m_TileCellDetailOccMapLowGainOvThr[partition][vecInd]->Fill(drawer, ch1, weight);
              if (ene1 > 30000.) m_TileCellDetailOccMapOvThr30GeV[partition][vecInd]->Fill(drawer, ch1);
              if (ene1 > 300000.) m_TileCellDetailOccMapOvThr300GeV[partition][vecInd]->Fill(drawer, ch1);
            }
            
            if (ch2Ok && ene2 > m_Threshold && (!badch2)) {
              
              if (gn2 == 1) m_TileCellDetailOccMapOvThr[partition2][vecInd]->Fill(drawer, ch2, 1.0);
              else m_TileCellDetailOccMapLowGainOvThr[partition2][vecInd]->Fill(drawer, ch2, 1.0);
              if (ene2 > 30000.) m_TileCellDetailOccMapOvThr30GeV[partition][vecInd]->Fill(drawer, ch2);
              if (ene2 > 300000.) m_TileCellDetailOccMapOvThr300GeV[partition][vecInd]->Fill(drawer, ch2);
            }
            
          } // end loop over TriggerType
          
          bool fillEneAndTimeDiff(true);
          
          // avoid double peak structure in energy and time balance histograms
          if ((gn1 == 0 && gn2 == 1 && (ene1 < 2000 || std::abs(ene1 / ene2) > 5))
              || (gn1 == 1 && gn2 == 0 && (ene2 < 2000 || std::abs(ene2 / ene1) > 5))) {
            
            fillEneAndTimeDiff = false;
          }
          
          
          // check if cell is not completely bad
          if (cell_isbad < 2) {
            
            for (unsigned int i = 0; i < m_eventTrigs.size(); i++) {
              int vecInd = vecIndx(i);
              if (samp != 3 && fillEneAndTimeDiff){  //Don't make samp E
                //m_TileCellEneRatSamp [ partition ][ samp ][ vecInd ]->Fill(eratio, 1.);
                m_TileCellEneDiffSamp[ partition ][ samp ][ vecInd ]->Fill(ediff, 1.);
                //m_TileCellEneRatSamp [ NumPart   ][ samp ][ vecInd ]->Fill(eratio, 1.);
                m_TileCellEneDiffSamp[ NumPart   ][ samp ][ vecInd ]->Fill(ediff, 1.);
                
                if ((ene1 > m_ThresholdForTime
                     || ene2 > m_ThresholdForTime)
                    && (m_iscoll || m_eventTrigs[i]==Trig_b7)) {        
                  
                  m_TileCellTimeDiffSamp[ partition ][ samp ][ vecInd ]->Fill(tdiff, 1.);
                  m_TileCellTimeDiffSamp[ NumPart   ][ samp ][ vecInd ]->Fill(tdiff, 1.);
                }
                
              }
            }
            
            if (cell_isbad<1 && (! single_PMT) ) { // we fill these histograms only if the both PMTs are good
      
              m_TileCellEneBal[ partition ]->Fill(module, eratio);

              if ((ene1 > m_ThresholdForTime || ene2 > m_ThresholdForTime)
                  && m_iscoll ) {
                
                m_TileCellTimBal[ partition ]->Fill(module, tdiff);
              }


              
              for (unsigned int i = 0; i < m_eventTrigs.size(); i++) {
                int vecInd = vecIndx(i);
                
                if (fillEneAndTimeDiff) {
                  m_TileCellEneDiffSamp[ partition ][ AllSamp ][ vecInd ]->Fill(ediff, 1.);
                  // also store the energy ratio diff/energy
                  //m_TileCellEneRatSamp [ partition ][ AllSamp ][ vecInd ]->Fill(eratio, 1.);
                  
                  m_TileCellEneDiffSamp[ NumPart ][ AllSamp ][ vecInd ]->Fill(ediff, 1.);
                  // also store the energy ratio diff/energy
                  //m_TileCellEneRatSamp [ NumPart ][ AllSamp ][ vecInd ]->Fill(eratio, 1.);
                  if ((ene1 > m_ThresholdForTime || ene2 > m_ThresholdForTime)
                      && (m_iscoll || m_eventTrigs[i]==Trig_b7)) {        
                    
                    m_TileCellTimeDiffSamp[ NumPart  ][ AllSamp ][ vecInd ]->Fill(tdiff, 1.);
                    m_TileCellTimeDiffSamp[ partition][ AllSamp ][ vecInd ]->Fill(tdiff, 1.);
                  }        
                }            
            
                // check if the energy or timing balance is out of range
                if (TMath::Abs(eratio) > m_EneBalThreshold) 
                  m_TileCellEneBalModPart[ vecInd ]->Fill(module, fpartition, 1.0);
                if (TMath::Abs(tdiff) > m_TimBalThreshold 
                    && (ene1 > m_ThresholdForTime || ene2 > m_ThresholdForTime)
                    && (m_iscoll || m_eventTrigs[i]==Trig_b7)) {        
                  
                  m_TileCellTimBalModPart[ vecInd ]->Fill(module, fpartition, 1.0);
                }
                
                m_TileCellEneDiffChanMod[partition][vecInd]->Fill(drawer, ch1, ediff);
                m_TileCellEneDiffChanMod[partition2][vecInd]->Fill(drawer, ch2, -1 * ediff);
                
              }
              
              modulecorr[partition].inputxy( m_tileID->module(id));
              modulecorr[NumPart].inputxy( m_tileID->module(id));

            } // end if cell has two pmts and both pmts are good
    
          } // end if cell is good
    
          //store info for BCID plots. We count the number of cells over threshold per partition
          ++cellcnt[partition];
    
        } //end if energy > threshold

      } //end if real cell

      // count total number of cells in a partition
      ++nCells[partition];

      // accumulate total energy per sample per partition
      eTileSamp[partition][samp] += energy;

    } // end if tile_cell
  } // end of loop over the Cells                        

  // Calculate totals for all samples and all partitions
  for (int partition=0; partition<NumPart; partition++) {

    badonfly[NumPart] += badonfly[partition];
    cellcnt[NumPart] += cellcnt[partition];
    nCells[NumPart] += nCells[partition];
    nBadCells[NumPart] += nBadCells[partition];

    m_TileBadCell->Fill(partition,nBadCells[partition]);
  
    for (int samp=0; samp<AllSamp; samp++) {
      eTileSamp[partition][AllSamp] += eTileSamp[partition][samp];
      eTileSamp[ NumPart ][ samp  ] += eTileSamp[partition][samp];
    }
    eTileSamp[NumPart][AllSamp] += eTileSamp[partition][AllSamp];
  }

  if (msgLvl( MSG::DEBUG)) {
    msg(MSG::DEBUG) << "nCells:    " << nCells[0] << " " << nCells[1] << " " << nCells[2] << " " << nCells[3] << " " << nCells[4] << endmsg;
    if (nCells[4] > 0) {
      msg(MSG::DEBUG) << "aboveThr:  " << cellcnt[0] << " " << cellcnt[1] << " " << cellcnt[2] << " " << cellcnt[3] << " " << cellcnt[4] << endmsg;
      msg(MSG::DEBUG) << "maskOnFly: " << badonfly[0] << " " << badonfly[1] << " " << badonfly[2] << " " << badonfly[3] << " " << badonfly[4] << endmsg;

      msg(MSG::DEBUG) << "EneSampA: " << eTileSamp[0][SampA] << " " << eTileSamp[1][SampA] << " " << eTileSamp[2][SampA] << " " << eTileSamp[3][SampA] << " " << eTileSamp[4][SampA] << endmsg;
      msg(MSG::DEBUG) << "EneSampB: " << eTileSamp[0][SampB] << " " << eTileSamp[1][SampB] << " " << eTileSamp[2][SampB] << " " << eTileSamp[3][SampB] << " " << eTileSamp[4][SampB] << endmsg;
      msg(MSG::DEBUG) << "EneSampD: " << eTileSamp[0][SampD] << " " << eTileSamp[1][SampD] << " " << eTileSamp[2][SampD] << " " << eTileSamp[3][SampD] << " " << eTileSamp[4][SampD] << endmsg;
      msg(MSG::DEBUG) << "EneSampE: " << eTileSamp[0][SampE] << " " << eTileSamp[1][SampE] << " " << eTileSamp[2][SampE] << " " << eTileSamp[3][SampE] << " " << eTileSamp[4][SampE] << endmsg;
      msg(MSG::DEBUG) << "EneTotal: " << eTileSamp[0][AllSamp] << " " << eTileSamp[1][AllSamp] << " " << eTileSamp[2][AllSamp] << " " << eTileSamp[3][AllSamp] << " " << eTileSamp[4][AllSamp] << endmsg;
    }
  }
  
  // Start filling summary histograms

  // Fill BCID plots
  for (unsigned int i = 0; i < m_eventTrigs.size(); i++) {
    int vecInd = vecIndx(i);
    //m_TileEventsPerBCID[ vecInd ]->Fill(bcid, 1.);
    for (int partition = 0; partition < NPartHisto; partition++) {

      float ene = eTileSamp[partition][AllSamp] - eTileSamp[partition][SampE];  //Total Energy in partition without E sample

      m_TileCellOccOvThrBCID[ partition ][ vecInd ]->Fill(bcid, cellcnt[partition]);
      //m_TileCellOccEneBCID  [ partition ][ vecInd ]->Fill(bcid, ene);

      if (partition != NumPart) {m_TileCellEvEneTim    [ partition ][ vecInd ]->Fill(evtNum, ene);} //don't make AllPart 
      if (partition != NumPart) {m_TileCellEvEneLumi   [ partition ][ vecInd ]->Fill(lumi,   ene);} //don't make AllPart
      //m_TileCellEvEneRebin  [ partition ][ vecInd ]->Fill(ene,    1.0);

      //for (int samp=0; samp<AllSamp; samp++) {
        m_TileCellEvEneSamp [ partition ][ 3 ][ vecInd ]->Fill(eTileSamp[partition][3], 1.0); //Changed samp to 3, only make Samp E
     // }

      //if (partition != NumPart) { // sum over samples in one partition includes E sample
      //  m_TileCellEvEneSamp [ partition ][ AllSamp ][ vecInd ]->Fill(eTileSamp[partition][AllSamp], 1.0);
      //} else {  // sum over samples in whole TileCal exccludes E sample
      //  m_TileCellEvEneSamp [ partition ][ AllSamp ][ vecInd ]->Fill(ene, 1.0);
      //}
    }
  }

  // Fill histograms per LB
  static uint32_t old_lumi;
  static int f_first_lb = 1;
  if (f_first_lb)
    old_lumi = lumi;


  for (int partition = 0; partition < NPartHisto; partition++) {
    for (int j = 0; j < modulecorr[partition].numberOfPairs(); j++) {
      for (unsigned int i = 0; i < m_eventTrigs.size(); i++) {
        int vecInd = vecIndx(i);

        m_TileCellModuleCorr[ partition ][ vecInd ]->Fill(modulecorr[partition].xOfPairAt(j),                
                                                          modulecorr[partition].yOfPairAt(j),
                                                          modulecorr[partition].weight()   );
      }
    }

    for (unsigned int i = 0; i < m_eventTrigs.size(); i++) {
      int vecInd = vecIndx(i);

      if (m_doOnline) {
        if (old_lumi < lumi) { // we need move bins
          int bin;
          int delta_lb = lumi - old_lumi;
          double total_entries = 0.;

          for (bin = 1; bin <= 100 - delta_lb; bin++) {
            double entries = m_TilenCellsLB[ partition ][ vecInd ]->GetBinEntries(bin + delta_lb);
            double content = m_TilenCellsLB[ partition ][ vecInd ]->GetBinContent(bin + delta_lb);
            double error   = m_TilenCellsLB[ partition ][ vecInd ]->GetBinError(bin + delta_lb);
            total_entries += entries;
            m_TilenCellsLB[ partition ][ vecInd ]->SetBinEntries(bin, entries);
            m_TilenCellsLB[ partition ][ vecInd ]->SetBinContent(bin, content * entries);
            m_TilenCellsLB[ partition ][ vecInd ]->SetBinError(bin, sqrt((error * error * entries + content * content) * entries));
          }

          for (; bin <= 100; bin++) { // set rest to zero
            m_TilenCellsLB[ partition ][ vecInd ]->SetBinEntries(bin, 0.);
            m_TilenCellsLB[ partition ][ vecInd ]->SetBinContent(bin, 0.);
            m_TilenCellsLB[ partition ][ vecInd ]->SetBinError(bin,   0.);
          }

          m_TilenCellsLB[ partition ][ vecInd ]->SetEntries(total_entries);
        } // end of move bins
        m_TilenCellsLB[ partition ][ vecInd ]->Fill(0., nCells[partition]);
      } else {// End Of Online

        m_TilenCellsLB[ partition ][ vecInd ]->Fill(lumi, nCells[partition]);
      }
      //m_TilenCells[ partition ][ vecInd ]->Fill(nCells[partition], 1.);
    }
  }

  // number of masked channels on the fly in a partition

  if (m_doOnline) {
    for (int p = 0; p < NPartHisto; p++) {
      if (old_lumi < lumi) { // we need move bins
        int delta_lb = lumi - old_lumi;
        ShiftTprofile(m_TileMaskChannonFlyLumi[p], delta_lb);
        ShiftTprofile(m_TileMaskCellonFlyLumi[p], delta_lb);
      }

      m_TileMaskChannonFlyLumi[p]->Fill(0., badonfly[p]);
      m_TileMaskCellonFlyLumi[p]->Fill(0., nBadCells[p]);
    }
  } else {
    for (int p = 0; p < NPartHisto; p++) {
      m_TileMaskChannonFlyLumi[p]->Fill(lumi, badonfly[p]);
      m_TileMaskCellonFlyLumi[p]->Fill(lumi, nBadCells[p]);
    }
  }


  // Fill histograms of masked channels per LB from DB
  if (old_lumi < lumi || f_first_lb) {
    f_first_lb = 0;
    old_lumi = lumi;
    short nAdcAll = 0;
    for (int p = 0; p < NumPart; p++) {
      short nAdc = nAdcBad(p);
      m_TileMaskChannfromDBLumi[p]->Fill(lumi, nAdc);
      nAdcAll += nAdc;
    }
    m_TileMaskChannfromDBLumi[NumPart]->Fill(lumi, nAdcAll);
  }

  //Fill synchronization plots
  calculateSynch();

  // Set number of events as entries
  for (unsigned int i = 0; i < m_eventTrigs.size(); ++i) {

    ++m_nEventsProcessed[m_eventTrigs[i]];
    int nEventsPerTrig(m_nEventsProcessed[m_eventTrigs[i]]);

    int vecInd = vecIndx(i);

    for (int sample = 0; sample < TotalSamp; ++sample) {
      m_TileCellEneEtaPhiSamp[sample][vecInd]->SetEntries(nEventsPerTrig);    
    }

    for (int partition = 0; partition < NumPart; ++partition) {
      m_TileCellDetailOccMap[partition][vecInd]->SetEntries(nEventsPerTrig);
    }
  }


  return StatusCode::SUCCESS;
}

/*---------------------------------------------------------*/
StatusCode TileCellMonTool::procHistograms() {
/*---------------------------------------------------------*/

  if( endOfLumiBlock ||  endOfRun ) {
    ATH_MSG_INFO( "in procHistograms()" );
  }

  return StatusCode::SUCCESS;
}

/*---------------------------------------------------------*/
StatusCode TileCellMonTool::checkHists(bool /* fromFinalize */) {
/*---------------------------------------------------------*/

  ATH_MSG_INFO( "in checkHists()" );

  return StatusCode::SUCCESS;
}

/*---------------------------------------------------------*/
void TileCellMonTool::calculateSynch( ) {
/*---------------------------------------------------------*/

  int size = m_muonCells.size();

  for (int j = 1; j < size; j++) {

    float x_j = m_muonCells[j]->x();
    float y_j = m_muonCells[j]->y();
    float z_j = m_muonCells[j]->z();
    float t_j = m_muonCells[j]->time();

    for (int l = 0; l < j; l++) {

      float x_l = m_muonCells[l]->x();
      float y_l = m_muonCells[l]->y();
      float z_l = m_muonCells[l]->z();
      float t_l = m_muonCells[l]->time();

      float dc = sqrt( (x_j - x_l) * (x_j - x_l) + (y_j - y_l) * (y_j - y_l) + (z_j - z_l) * (z_j - z_l));

      float tc = t_l - t_j;

      if (y_l > y_j) {
        tc = -tc;
      }

      float te = dc / 300.;
      float td = te - tc;

      for (unsigned int i = 0; i < m_eventTrigs.size(); i++) {
        m_TileCellSynch[vecIndx(i)]->Fill(td);
      }
    }
  }

  for (unsigned int i = 0; i < m_eventTrigs.size(); i++) {
    m_TileCellSynch[vecIndx(i)]->SetEntries(m_nEvents); // Number of events Reference
  }

}

short TileCellMonTool::isCellBad(const TileCell* tile_cell) {

  // taking channel status from DP, ignore masking on the fly
  short result(0);

  const CaloDetDescrElement * caloDDE = tile_cell->caloDDE();

  IdentifierHash hash1 = caloDDE->onl1();
  if (hash1 != TileHWID::NOT_VALID_HASH) {
    HWIdentifier hw = m_tileHWID->channel_id(hash1);
    TileBchStatus status = m_tileBadChanTool->getAdcStatus(hw);
    if (status.isBad()) ++result;
  } else {
    result = 1; // something is wrong, can not be like this
  }

  IdentifierHash hash2 = caloDDE->onl2();
  if (hash2 != TileHWID::NOT_VALID_HASH) {
    HWIdentifier hw = m_tileHWID->channel_id(hash2);
    TileBchStatus status = m_tileBadChanTool->getAdcStatus(hw);
    if (status.isBad()) ++result;
  } else if (result) {
    result = 2; // for single-channel cell make result 0 or 2
  }

  return result;
}

short TileCellMonTool::isAdcBad(int partition, int module, int channel, int gain) {

  unsigned int idx = TileCalibUtils::getDrawerIdx(m_partition2ros[partition], module - 1);
  TileBchStatus status = m_tileBadChanTool->getAdcStatus(idx, channel, gain);

  return ((status.isBad()) ? 1 : 0);
}

short TileCellMonTool::nAdcBad(int partition) {
  short result = 0;

  for (int drawer = 0; drawer < 64; drawer++) {
    unsigned int idx = TileCalibUtils::getDrawerIdx(m_partition2ros[partition], drawer);
    for (int channel = 0; channel < 48; channel++) {
      for (int gain = 0; gain < 2; gain++) {
        TileBchStatus status = m_tileBadChanTool->getAdcStatus(idx, channel, gain);
        if (status.isBad()) ++result;
      }
    }
  }

  return result;
}


// Operations to be done only once at the first event
/*---------------------------------------------------------*/
void TileCellMonTool::FirstEvInit() {
/*---------------------------------------------------------*/

  m_isFirstEv = false; //Set the flag

  if ( bookHistTrig( AnyTrig ).isFailure() ) {
    ATH_MSG_WARNING( "Error booking Cell histograms for Trigger " << m_TrigNames[ AnyTrig ] );
  }        

  std::ostringstream sene; sene.str("");
  sene << m_NegThreshold / 1000.0;

  std::string runNumStr =  getRunNumStr() ;

  ////////////////////////// Book Profile Histograms as a function of the module for energy and time imbalance

  for (int p = 0; p < NumPart; p++) {

    m_TileCellEneBal[p] = bookProfile("", "tileCellEneBal" + m_PartNames[p]
                                      , "Run " + runNumStr + " Partition " + m_PartNames[p] + ": cell PMT Energy Balance"
                                      , 64, 0.5, 64.5, -5000., 5000.);
    m_TileCellEneBal[p]->SetYTitle("Energy balance between cells PMTs (u-d)/(u+d)");
    m_TileCellEneBal[p]->SetXTitle("Module");

    m_TileCellTimBal[p] = bookProfile("", "tileCellTimBal" + m_PartNames[p]
                                       , "Run " + runNumStr + " Partition " + m_PartNames[p] + ": Cell's PMTs Time Difference. Collision Events, either E_{ch} > " + TC::to_string(m_ThresholdForTime) + " MeV"
                                       , 64, 0.5, 64.5, -200., 200.);
    m_TileCellTimBal[p]->SetYTitle("Time balance between cells PMTs (ns)");
    m_TileCellTimBal[p]->SetXTitle("Module");
  }
  
  ////////////////////////// Book Histograms with the chanel status from the DB and masking on the fly

  for (int p = 0; p < NumPart; p++) {
    for (int g = 0; g < 2; g++) { //gain = LowGain, HighGain
      m_TileCellStatFromDB[p][g] = book2S("", "tileCellStatfromDB_" + m_PartNames[p] + "_" + m_GainNames[g]
                                          , "Run " + runNumStr + " Partition " + m_PartNames[p] + " " + m_GainNames[g] + ": Cell channel status in DB"
                                          , 64, 0.5, 64.5, 48, -0.5, 47.5);
      //m_TileCellStatFromDB[p][g]->SetXTitle("Module");      m_TileCellStatFromDB[p][g]->SetYTitle("Channel");
      SetBinLabel(m_TileCellStatFromDB[p][g]->GetYaxis(), m_cellchLabel[p]);
      SetBinLabel(m_TileCellStatFromDB[p][g]->GetXaxis(), m_moduleLabel[p]);
      // setting average bit for histogram, but apparently it doesn't work ... need to investigate
      m_TileCellStatFromDB[p][g]->SetBit(TH1::kIsAverage);
    } 

    m_TileCellStatOnFly[p] = book2F("", "tileCellStatOnFly_" + m_PartNames[p]
                                    , "Run " + runNumStr + " Partition " + m_PartNames[p] + ": Channels masked on the fly"
                                    , 64, 0.5, 64.5, 48, -0.5, 47.5);
    SetBinLabel(m_TileCellStatOnFly[p]->GetYaxis(), m_cellchLabel[p]);
    SetBinLabel(m_TileCellStatOnFly[p]->GetXaxis(), m_moduleLabel[p]);

    m_TileCellDetailNegOccMap[p] = book2F("", "tileCellDetailNegOccMap_" + m_PartNames[p]
                                          , "Run " + runNumStr + " Partition " + m_PartNames[p] + ": Occupancy Map Below Negative Threshold " + sene.str() + " GeV"
                                          , 64, 0.5, 64.5, 48, -0.5, 47.5);
    SetBinLabel(m_TileCellDetailNegOccMap[p]->GetYaxis(), m_cellchLabel[p]);
    SetBinLabel(m_TileCellDetailNegOccMap[p]->GetXaxis(), m_moduleLabel[p]);
  }
  
  ////////////////////////// Book Histograms with the chanel status as a function of lumi block

  for (int p = 0; p < NPartHisto; p++) {

    if (m_doOnline) {
      m_TileMaskChannonFlyLumi[p] = bookProfile("", "tileMaskChannOnFlyLumi_" + m_PartNames[p]
                                                , "Run " + runNumStr + " Partition " + m_PartNames[p] + ": Number of masked channels on the fly"
                                                , 100, -99.5, 0.5);
      m_TileMaskChannonFlyLumi[p]->SetYTitle("Number of masked channels");
      m_TileMaskChannonFlyLumi[p]->SetXTitle("Last LumiBlocks");

      ////////////////////////// Book Histograms with the cell status as a function of lumi block
      m_TileMaskCellonFlyLumi[p] = bookProfile("", "tileMaskCellOnFlyLumi_" + m_PartNames[p]
                                               , "Run " + runNumStr + " Partition " + m_PartNames[p] + ": Number of masked cells on the fly"
                                               , 100, -99.5, 0.5);
      m_TileMaskCellonFlyLumi[p]->SetYTitle("Number of masked cells");
      m_TileMaskCellonFlyLumi[p]->SetXTitle("Last LumiBlocks");

    } else {

      m_TileMaskChannonFlyLumi[p] = bookProfile("", "tileMaskChannOnFlyLumi_" + m_PartNames[p]
                                                , "Run " + runNumStr + " Partition " + m_PartNames[p] + ": Number of masked channels on the fly"
                                                , 1500, -0.5, 1499.5);
      m_TileMaskChannonFlyLumi[p]->SetYTitle("Number of masked channels");
      m_TileMaskChannonFlyLumi[p]->SetXTitle("LumiBlock");
      
      
      ////////////////////////// Book Histograms with the cell status as a function of lumi block
      m_TileMaskCellonFlyLumi[p] = bookProfile("", "tileMaskCellOnFlyLumi_" + m_PartNames[p]
                                               , "Run " + runNumStr + " Partition " + m_PartNames[p] + ": Number of masked cells on the fly"
                                               , 1500, -0.5, 1499.5);
      m_TileMaskCellonFlyLumi[p]->SetYTitle("Number of masked cells");
      m_TileMaskCellonFlyLumi[p]->SetXTitle("LumiBlock");
      
    }

    m_TileMaskChannfromDBLumi[p] = bookProfile("", "tileMaskChannfromDBLumi_" + m_PartNames[p]
                                               , "Run " + runNumStr + " Partition " + m_PartNames[p] + ": Number of masked channels in DB"
                                               , 1500, -0.5, 1499.5);
    m_TileMaskChannfromDBLumi[p]->SetYTitle("Number of masked channels");
    m_TileMaskChannfromDBLumi[p]->SetXTitle("LumiBlock");


  }

  m_TileBadCell = bookProfile("","tileBadCell"
                               , "Run " + runNumStr + ": Average number of Bad Cells"
                               , 4, -0.5, 3.5) ;
  SetBinLabel(m_TileBadCell->GetXaxis(), m_PartNames, 4);
  m_TileBadCell->SetYTitle("Number of bad cells");

  if  (!m_TileCellStatFromDB[0][0]) {
    ATH_MSG_ERROR( "Trying to fill histograms before they are booked" );
    ATH_MSG_ERROR( "Rectified for this algorithm, others monitoring algorithms might crash" );
  } else {

    short status = 0;
    for (int g = 0; g < 2; g++) {
      for (int p = 0; p < NumPart; p++) {
        for (int m = 1; m < 65; m++) {
          for (int ch = 0; ch < 48; ch++) {
            status = isAdcBad(p, m, ch, g);
            m_TileCellStatFromDB[p][g]->Fill(m, ch, status);
          }
        }
      } //close partition
    } //close gain

  } //close if

}

