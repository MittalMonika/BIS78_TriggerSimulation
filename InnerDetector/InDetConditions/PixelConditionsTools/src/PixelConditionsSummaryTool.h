/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELCONDITIONSSERVICES_PIXELCONDITIONSSUMMARYTOOL_H
#define PIXELCONDITIONSSERVICES_PIXELCONDITIONSSUMMARYTOOL_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetConditionsSummaryService/IInDetConditionsTool.h"

#include "PixelConditionsTools/IPixelDCSConditionsTool.h"
#include "PixelConditionsServices/IPixelByteStreamErrorsSvc.h"

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "InDetIdentifier/PixelID.h"

#include "PixelConditionsData/PixelModuleData.h"
#include "StoreGate/ReadCondHandleKey.h"

class PixelConditionsSummaryTool: public AthAlgTool, public IInDetConditionsTool{
  public:
    static InterfaceID& interfaceID();

    PixelConditionsSummaryTool(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~PixelConditionsSummaryTool();
    virtual StatusCode initialize() override;

    virtual bool isActive(const Identifier & elementId, const InDetConditions::Hierarchy h=InDetConditions::DEFAULT) const override final;
    virtual bool isActive(const IdentifierHash & moduleHash) const override final;
    virtual bool isActive(const IdentifierHash & moduleHash, const Identifier & elementId)  const override final;
    virtual double activeFraction(const IdentifierHash & moduleHash, const Identifier & idStart, const Identifier & idEnd)  const override final;

    virtual bool isGood(const Identifier & elementId, const InDetConditions::Hierarchy h=InDetConditions::DEFAULT) const override final;
    virtual bool isGood(const IdentifierHash & moduleHash) const override final;
    virtual bool isGood(const IdentifierHash & moduleHash, const Identifier & elementId) const override final;
    virtual double goodFraction(const IdentifierHash & moduleHash, const Identifier & idStart, const Identifier & idEnd) const override final;

  private:
    const PixelID* m_pixelID;

    ToolHandle<IPixelDCSConditionsTool>     m_DCSConditionsTool    {this, "PixelDCSConditionsTool",     "PixelDCSConditionsTool",     "Tool to retrieve Pixel information"};

    ServiceHandle< IPixelByteStreamErrorsSvc > m_pixelBSErrorsSvc;
    std::vector<std::string> m_isActiveStatus;
    std::vector<std::string> m_isActiveStates;

    bool m_useDCSState;
    bool m_useByteStream;
    bool m_useTDAQ;
    bool m_useDeadMap;

    SG::ReadCondHandleKey<PixelModuleData> m_condKey{this, "PixelModuleData", "PixelModuleData", "Output key of pixel module data"};
};

inline InterfaceID& PixelConditionsSummaryTool::interfaceID(){
  static InterfaceID IID_PixelConditionsSummaryTool("PixelConditionsSummaryTool", 1, 0);
  return IID_PixelConditionsSummaryTool;
}

#endif
