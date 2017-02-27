/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
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
#include "HitManagement/TimedHitCollection.h"
#include "InDetSimEvent/SiHitCollection.h"

#include "InDetRawData/InDetRawDataCLASS_DEF.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

#include "StoreGate/WriteHandle.h"

#include "InDetSimData/InDetSimDataCollection.h"
// STSTST #include "SiDigitization/ISiChargedDiodesProcessorTool.h"
#include "PixelProcessorTool.h"
#include "SubChargesTool.h"
#include "FrontEndSimTool.h"

#include "InDetReadoutGeometry/PixelDetectorManager.h"

class PixelDigitizationTool : public PileUpToolBase {

  public:
    PixelDigitizationTool(const std::string &type, const std::string &name, const IInterface *pIID);

    virtual StatusCode initialize();
    virtual StatusCode processAllSubEvents();
    virtual StatusCode finalize();

    StatusCode prepareEvent(unsigned int);
    StatusCode digitizeEvent();
    StatusCode mergeEvent();
    virtual StatusCode processBunchXing(int bunchXing, SubEventIterator bSubEvents, SubEventIterator eSubEvents) override final;

  protected:
    void addSDO(SiChargedDiodeCollection *collection);

  private:

    PixelDigitizationTool();
    PixelDigitizationTool(const PixelDigitizationTool&);
    PixelDigitizationTool &operator=(const PixelDigitizationTool&);

    std::vector<SiHitCollection*> hitCollPtrs;

    SG::WriteHandle<PixelRDO_Container>     m_rdoContainer;
    SG::WriteHandle<InDetSimDataCollection> m_simDataColl;

    int               m_HardScatterSplittingMode;
    bool              m_HardScatterSplittingSkipper;
    std::string       m_rndmEngineName;

    bool              m_onlyHitElements;

// STSTST    ToolHandleArray<ISiChargedDiodesProcessorTool> m_diodesProcsTool;
    ToolHandleArray<PixelProcessorTool>            m_processorTool;
    ToolHandleArray<SubChargesTool>                m_chargeTool;
    ToolHandleArray<FrontEndSimTool>               m_fesimTool;

    const PixelID            *m_detID;

    IntegerProperty           m_vetoThisBarcode;

    TimedHitCollection<SiHit> *m_timedHits;

  protected:

    ServiceHandle <IAtRndmGenSvc> m_rndmSvc;
    ServiceHandle <PileUpMergeSvc> m_mergeSvc;

    CLHEP::HepRandomEngine *m_rndmEngine;
    const InDetDD::SiDetectorManager *m_detManager;

    std::string   m_inputObjectName;
    bool          m_createNoiseSDO;

};

#endif // PIXELDIGITIZATION_PIXELDIGITIZATIONTOOL_H
