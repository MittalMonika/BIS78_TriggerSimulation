/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonSegmentOverlapRemovalTool.h"
 
#include "GaudiKernel/MsgStream.h"
#include "MuonIdHelpers/MuonIdHelperTool.h"
#include "MuonRecHelperTools/MuonEDMHelperTool.h"
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"

#include "MuonSegment/MuonSegment.h"
#include "MuonSegment/MuonSegmentCombination.h"

#include "MuonSegmentMakerUtils/MuonSegmentKey.h"
#include "MuonSegmentMakerUtils/CompareMuonSegmentKeys.h"

#include "TrkEventPrimitives/FitQuality.h" 


namespace Muon {

  MuonSegmentOverlapRemovalTool::MuonSegmentOverlapRemovalTool(const std::string& ty,const std::string& na,const IInterface* pa)
    : AlgTool(ty,na,pa),
      m_idHelperTool("Muon::MuonIdHelpers/MuonIdHelperTool"), 
      m_helperTool("Muon::MuonEDMHelperTool/MuonEDMHelperTool"),
      m_printer("Muon::MuonEDMPrinterTool/MuonEDMPrinterTool"),
      m_log(0)
  {
    declareInterface<IMuonSegmentOverlapRemovalTool>(this);

    declareProperty("RemovePartialOverlaps",m_removePartialOverlaps = true,"removal partial overlaps between segments");
    declareProperty("OverlapFractionCut",m_overlapFractionCut = 0.8,"Cut overlap fraction, if fraction is smaller than cut both segments are kept");
  }


  MuonSegmentOverlapRemovalTool::~MuonSegmentOverlapRemovalTool(){}


  StatusCode MuonSegmentOverlapRemovalTool::initialize()
  {
    StatusCode sc = AlgTool::initialize(); 
    if ( sc.isFailure() ) {
      return sc;
    }

    m_log = new MsgStream(msgSvc(),name());
    m_debug = m_log->level() <= MSG::DEBUG;
    m_verbose = m_log->level() <= MSG::VERBOSE;


    sc = m_helperTool.retrieve();
    if (sc.isSuccess()){
      *m_log<<MSG::INFO << "Retrieved " << m_helperTool << endreq;
    }else{
      *m_log<<MSG::FATAL<<"Could not get " << m_helperTool <<endreq; 
      return sc;
    }

    sc = m_printer.retrieve();
    if (sc.isSuccess()){
      *m_log<<MSG::INFO << "Retrieved " << m_printer << endreq;
    }else{
      *m_log<<MSG::FATAL<<"Could not get " << m_printer <<endreq; 
      return sc;
    }

    sc = m_idHelperTool.retrieve();
    if (sc.isSuccess()){
      *m_log<<MSG::INFO << "Retrieved " << m_idHelperTool << endreq;
    }else{
      *m_log<<MSG::FATAL<<"Could not get " << m_idHelperTool <<endreq; 
      return sc;
    }

    
    return StatusCode::SUCCESS;
  }
  StatusCode MuonSegmentOverlapRemovalTool::finalize()
  {
    delete m_log;
    StatusCode sc = AlgTool::finalize(); 
    if( sc.isFailure() ) return StatusCode::FAILURE;
    return sc;
  }

