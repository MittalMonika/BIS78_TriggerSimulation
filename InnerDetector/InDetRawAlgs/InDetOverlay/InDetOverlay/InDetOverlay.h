/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-

/** 
 * @file
 *
 * Overlaying RDOs from two different events for InDet subdetectors.
 *
 * @author Andrei Gaponenko, 2006-2009
 */

#ifndef INDETOVERLAY_H
#define INDETOVERLAY_H

#include <string>

#include "GaudiKernel/ServiceHandle.h"

#include "IDC_OverlayBase/IDC_OverlayBase.h"

#include "InDetRawData/TRT_RDO_Container.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "TRT_ElectronPidTools/ITRT_LocalOccupancy.h"
typedef  InDetRawDataCollection<TRT_RDORawData> TRT_RDO_Collection;

#include "InDetSimData/InDetSimDataCollection.h"
#include "AthenaKernel/IAtRndmGenSvc.h"


class StoreGateSvc;
class SCT_ID;
class TRT_ID;

namespace CLHEP {
  class HepRandomEngine;
}

class InDetOverlay : public IDC_OverlayBase {
public:
  
  InDetOverlay(const std::string &name,ISvcLocator *pSvcLocator);
  
  virtual StatusCode overlayInitialize();
  virtual StatusCode overlayExecute();
  virtual StatusCode overlayFinalize();

  // SCT collection merging needs an ID helper.  Instead of retrieving
  // it thousands of times per event in the collection merging code,
  // do it once in InDetOverlay initialization and make available here
  const SCT_ID *get_sct_id() const { return m_sct_id; }

  enum SCT_numbers {NextBC=0, CurrentBC=1, PreviousBC=2, AnyBC=3, NumberOfBCs=3, NumberOfBitSets=4,
                    NumberOfStrips=768,
                    MCSource=0, DataSource=1, NumberOfSources=2};
private:

 
  void overlayTRTContainers(const TRT_RDO_Container *pileupContainer,
                            const TRT_RDO_Container *signalContainer,
                            TRT_RDO_Container *outputContainer,
                            std::map<int,double>&  occupancyMap,
                            const InDetSimDataCollection& SDO_Map);


  void mergeTRTCollections(TRT_RDO_Collection *mc_coll, 
                           TRT_RDO_Collection *data_coll, 
                           TRT_RDO_Collection *out_coll, 
                           double occupancy, 
                           const InDetSimDataCollection& SDO_Map);

private:

  ServiceHandle<StoreGateSvc> m_detStore;
  const SCT_ID *m_sct_id;
  const TRT_ID *m_trt_id;

  // ----------------------------------------------------------------
  //! jO controllable properties.
  //! "Main" containers are read, have data from "overlay" containers added,
  //! and written out with the original SG keys.
  bool m_do_TRT, m_do_TRT_background;
  SG::ReadHandleKey<TRT_RDO_Container> m_mainInputTRTKey;
  SG::ReadHandleKey<TRT_RDO_Container> m_overlayInputTRTKey;
  SG::WriteHandleKey<TRT_RDO_Container> m_mainOutputTRTKey;

  bool m_do_SCT, m_do_SCT_background;
  SG::ReadHandleKey<SCT_RDO_Container> m_mainInputSCTKey;
  SG::ReadHandleKey<SCT_RDO_Container> m_overlayInputSCTKey;
  SG::WriteHandleKey<SCT_RDO_Container> m_mainOutputSCTKey;

  bool m_do_Pixel, m_do_Pixel_background;
  SG::ReadHandleKey<PixelRDO_Container> m_mainInputPixelKey;
  SG::ReadHandleKey<PixelRDO_Container> m_overlayInputPixelKey;
  SG::WriteHandleKey<PixelRDO_Container> m_mainOutputPixelKey;
  
  // Following tools, services and configurables are there only for the correct of HT hits
  ServiceHandle <IAtRndmGenSvc> m_rndmSvc;
  std::string                   m_rndmEngineName;
  CLHEP::HepRandomEngine *      m_rndmEngine;
  
  std::string m_TRTinputSDO_Name;
  double                                      m_HTOccupancyCorrectionB;
  double                                      m_HTOccupancyCorrectionEC;
  double                                      m_HTOccupancyCorrectionB_noE;
  double                                      m_HTOccupancyCorrectionEC_noE;
  ToolHandle< InDet::ITRT_LocalOccupancy >    m_TRT_LocalOccupancyTool; 
};

#endif/*INDETOVERLAY_H*/
