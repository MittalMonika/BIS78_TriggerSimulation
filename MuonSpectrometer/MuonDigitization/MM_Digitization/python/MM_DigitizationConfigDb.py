# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addTool
addTool("MM_Digitization.MM_DigitizationConfig.MM_DigitizationTool","MmDigitizationTool")
addTool("MM_Digitization.MM_DigitizationConfig.MM_Response_DigitTool","MM_Response_DigitTool")
addTool("MM_Digitization.MM_DigitizationConfig.getMMRange", "MMRange")
addTool("MM_Digitization.MM_DigitizationConfig.MM_OverlayDigitizationTool", "MM_OverlayDigitizationTool")

