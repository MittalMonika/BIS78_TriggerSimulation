/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SCT_CalibHitmapSvc.cxx
 * Implementation file for the SCT_CalibHitmapTool class
 * @author Shaun Roe
**/

#include "SCT_CalibHitmapTool.h"
#include "SCT_CalibUtilities.h"
#include "SCT_CalibNumbers.h"

// RAW data access
#include "InDetRawData/InDetRawDataCLASS_DEF.h"
#include "InDetRawData/SCT3_RawData.h"
#include "StoreGate/ReadHandle.h"

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "GaudiKernel/ITHistSvc.h"

#include "TH1I.h"
#include "TH1F.h"
#include "TFile.h"
#include "TFileCollection.h"
#include "THashList.h"

using namespace std;
using namespace SCT_CalibAlgs;
const static string pathRoot("/HitMaps/");
const static string detectorNames[] = { "negativeEndcap", "barrel", "positiveEndcap" };
const static string detectorPaths[]= {"SCTEC/", "SCTB/","SCTEA/"};

SCT_CalibHitmapTool::SCT_CalibHitmapTool(const std::string& type, const std::string& name, const IInterface* parent) :
   base_class(type, name, parent),
   m_detStore("DetectorStore", name),
   m_evtStore("StoreGateSvc", name),
   m_pSCTHelper(0),
   m_sct_waferHash(0),
   m_sct_firstStrip(0),
   m_sct_rdoGroupSize(0),
   m_rdoContainerKey(std::string("SCT_RDOs"))
{
}

StatusCode
SCT_CalibHitmapTool::initialize() {
   if ( m_detStore->retrieve( m_pSCTHelper, "SCT_ID").isFailure()) return msg( MSG::ERROR) << "Unable to retrieve SCTHelper" << endmsg, StatusCode::FAILURE;
   //
   m_waferItrBegin  = m_pSCTHelper->wafer_begin();
   m_waferItrEnd  = m_pSCTHelper->wafer_end();
   //

   // Read Handle Key
   ATH_CHECK(m_rdoContainerKey.initialize());

   return StatusCode::SUCCESS;
}

StatusCode
SCT_CalibHitmapTool::finalize() {
   msg(MSG::VERBOSE) << "SCT_CalibHitmapSvc::finalize()"<<endmsg;
   if (m_sct_waferHash) delete m_sct_waferHash;
   if (m_sct_rdoGroupSize) delete m_sct_rdoGroupSize;
   if (m_sct_firstStrip) delete m_sct_firstStrip;
   return StatusCode::SUCCESS;
}

StatusCode
SCT_CalibHitmapTool::queryInterface(const InterfaceID & riid, void** ppvInterface ) {
   if ( ISCT_CalibHistoTool::interfaceID().versionMatch(riid) ) {
      *ppvInterface = dynamic_cast<ISCT_CalibHistoTool*>(this);
   } else {
      return AthAlgTool::queryInterface(riid, ppvInterface);
   }
   addRef();
   return StatusCode::SUCCESS;
}

bool
SCT_CalibHitmapTool::book() {
   bool result(true);
   //pointers to the histos are deleted by m_thistSvc methods
   m_phistoVector.clear();
   string histoName=pathRoot+"GENERAL/";
   //histogram for numbers of events
   m_numberOfEventsHisto=new TH1I("events","Events",1,0.5,1.5);
   if( m_thistSvc->regHist( histoName.c_str(), m_numberOfEventsHisto ).isFailure() ) {
      msg( MSG::ERROR ) << "Error in booking EventNumber histogram" << endmsg;
   }
   //histograms for each wafer
   SCT_ID::const_id_iterator waferItr  = m_waferItrBegin;
   std::string hitmapPaths[3];
   for (int i(0); i<3; ++i) {
      hitmapPaths[i]=pathRoot+detectorPaths[i];
   }
   for( ; waferItr not_eq m_waferItrEnd; ++waferItr ) {
      const Identifier & waferId = *waferItr;
      const int bec( m_pSCTHelper->barrel_ec( waferId ) );
      const string formattedPosition=formatPosition(waferId, m_pSCTHelper);
      std::string histotitle = string( "SCT " ) + detectorNames[ bec2Index(bec) ] + string( " Hitmap: plane " ) + formattedPosition;
      std::string name=hitmapPaths[bec2Index(m_pSCTHelper->barrel_ec( waferId ))] + formattedPosition;
      TH1F* hitmapHisto_tmp = new TH1F( TString( formattedPosition ), TString( histotitle ), nbins, firstStrip-0.5, lastStrip+0.5 );

      //cout<<name.c_str()<<endl;
      if( m_thistSvc->regHist( name.c_str(), hitmapHisto_tmp ).isFailure()) {
         msg( MSG::ERROR ) << "Error in booking Hitmap histogram" << endmsg;
      } else {
         m_phistoVector.push_back( hitmapHisto_tmp );
      }

   }
   return result;
}

