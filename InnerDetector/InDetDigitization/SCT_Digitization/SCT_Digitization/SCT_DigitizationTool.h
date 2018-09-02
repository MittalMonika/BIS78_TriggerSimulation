/* -*- C++ -*- */

/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SCT_DIGITZATION_SCT_DIGITZATIONTOOL_H
#define SCT_DIGITZATION_SCT_DIGITZATIONTOOL_H
/** @file SCT_DigitizationTool.h
 * @brief Digitize the SCT using an implementation of IPileUpTool
 * $Id: SCT_DigitizationTool.h,v 1.0 2009-09-22 18:34:42 jchapman Exp $
 * @author John Chapman - ATLAS Collaboration
 */

//Base class header
#include "PileUpTools/PileUpToolBase.h"

// Athena headers
#include "AthenaKernel/IAtRndmGenSvc.h"
#include "CommissionEvent/ComTime.h"
#include "HitManagement/TimedHitCollection.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "InDetSimData/InDetSimDataCollection.h"
#include "InDetSimEvent/SiHitCollection.h"
#include "PileUpTools/PileUpMergeSvc.h"
#include "SCT_Digitization/ISCT_FrontEnd.h"
#include "SCT_Digitization/ISCT_RandomDisabledCellGenerator.h"
#include "SCT_Digitization/ISCT_SurfaceChargesGenerator.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteHandleKey.h"

// Gaudi headers
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

// STL headers
#include <string>

// Forward declarations
class ISiChargedDiodesProcessorTool;
class SCT_ID;
class SiChargedDiodeCollectioon;

namespace CLHEP
{
  class HepRandomEngine;
}

class SCT_DigitizationTool : public extends<PileUpToolBase, IPileUpTool>
{
public:
  static const InterfaceID& interfaceID();
  SCT_DigitizationTool(const std::string& type,
                       const std::string& name,
                       const IInterface* parent);
  /**
     @brief Called before processing physics events
  */
  StatusCode prepareEvent(unsigned int) override final;
  virtual StatusCode processBunchXing(int bunchXing,
                                      SubEventIterator bSubEvents,
                                      SubEventIterator eSubEvents) override final;
  virtual StatusCode mergeEvent() override final;

  virtual StatusCode initialize() override final;
  virtual StatusCode processAllSubEvents() override final;

protected:

  bool       digitizeElement(SiChargedDiodeCollection* chargedDiodes); //!
  void       applyProcessorTools(SiChargedDiodeCollection* chargedDiodes); //!
  void       addSDO(SiChargedDiodeCollection* collection);

  void storeTool(ISiChargedDiodesProcessorTool* p_processor) {m_diodeCollectionTools.push_back(p_processor);}

private:

  /**
     @brief initialize the required services
  */
  StatusCode initServices();
  /**
     @brief initialize the random number engine
  */
  StatusCode initRandomEngine();
  /**
     @brief Initialize the SCT_FrontEnd AlgTool
  */
  StatusCode initFrontEndTool();
  /**
     @brief Initialize the SCT_RandomDisabledCellGenerator AlgTool
  */
  StatusCode initDisabledCells();
  /**
     @brief Initialize the SCT_SurfaceChargesGenerator AlgTool
  */
  StatusCode initSurfaceChargesGeneratorTool();

  /** RDO and SDO methods*/
  /**
     @brief Create RDOs from the SiChargedDiodeCollection for the current wafer and save to StoreGate
     @param chDiodeCollection       list of the SiChargedDiodes on the current wafer
  */
  StatusCode createAndStoreRDO(SiChargedDiodeCollection* chDiodeCollection);
  /**
     @brief Create RDOs from the SiChargedDiodeCollection for the current wafer
     @param chDiodeCollection       list of the SiChargedDiodes on the current wafer
  */

  SCT_RDO_Collection* createRDO(SiChargedDiodeCollection* collection);