  MuonSegmentOverlapRemovalTool::SegVec 
  MuonSegmentOverlapRemovalTool::removeDuplicates( const MuonSegmentOverlapRemovalTool::SegVec& segments ) const {

    if( m_debug ) *m_log << MSG::DEBUG << " working on segment vector of size " << segments.size() << std::endl;

    CompareMuonSegmentKeys compareSegmentKeys;

    // create a vector with pairs of MuonSegmentKey and a pointer to the corresponding segment to resolve ambiguities
    std::vector< std::pair<MuonSegmentKey,const MuonSegment*> > goodSegments;
    goodSegments.reserve(segments.size());

    // loop over new segments, copy them into collection
    SegCit sit = segments.begin();
    SegCit sit_end = segments.end();
    for( ; sit!=sit_end;++sit){

      // create segment key object
      MuonSegmentKey sk(**sit);
      Identifier chId = m_helperTool->chamberId(**sit);
      bool isCsc = m_idHelperTool->isCsc(chId);

      // should this segment be inserted?
      bool insertAsGood(true);

      // loop over already accepted segments and compare the current segment with the good ones
      std::vector< std::pair<MuonSegmentKey,const MuonSegment*> >::iterator gsit = goodSegments.begin();
      std::vector< std::pair<MuonSegmentKey,const MuonSegment*> >::iterator gsit_end = goodSegments.end();
      for( ;gsit!=gsit_end;++gsit ){
	CompareMuonSegmentKeys::OverlapResult overlapResult = compareSegmentKeys( gsit->first, sk );
	if( overlapResult == CompareMuonSegmentKeys::Identical ) {
	  // for now keep the one with the best chi2
	  const Trk::FitQuality* fq1 = gsit->second->fitQuality();
	  const Trk::FitQuality* fq2 = (*sit)->fitQuality();
	  double chi2good = fq1 ? fq1->chiSquared() : 1e10;
	  double chi2current = fq2 ? fq2->chiSquared() : 1e10;
	  // we should not insert this segment
	  if( chi2good > chi2current ) {
	    if( m_verbose ) *m_log << MSG::VERBOSE << " replacing (chi2) " << m_printer->print(*gsit->second) << std::endl
				  << " with      " << m_printer->print(**sit) << std::endl;

	    // good segment of worse quality, replace it
	    gsit->first  = sk;
	    gsit->second = *sit;
	    

	  }else{
	    if( m_verbose ) *m_log << MSG::VERBOSE << " discarding (chi2) " << m_printer->print(**sit) << std::endl;
	  }
	  insertAsGood = false;
	  break;
	}else if( overlapResult == CompareMuonSegmentKeys::SuperSet ){
	  // good segment of better quality, keep it and discard current segment
	  if( m_verbose ) *m_log << MSG::VERBOSE << " discarding (subset) " << m_printer->print(**sit) << std::endl;
	  insertAsGood = false;
	  break;
	}else if( overlapResult == CompareMuonSegmentKeys::SubSet ){
	  // good segment of worse quality, replace it
	  if( m_verbose ) *m_log << MSG::VERBOSE << " replacing (superset) " << m_printer->print(*gsit->second) << std::endl
				<< " with      " << m_printer->print(**sit) << std::endl;
	  gsit->first  = sk;
	  gsit->second = *sit;
	  insertAsGood = false;
	  break;
	}else if( overlapResult == CompareMuonSegmentKeys::PartialOverlap && m_removePartialOverlaps ){

	  // keep CSC segments with partial overlap
	  if( isCsc ) continue;

	  // partial overlap, for now keep the one most hits
	  if( compareSegmentKeys.segment1Size < compareSegmentKeys.segment2Size ){
	    if( m_verbose ) *m_log << MSG::VERBOSE << " replacing (mdt hits) " << m_printer->print(*gsit->second) << std::endl
				  << " with      " << m_printer->print(**sit) << std::endl;

	    // good segment of worse quality, replace it
	    gsit->first  = sk;
	    gsit->second = *sit;
	  }else if( compareSegmentKeys.segment1Size == compareSegmentKeys.segment2Size ){
	    // same size

	    // if the original entry has more trigger hits discard current segment
	    if( compareSegmentKeys.segment1SizeTrigger > compareSegmentKeys.segment2SizeTrigger ){
	      if( m_verbose )  *m_log << MSG::VERBOSE << " discarding (trigger hits) " << m_printer->print(**sit) << std::endl;
	      insertAsGood = false;
	      break;
	    }else if( compareSegmentKeys.segment1SizeTrigger < compareSegmentKeys.segment2SizeTrigger ){
	      // good segment of worse quality, replace it
	      if( m_verbose ) *m_log << MSG::VERBOSE << " replacing (trigger hits) " << m_printer->print(*gsit->second) << std::endl
				     << " with      " << m_printer->print(**sit) << std::endl;
	      gsit->first  = sk;
	      gsit->second = *sit;
	      insertAsGood = false;
	      break;	    
	    }else if( compareSegmentKeys.segment1SizeTrigger == compareSegmentKeys.segment2SizeTrigger ){

	      // for now look at the overlap fraction
	      
	    }
	    // calculate the overlap fraction
	    double overlapFraction = (double)compareSegmentKeys.intersectionSize/(double)compareSegmentKeys.segment1Size;
	    if( overlapFraction > m_overlapFractionCut ){
	      if( m_verbose )  *m_log << MSG::VERBOSE << " discarding (overlap fraction) " << m_printer->print(**sit) << std::endl;
	      insertAsGood = false;
	      break;
	    }

	    continue;
	  }else{
	    if( m_verbose ) *m_log << MSG::VERBOSE << " discarding " << m_printer->print(**sit) << std::endl;
	  }
	  insertAsGood = false;
	  break;
	}else if( overlapResult == CompareMuonSegmentKeys::NoOverlap ){
	  // no overlap, move on to next segment
	  continue;
	}else if( overlapResult == CompareMuonSegmentKeys::Unknown ){
	  *m_log << MSG::WARNING  << " Got invalid return argument comparing segments: " << compareSegmentKeys.print(overlapResult) << std::endl;
	  continue;
	}
      }
      
      // add segment if needed
      if( insertAsGood ) {
	if( m_verbose ) *m_log << MSG::VERBOSE << " no overlap " << m_printer->print(**sit) << std::endl;
	goodSegments.push_back( std::make_pair(sk,*sit) );
      }
    } 

    SegVec cleanedSegments;
    std::vector< std::pair<MuonSegmentKey,const MuonSegment*> >::iterator gsit = goodSegments.begin();
    std::vector< std::pair<MuonSegmentKey,const MuonSegment*> >::iterator gsit_end = goodSegments.end();
    for( ; gsit!=gsit_end;++gsit){
      cleanedSegments.push_back( gsit->second );
    }
    if( m_debug ) *m_log << MSG::DEBUG << " Segments after removal " << cleanedSegments.size() 
			 << " total number of removed segments " << segments.size() - cleanedSegments.size() << endreq;
    
    return cleanedSegments;
  }

