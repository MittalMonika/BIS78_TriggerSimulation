/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: MuonSegmentCnvAlg.cxx 298140 2013-11-19 11:32:49Z emoyse $

// Gaudi/Athena include(s):
#include "AthenaKernel/errorcheck.h"

// EDM include(s):
#include "xAODMuon/MuonSegmentAuxContainer.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "TrkSegment/SegmentCollection.h"
#include "TrkSegment/Segment.h"
#include "MuonSegment/MuonSegment.h"
#include "AthLinks/ElementLink.h"

// Local include(s):
#include "MuonSegmentCnvAlg.h"

namespace xAODMaker {

  MuonSegmentCnvAlg::MuonSegmentCnvAlg( const std::string& name,
    ISvcLocator* svcLoc )
    : AthAlgorithm( name, svcLoc ),
      m_muonSegmentConverterTool("Muon::MuonSegmentConverterTool/MuonSegmentConverterTool")
  {

    declareProperty( "SegmentContainerName", m_muonSegmentLocation = "MuonSegments" );
    declareProperty( "xAODContainerName", m_xaodContainerName = "MuonSegments" );
  }

  StatusCode MuonSegmentCnvAlg::initialize() {

    ATH_MSG_DEBUG( "SegmentContainerName  = " << m_muonSegmentLocation.key() << "  xAODContainerName = " << m_xaodContainerName.key() );
    ATH_CHECK(m_muonSegmentConverterTool.retrieve());
    ATH_CHECK(m_muonSegmentLocation.initialize());
    ATH_CHECK(m_xaodContainerName.initialize());

      // Return gracefully:
    return StatusCode::SUCCESS;
  }

  StatusCode MuonSegmentCnvAlg::execute() {

     // Retrieve the AOD particles:
     const Trk::SegmentCollection* segments = 0;
     SG::ReadHandle<Trk::SegmentCollection> h_segments(m_muonSegmentLocation);
     if(h_segments.isValid()) {
       segments = h_segments.cptr();
     }
     else{
       ATH_MSG_FATAL( "Unable to retrieve " << m_muonSegmentLocation.key() );
       return StatusCode::FAILURE;
     }

     // If there's no input, we're done already:
     if( ! segments ) return StatusCode::SUCCESS;

     // Create the xAOD container and its auxiliary store:
     SG::WriteHandle<xAOD::MuonSegmentContainer> xaod (m_xaodContainerName);
     ATH_CHECK(xaod.record(std::make_unique<xAOD::MuonSegmentContainer>(),
                           std::make_unique<xAOD::MuonSegmentAuxContainer>()));

     unsigned int index = 0;
     for( auto it = segments->begin();it!=segments->end();++it,++index ){
        const Muon::MuonSegment* muonSegment = dynamic_cast<const Muon::MuonSegment*>(&(**it));
        if( !muonSegment ) continue;
        ElementLink< ::Trk::SegmentCollection > link(m_muonSegmentLocation.key(),index);
        /*xAOD::MuonSegment* xaodSegment =*/
        m_muonSegmentConverterTool->convert(link,xaod.ptr());
     }
     ATH_MSG_DEBUG( "Recorded MuonSegments with key: " << m_xaodContainerName.key() << " size " << xaod->size() );

     return StatusCode::SUCCESS;
  }
} // namespace xAODMaker
