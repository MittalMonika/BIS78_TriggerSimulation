/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#include <GaudiKernel/DeclareFactoryEntries.h>

#include <AsgAnalysisAlgorithms/AsgPtEtaSelectionTool.h>
#include <AsgAnalysisAlgorithms/AsgSelectionAlg.h>
#include <AsgAnalysisAlgorithms/AsgViewFromSelectionAlg.h>
#include <AsgAnalysisAlgorithms/KinematicHistAlg.h>
#include <AsgAnalysisAlgorithms/ObjectCutFlowHistAlg.h>
#include <AsgAnalysisAlgorithms/OverlapRemovalAlg.h>
#include <AsgAnalysisAlgorithms/PileupReweightingAlg.h>
#include <AsgAnalysisAlgorithms/SysListLoaderAlg.h>

DECLARE_TOOL_FACTORY (CP::AsgPtEtaSelectionTool)
DECLARE_ALGORITHM_FACTORY (CP::AsgSelectionAlg)
DECLARE_ALGORITHM_FACTORY (CP::AsgViewFromSelectionAlg)
DECLARE_ALGORITHM_FACTORY (CP::KinematicHistAlg)
DECLARE_ALGORITHM_FACTORY (CP::ObjectCutFlowHistAlg)
DECLARE_ALGORITHM_FACTORY (CP::OverlapRemovalAlg)
DECLARE_ALGORITHM_FACTORY (CP::PileupReweightingAlg)
DECLARE_ALGORITHM_FACTORY (CP::SysListLoaderAlg)

DECLARE_FACTORY_ENTRIES(AsgAnalysisAlgorithms) {
  DECLARE_ALGTOOL (CP::AsgPtEtaSelectionTool)
  DECLARE_ALGORITHM (CP::AsgSelectionAlg)
  DECLARE_ALGORITHM (CP::AsgViewFromSelectionAlg)
  DECLARE_ALGORITHM (CP::KinematicHistAlg)
  DECLARE_ALGORITHM (CP::ObjectCutFlowHistAlg)
  DECLARE_ALGORITHM (CP::OverlapRemovalAlg)
  DECLARE_ALGORITHM (CP::PileupReweightingAlg)
  DECLARE_ALGORITHM (CP::SysListLoaderAlg)
}