  StatusCode getNextEvent();
  void       digitizeAllHits();     //!< digitize all hits
  void       digitizeNonHits();     //!< digitize SCT without hits

  float m_tfix;           //!< Use fixed timing for cosmics
  float m_comTime;         //!< Use Commission time for timing

  bool m_enableHits;            //!< Flag to enable hits
  bool m_onlyHitElements;       //!<
  bool m_cosmicsRun;            //!< Select a cosmic run
  bool m_useComTime;            //!< Flag to set the use of cosmics time for timing
  bool m_barrelonly;            //!< Only the barrel layers
  bool m_randomDisabledCells;   //!< Use Random disabled cells, default no
  bool m_createNoiseSDO;        //!< Create SDOs for strips with only noise hits (huge increase in SDO collection size)
  int  m_HardScatterSplittingMode; //!< Process all SiHit or just those from signal or background events
  bool m_HardScatterSplittingSkipper;
  BooleanProperty m_WriteSCT1_RawData;     //!< Write out SCT1_RawData rather than SCT3_RawData RDOs

  std::vector<bool> m_processedElements; //!< vector of processed elements - set by digitizeHits() */
  /**
     @brief Called when m_WriteSCT1_RawData is altered. Does nothing, but required by Gaudi.
  */

  void SetupRdoOutputType(Property&);

  SG::ReadHandleKey<ComTime> m_ComTimeKey{this, "ComTimeKey", "ComTime", "Handle to retrieve commissioning timing info from SG"};

  const SCT_ID*                                      m_detID;                             //!< Handle to the ID helper
  ToolHandle<ISCT_FrontEnd> m_sct_FrontEnd{this, "FrontEnd", "SCT_FrontEnd", "Handle the Front End Electronic tool"};
  ToolHandle<ISCT_SurfaceChargesGenerator> m_sct_SurfaceChargesGenerator{this, "SurfaceChargesGenerator", "SCT_SurfaceChargesGenerator", "Choice of using a more detailed charge drift model"};
  ToolHandle<ISCT_RandomDisabledCellGenerator> m_sct_RandomDisabledCellGenerator{this, "RandomDisabledCellGenerator", "SCT_RandomDisabledCellGenerator", ""};

  std::vector<SiHitCollection*> m_hitCollPtrs;

  SG::WriteHandleKey<SCT_RDO_Container> m_rdoContainerKey{this, "OutputObjectName", "SCT_RDOs", "Output Object name"};
  SG::WriteHandle<SCT_RDO_Container> m_rdoContainer; //!< RDO container handle
  SG::WriteHandleKey<InDetSimDataCollection> m_simDataCollMapKey{this, "OutputSDOName", "SCT_SDO_Map", "Output SDO container name"};
  SG::WriteHandle<InDetSimDataCollection>            m_simDataCollMap; //!< SDO Map handle
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_SCTDetEleCollKey{this, "SCTDetEleCollKey", "SCT_DetectorElementCollection", "Key of SiDetectorElementCollection for SCT"};

  std::string                                        m_inputObjectName;     //! name of the sub event  hit collections.
  ServiceHandle <IAtRndmGenSvc>                      m_rndmSvc;             //!< Random number service
  ServiceHandle <PileUpMergeSvc>                     m_mergeSvc; //!

  CLHEP::HepRandomEngine*                            m_rndmEngine;          //! Random number engine used - not init in SiDigitization
  std::list<ISiChargedDiodesProcessorTool*>          m_diodeCollectionTools;
  TimedHitCollection<SiHit>*                         m_thpcsi;
  SiChargedDiodeCollection*                          m_chargedDiodes;
  IntegerProperty                                    m_vetoThisBarcode;
};

static const InterfaceID IID_ISCT_DigitizationTool ("SCT_DigitizationTool", 1, 0);
inline const InterfaceID& SCT_DigitizationTool::interfaceID()
{
  return IID_ISCT_DigitizationTool;
}

#endif // SCT_DIGITZATION_SCT_DIGITZATIONTOOL_H
