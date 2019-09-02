/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_MUONTRUTHSUMMARYTOOL_H
#define MUON_MUONTRUTHSUMMARYTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonRecToolInterfaces/IMuonTruthSummaryTool.h"
#include "MuonIdHelpers/MuonIdHelperTool.h"
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"
#include "MuonRecHelperTools/IMuonEDMHelperSvc.h"
#include "TrkTruthData/PRD_MultiTruthCollection.h"
#include "GaudiKernel/IIncidentListener.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include <string>
#include <set>
#include <map>

namespace Trk {
  class Track;
  class MeasurementBase;
}

class IIncidentSvc;
class ITHistSvc;
class TTree;

namespace Muon {
  class MuonSegment;

  /** Interface for tools calculating hit count summaries for track */
  class MuonTruthSummaryTool : public AthAlgTool, virtual public IMuonTruthSummaryTool, virtual public IIncidentListener {      
  public:
    //** Constructor with parameters */
    MuonTruthSummaryTool( const std::string& t, const std::string& n, const IInterface* p );
 
    // Athena Hooks
    StatusCode  initialize();
    StatusCode  finalize();

    /** clear tool */
    void clear();

    /** init truth */
    void init() const;

    /** get the associated barcode for the identifier, return -1 if the channel was not hit by a muon */
    int getBarcode( const Identifier& id ) const ;

    /** get the associated pdgId for a given barcode */
    int getPdgId( int barcode ) const;

    /** add identifier */
    void add( const Identifier& id, int level );

    /** add segment */
    void add( const MuonSegment& seg, int level );

    /** add track */
    void add( const Trk::Track& track, int level );
    

    /** print summary */
    std::string printSummary() ;

    void handle(const Incident& inc);

  private:

    /** add measurements */
    void add( const std::vector<const Trk::MeasurementBase*>& measurements, int level );
    void getTruth() const;
    std::string printSummary( const std::set<Identifier>& truth, const std::set<Identifier>& found );

    ToolHandle<MuonIdHelperTool>                m_idHelper;
    ServiceHandle<IMuonEDMHelperSvc>            m_edmHelperSvc {this, "edmHelper", 
      "Muon::MuonEDMHelperSvc/MuonEDMHelperSvc", 
      "Handle to the service providing the IMuonEDMHelperSvc interface" };
    ToolHandle<MuonEDMPrinterTool>              m_printer;
    ServiceHandle< IIncidentSvc >               m_incidentSvc;
    mutable bool m_wasInit;
    bool m_useNSW;

    SG::ReadHandleKeyArray<PRD_MultiTruthCollection> m_TruthNames{this,"TruthNames",{"RPC_TruthMap","TGC_TruthMap","MDT_TruthMap"},"truth names"};

    mutable std::map<int,int>                           m_pdgIdLookupFromBarcode;
    mutable std::map<Identifier,int>                    m_truthHits; // map containing truth hits associated with muons, stores barcode as second element
    mutable std::map<int,std::set<Identifier> >         m_truthDataPerLevel;
    mutable std::map<int,unsigned int >         m_lossesPerLevel; // We can get rid of the mutable, once printSummary() isn't const.
    mutable unsigned int                        m_truthHitsTotal;
    
    TTree *                                     m_tree;
    ITHistSvc *                                 m_thistSvc;
    bool                                        m_writeTree; // Set to true in order to write out ntuple
    std::string                                 m_treeName;
    std::string                                 m_histStream;
    unsigned int                                m_level;
    int                                         m_selectedPdgId;
    
    std::vector< unsigned int           >       m_numChambers;
    std::vector< std::vector<uint8_t>*  >       m_numMissedHits; 
    std::vector< std::vector<uint8_t>*  >       m_missedHitTechnologyIndex; // [0..5] maps to "MDT", "CSC", "RPC", "TGC", "STGC", "MM"
    std::vector< std::vector<int>    *  >       m_missedHitStationPhi;      // increases with eta (0 at eta=0)
    std::vector< std::vector<int>    *  >       m_missedHitStationSector;      // increases with eta (0 at eta=0)
    std::vector< std::vector<int>    *  >       m_missedHitStationEta;      // increases with phi
    // std::vector< std::vector<float>    *  >     m_missedHitR; // Can't do these without re-writing code a bit to have link to Meas. Not sure it's worth it.
    // std::vector< std::vector<float>    *  >     m_missedHitZ;   
    // std::vector< std::vector<std::string>* >         m_missedHitStationName;     // "BIL", "EMS", "T1F", etc.
    std::vector< std::vector<int>* >            m_missedHitStationNameIndex;   // BI=0 , BM=1, BO=2, BE=3,
      
    void fillChamberVariables(const Identifier& chamberId, const unsigned int numMissedHits);
    void clearChamberVariables(const unsigned int level);
    void initChamberVariables(const unsigned int levels);
  };
  
} // end of name space

#endif

