/*
 *   Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
 *   */

/**
 *
 *  NAME:     HLTCaloCellMaker.h
 *   PACKAGE:  Trigger/TrigAlgorithms/TrigT2CaloCommon
 *
 *    AUTHOR:   Denis Oliveira Damazio
 *
 *     PURPOSE:  New Algorithm to produce CaloCellContainer
 *      **/

#ifndef TRIGT2CALOCOMMON_HLTCALOCELLMAKER_H
#define TRIGT2CALOCOMMON_HLTCALOCELLMAKER_H

#include <string>
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "CaloEvent/CaloCellContainerVector.h"
#include "AthContainers/ConstDataVector.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "TileConditions/TileEMScale.h"


class ITrigCaloDataAccessSvc;

/**  Test Algorithm to unpack sequence
 *     produces the CaloCellContainer output. */
class HLTCaloCellMaker: public AthReentrantAlgorithm {
     public:
	HLTCaloCellMaker(const std::string & name, ISvcLocator* pSvcLocator);
	~HLTCaloCellMaker();

	StatusCode execute_r(const EventContext& context ) const ;
	StatusCode initialize();
     private:

	SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiCollectionKey;
	SG::WriteHandleKey<ConstDataVector<CaloCellContainerVector> > m_cellContainerVKey;
	SG::WriteHandleKey<ConstDataVector<CaloCellContainer> > m_cellContainerKey;
        /// FIXME: Temporary (i hope) to get dependency needed by BS converter.
        SG::ReadCondHandleKey<TileEMScale> m_tileEMScaleKey;

	ServiceHandle<ITrigCaloDataAccessSvc> m_dataAccessSvc;
	bool m_roiMode;
};

#endif

