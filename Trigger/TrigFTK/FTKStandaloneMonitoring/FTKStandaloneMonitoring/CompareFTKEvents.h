/* CompareFTKEvents class
 * author: Maddalena.Giulini@cern.ch
 * started: 27/02/2017
 *
 * Class to compare BS file and NTUP_FTK transformed file
 */
#ifndef __CompareFTKEvents__
#define __CompareFTKEvents__


#include <iostream>
#include <fstream>

#include <getopt.h>
#include <vector>
#include <cmath>
#include "TTree.h"
#include "TFile.h"


#include "ByteStreamData/RawEvent.h"
#include "TrigFTKSim/FTKTrackStream.h"
#include "TrigFTK_RawData/FTK_RawTrackContainer.h"
#include "TrigFTK_RawData/FTK_RawPixelCluster.h"
#include "TrigFTK_RawData/FTK_RawSCT_Cluster.h"
#include "FTKStandaloneMonitoring/FTKTrkAssoc.h"
#include "FTKStandaloneMonitoring/CompareFTKTracks.h"

class CompareFTKEvents{
  public:
    /*! \brief Constructor, does nothing
    */
    CompareFTKEvents();
    CompareFTKEvents(const std::string &BSfile, const std::string &NTUP_FTK_file);
    //~CompareFTKEvents();
    std::streampos readBSevent(int ievent,std::streampos startbufpos);
    void readNTUP_FTKfile();
    void PrintFiles();
    void EventLoop();
    void SetHistos(std::vector<std::string> histo_list);
    void SetEvtInfoString(std::string &str_tree_evtinfo){m_str_tree_evtinfo=str_tree_evtinfo;}
    void SetTreeNTUPString(std::string &str_tree_ftkdata){m_str_tree_ftkdata=str_tree_ftkdata;}
    void SetBranchString(std::string &str_tree_ftkstream){m_str_tree_ftkstream=str_tree_ftkstream;}
    void SetVerbose(){m_verbose=true;}
    void SetNEvents(int nevtmax){m_nevtmax=nevtmax;}
    void Execute();
    int GetNEventsBS();
    ~CompareFTKEvents();
  private:
    StatusCode decode(uint32_t nTracks, OFFLINE_FRAGMENTS_NAMESPACE::PointerType rodData, FTK_RawTrackContainer* result);
    //size_t decodeNumberOfTracks(OFFLINE_FRAGMENTS_NAMESPACE::PointerType rodData);
    FTK_RawTrack* unpackFTTrack( OFFLINE_FRAGMENTS_NAMESPACE::PointerType data);
    void unpackPixCluster(OFFLINE_FRAGMENTS_NAMESPACE::PointerType data, FTK_RawPixelCluster& cluster);
    void unpackSCTCluster(OFFLINE_FRAGMENTS_NAMESPACE::PointerType data, FTK_RawSCT_Cluster& cluster) { cluster.setWord(*data); }
    int m_nevtmax=0;
    const size_t TrackBlobSize = 22; // magic number from BS specification
    const size_t TrackParamsBlobSize = 6; // --||--
    const size_t PixHitParamsBlobSize = 2; // --||--
    const size_t SCTHitParamsBlobSize = 1; // --||--
    const size_t NPixLayers=4;
    const size_t NSCTLayers=8;
    std::vector<uint32_t> m_data;
    std::string m_BSfile;
    std::string m_NTUP_FTK_file;
    uint32_t m_checkword=0xaa1234aa;
    uint32_t trackBlockOffsetStart=0;	  // Nr of words in ROB data block before track blocks start
    uint32_t trackBlockOffsetEnd=0;	  // Nr of words in ROB data block after track blocks end
    int m_Nevents_BS;
    int m_Nevents_NTUPFTK;
    std::ifstream m_myBSfile;
    std::streampos m_tmpbufpos=0;
    TTree *m_tevtinfo;
    TTree *m_theTree;
    TFile *m_fntupftk;
    //gnamFTKEventFragment* ftkEF= new gnamFTKEventFragment();
    int RN, EN, LB, BCID, EL1ID, l1TT;
    FTKTrackStream *m_ft;
    bool m_allmatched=true;
    bool m_verbose=false;
    CompareFTKTracks * m_compTrk;
    std::vector<TH1D *> vec_histo;
    std::vector<std::string> m_histo_list;
    std::map<std::string , TH1D * > map_histo;
    std::map<std::string , TH2D * > map_histo_2D;
    TFile * m_fout;
    void CreateHistos();
    void WriteHistos();
    std::string m_str_tree_evtinfo="evtinfo";
    std::string m_str_tree_ftkdata="ftkdata";
    std::string m_str_tree_ftkstream="FTKMergedTracksStream";
    std::map<std::string, std::vector<double> > histo_param={{"pt",{1000.,0.,100000.}},
                                                              {"eta",{100.,-2.5,2.5}},
							      {"phi",{100.,-3.2,3.2}},
							      {"d0",{100.,-5.,5.}},
							      {"z0",{100,-50,50}},
							      {"chi2",{100,0,50}},
							      {"ETA_PHI",{100,-2.5,2.5,100,-3.2,3.2}}};
    std::vector<std::string> variable_list={"pt","eta","phi","d0","z0","chi2","ETA_PHI"};
  };
#endif //__CompareFTKEvents__