  const MuonSegmentCombination* MuonSegmentOverlapRemovalTool::removeDuplicates( const MuonSegmentCombination& combi ) const {


    // store pointers to segments that should be kept
    std::set<const MuonSegment*> segmentsToBeKept;
    unsigned int nsegments = 0; // total number of segments 

    unsigned int nstations = combi.numberOfStations();

    if( m_debug )  *m_log << MSG::DEBUG << " removing duplicate from MuonSegmentCombination with  " << nstations << " stations" << endreq;
      

    // loop over segments, add them to the chamber layers
    // loop over chambers in combi and extract segments
    for(unsigned int i=0; i<nstations; ++i){

      // loop over segments in station
      const MuonSegmentCombination::SegmentVec* stationSegs = combi.stationSegments( i ) ;
         
      // check if not empty
      if( !stationSegs || stationSegs->empty() ) continue;
      
      nsegments += stationSegs->size();
      
      // clean segments
      SegVec cleanedSegments = removeDuplicates(*stationSegs);
      segmentsToBeKept.insert( cleanedSegments.begin(),cleanedSegments.end() );
    }

    // check whether all segments were kept
    if( nsegments == segmentsToBeKept.size() ){
      if( m_debug )  *m_log << MSG::DEBUG << "  no segments removed " << endreq;
      return &combi;
    }

    MuonSegmentCombination* newCombi = new MuonSegmentCombination();

    // create a new combination removing segments that overlap
    for(unsigned int i=0; i<nstations; ++i){

      // loop over segments in station
      const MuonSegmentCombination::SegmentVec* stationSegs = combi.stationSegments( i ) ;
         
      // check if not empty
      if( !stationSegs || stationSegs->empty() ) continue;

      std::vector< const MuonSegment* >* segVec = new std::vector< const MuonSegment* >();
      segVec->reserve( stationSegs->size() );

      // loop over segments and add them if they are in the keep set
      MuonSegmentCombination::SegmentVec::const_iterator ipsg=stationSegs->begin();    
      MuonSegmentCombination::SegmentVec::const_iterator ipsg_end=stationSegs->end();
      for (;ipsg!=ipsg_end;++ipsg){

	const MuonSegment* seg = dynamic_cast<const MuonSegment*>(*ipsg);
	
	if( !segmentsToBeKept.count(seg) ){
	  continue;
	}else{
	  segVec->push_back( seg->clone() );
	}
      }	

      // if not empty add segments
      if( !segVec->empty() ) newCombi->addSegments( segVec );
      else delete segVec;
    }

    if( m_debug )  *m_log << MSG::DEBUG << " removed " << nsegments-segmentsToBeKept.size() << " duplicates, new size " << segmentsToBeKept.size() << endreq;
    
    return newCombi;
  }
}