bool
SCT_CalibHitmapTool::read(const std::string & fileName) {
   bool result(true);
   //pointers to the histos are deleted by m_thistSvc methods
   m_phistoVector.clear();
   TFile *fileHitmap = TFile::Open( fileName.c_str() );
   msg( MSG::INFO ) << "opening Hitmap file : " << fileName.c_str() << endmsg;

   if(fileHitmap==NULL) {
      msg( MSG::ERROR ) << "can not open Hitmap file : " << fileName.c_str() << endmsg;
      return result;
   }
   //histogram for numbers of events
   m_numberOfEventsHisto = (TH1I*) fileHitmap->Get("GENERAL/events");
   if( m_numberOfEventsHisto==NULL ) {
      msg( MSG::ERROR ) << "Error in reading EventNumber histogram" << endmsg;
   }
   //histograms for each wafer
   SCT_ID::const_id_iterator waferItr  = m_waferItrBegin;
   for( ; waferItr not_eq m_waferItrEnd; ++waferItr ) {
      const Identifier & waferId = *waferItr;
      const string formattedPosition=formatPosition(waferId, m_pSCTHelper);
      std::string name=detectorPaths[bec2Index(m_pSCTHelper->barrel_ec( waferId ))] + formattedPosition;
      TH1F* hitmapHisto_tmp = (TH1F*) fileHitmap->Get(name.c_str());
      if( hitmapHisto_tmp==NULL ) {
         msg( MSG::ERROR ) << "Error in reading Hitmap histogram" << endmsg;
      } else {
         m_phistoVector.push_back( hitmapHisto_tmp );
      }
   }
   return result;
}

bool
SCT_CalibHitmapTool::fill(const bool fromData) {
   //cout<<"fromData "<<fromData<<endl;
   if (fromData) {
      return fillFromData();
   }
   bool result(true);
   //--- Number of events
   m_numberOfEventsHisto->Fill( 1 );
   // both ways hshould give the same results
   // int eventNumber = m_numberOfEventsHisto->GetBinContent(1);
   //  int eventNumber = m_numberOfEventsHisto->GetEntries();

   //--- Fill hitmap
   const int MaxEntry = m_sct_waferHash->size();
   for( int i = 0; i != MaxEntry; ++i ) {
      const int theFirstStrip = (*m_sct_firstStrip)[i];
      const int endStrip  = (*m_sct_rdoGroupSize)[i] + theFirstStrip;
      const int index         = (*m_sct_waferHash)[i];
      TH1F * pThisHisto=m_phistoVector[ index ];
      for( int strip(theFirstStrip); strip !=endStrip; ++strip) {
         pThisHisto->Fill( strip);
      }
   }
   return result;
}

bool
SCT_CalibHitmapTool::fillFromData() {
   bool result(true);
   m_numberOfEventsHisto->Fill( 1 );
   // unused int eventNumber = m_numberOfEventsHisto->GetEntries();
   SG::ReadHandle<SCT_RDO_Container> prdoContainer(m_rdoContainerKey);
   if (not prdoContainer.isValid() ) msg(MSG::ERROR) <<"Failed to retrieve the SCT RDO container"<<endmsg;
   SCT_RDO_Container::const_iterator itr=prdoContainer->begin();
   const SCT_RDO_Container::const_iterator end=prdoContainer->end();
   for (; itr !=end; ++itr) {
      const InDetRawDataCollection<SCT_RDORawData>* SCT_Collection(*itr);
      if (not SCT_Collection) continue;
      const Identifier waferId = SCT_Collection->identify();
      const IdentifierHash waferHash = m_pSCTHelper->wafer_hash(waferId);
      TH1F * pThisHisto =m_phistoVector[(int)waferHash];
      DataVector<SCT_RDORawData>::const_iterator rdoItr = SCT_Collection->begin();
      const DataVector<SCT_RDORawData>::const_iterator rdoEnd = SCT_Collection->end();
      for(; rdoItr != rdoEnd; ++rdoItr) {
         int strip=m_pSCTHelper->strip((*rdoItr)->identify());
         const int endStrip=(*rdoItr)->getGroupSize() + strip;
         for (; strip != endStrip; ++strip) {
            pThisHisto->Fill(strip);
         }
      }
   }
   return result;
}
