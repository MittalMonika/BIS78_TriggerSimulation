/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PixelDigitizationTool.h
//   Header file for class PixelDigitizationTool
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
#ifndef PIXELDIGITIZATION_PIXELDIGITIZATIONTOOL_H
#define PIXELDIGITIZATION_PIXELDIGITIZATIONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "PileUpTools/PileUpToolBase.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "HitManagement/TimedHitCollection.h"
#include "InDetSimEvent/SiHitCollection.h"

#include "InDetRawData/InDetRawDataCLASS_DEF.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteHandleKey.h"
#include "PileUpTools/PileUpMergeSvc.h"

#include "InDetSimData/InDetSimDataCollection.h"
#include "SensorSimTool.h"
#include "FrontEndSimTool.h"
#include "EnergyDepositionTool.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

class PixelDigitizationTool : public PileUpToolBase {

  public:
    PixelDigitizationTool(const std::string &type, const std::string &name, const IInterface *pIID);

    virtual StatusCode initialize() override;
    virtual StatusCode processAllSubEvents() override;
    virtual StatusCode finalize() override;

    virtual StatusCode prepareEvent(unsigned int) override;
    StatusCode digitizeEvent();
    virtual StatusCode mergeEvent() override;
    virtual StatusCode processBunchXing(int bunchXing, SubEventIterator bSubEvents, SubEventIterator eSubEvents) override final;

  protected:
    void addSDO(SiChargedDiodeCollection *collection);

  private:

    PixelDigitizationTool();
    PixelDigitizationTool(const PixelDigitizationTool&);
    PixelDigitizationTool &operator=(const PixelDigitizationTool&);

    std::vector<SiHitCollection*> m_hitCollPtrs;

    Gaudi::Property<bool> m_onlyUseContainerName{this, "OnlyUseContainerName", true, "Don't use the ReadHandleKey directly. Just extract the container name from it."};
    SG::ReadHandleKey<SiHitCollection>         m_hitsContainerKey{this, "InputObjectName", "", "Input HITS collection name"};
    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey{this, "PixelDetEleCollKey", "PixelDetectorElementCollection", "Key of SiDetectorElementCollection for Pixel"};
    std::string                                m_inputObjectName{""};
    SG::WriteHandleKey<PixelRDO_Container>     m_rdoContainerKey{this, "RDOCollName", "PixelRDOs", "RDO collection name"};
    SG::WriteHandle<PixelRDO_Container>        m_rdoContainer{};
    SG::WriteHandleKey<InDetSimDataCollection> m_simDataCollKey{this, "SDOCollName", "PixelSDO_Map",  "SDO collection name"};
    SG::WriteHandle<InDetSimDataCollection>    m_simDataColl{};
    Gaudi::Property<int>                       m_HardScatterSplittingMode{this, "HardScatterSplittingMode", 0, "Control pileup & signal splitting"};
    bool                                       m_HardScatterSplittingSkipper{false};
    Gaudi::Property<bool>                      m_onlyHitElements{this, "OnlyHitElements", false, "Process only elements with hits"};

    const PixelID            *m_detID{};


    TimedHitCollection<SiHit> *m_timedHits{};

    ToolHandleArray<SensorSimTool> m_chargeTool{this, "ChargeTools", {}, "List of charge tools"};
    ToolHandleArray<FrontEndSimTool> m_fesimTool{this, "FrontEndSimTools", {}, "List of Front-End simulation tools"};
    ToolHandle<EnergyDepositionTool> m_energyDepositionTool{this, "EnergyDepositionTool", "EnergyDepositionTool", "Energy deposition tool"};

  protected:

    ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc", ""};  //!< Random number service
    ServiceHandle <PileUpMergeSvc> m_mergeSvc{this, "PileUpMergeSvc", "PileUpMergeSvc", ""};

    Gaudi::Property<bool>          m_createNoiseSDO{this, "CreateNoiseSDO",   false,  "Set create noise SDO flag"};

};

#endif // PIXELDIGITIZATION_PIXELDIGITIZATIONTOOL_H
