# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration

class SCT_StripVetoToolSetup:
    "Class to simplify setup of SCT_StripVetoTool and required conditions algorithms"

    def __init__(self):
        self.toolName = "SCT_StripVetoTool"
        self.tool = None

    def setTool(self):
        from AthenaCommon.AppMgr import ToolSvc
        if not hasattr(ToolSvc, self.toolName):
            from SCT_ConditionsTools.SCT_ConditionsToolsConf import SCT_StripVetoTool
            ToolSvc += SCT_StripVetoTool(name = self.toolName)
        self.tool = getattr(ToolSvc, self.toolName)

    def getTool(self):
        return self.tool

    def setup(self):
        self.setTool()
