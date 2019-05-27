/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// Tile includes
#include "TileRawChannelCorrectionAlg.h"
#include "TileEvent/TileMutableRawChannelContainer.h"

// Athena includes
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "AthenaKernel/errorcheck.h"

#include <memory>

TileRawChannelCorrectionAlg::TileRawChannelCorrectionAlg(const std::string& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{
}


TileRawChannelCorrectionAlg::~TileRawChannelCorrectionAlg() {
}


StatusCode TileRawChannelCorrectionAlg::initialize() {

  ATH_MSG_DEBUG( "Input raw channel container: '" << m_inputRawChannelContainerKey.key()
                << "'  output container: '" << m_outputRawChannelContainerKey.key() << "'" );

  ATH_CHECK( m_inputRawChannelContainerKey.initialize() );
  ATH_CHECK( m_outputRawChannelContainerKey.initialize() );

  ATH_CHECK( m_noiseFilterTools.retrieve() );
  ATH_MSG_DEBUG( "Successfully retrieve  NoiseFilterTools: " << m_noiseFilterTools );

  ATH_MSG_DEBUG( "initialization completed" );

  return StatusCode::SUCCESS;
}


StatusCode TileRawChannelCorrectionAlg::execute(const EventContext& ctx) const {


  const TileRawChannelContainer* inputRawChannels = SG::makeHandle(m_inputRawChannelContainerKey, ctx).get();
  auto outputRawChannels = std::make_unique<TileMutableRawChannelContainer>(*inputRawChannels);
  ATH_CHECK( outputRawChannels->status() );

  for (const ToolHandle<ITileRawChannelTool>& noiseFilterTool : m_noiseFilterTools) {
    if (noiseFilterTool->process(*outputRawChannels.get()).isFailure()) {
      ATH_MSG_ERROR( " Error status returned from noise filter " );
    } else {
      ATH_MSG_DEBUG( "Noise filter applied to the container" );
    }
  }

  SG::WriteHandle<TileRawChannelContainer> outputRawChannelContainer(m_outputRawChannelContainerKey, ctx);
  ATH_CHECK( outputRawChannelContainer.record(std::move(outputRawChannels)) );

  ATH_MSG_DEBUG( "TileRawChannelContainer registered successfully (" 
                 << m_outputRawChannelContainerKey.key() << ")");

  return StatusCode::SUCCESS;
}


StatusCode TileRawChannelCorrectionAlg::finalize() {

  ATH_MSG_DEBUG( "in finalize()" );

  return StatusCode::SUCCESS;
}
