// -*- C++ -*-

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELCONDITIONSALGORITHMS_PIXELALIGNCONDALG_H
#define PIXELCONDITIONSALGORITHMS_PIXELALIGNCONDALG_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

#include "GaudiKernel/ICondSvc.h"

#include "DetDescrConditions/AlignableTransformContainer.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "GeoModelUtilities/GeoAlignmentStore.h"

namespace InDetDD {
  class PixelDetectorManager;
}

class PixelAlignCondAlg : public AthAlgorithm
{
 public:
  PixelAlignCondAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~PixelAlignCondAlg() override = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

 private:
  BooleanProperty m_useDynamicAlignFolders{
    this, "UseDynamicAlignFolders", false, "Flag of dynamic or static ID alignment folders"};
  SG::ReadCondHandleKey<AlignableTransformContainer> m_readKeyStatic{
    this, "ReadKeyStatic", "/Indet/Align", "Key for the static alignment folder"};
  SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyDynamicL1{
    this, "ReadKeyDynamicL1", "/Indet/AlignL1/ID", "Key for the dynamic L1 alignment folder"};
  SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyDynamicL2{
    this, "ReadKeyDynamicL2", "/Indet/AlignL2/PIX", "Key for the dynamic L2 alignment folder"};
  SG::ReadCondHandleKey<AlignableTransformContainer> m_readKeyDynamicL3{
    this, "ReadKeyDynamicL3", "/Indet/AlignL3", "Key for the dynamic L3 alignment folder"};
  SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyDynamicIBL{
    this, "ReadKeyDynamicIBL", "/Indet/IBLDist", "Key for the dynamic IBL alignment folder"};

  SG::WriteCondHandleKey<GeoAlignmentStore> m_writeKey;

  ServiceHandle<ICondSvc> m_condSvc{this, "CondSvc", "CondSvc"};
  const InDetDD::PixelDetectorManager* m_detManager{nullptr};
};

#endif // SCT_CONDITIONSALGORITHMS_SCT_ALIGNCONDALG_H
