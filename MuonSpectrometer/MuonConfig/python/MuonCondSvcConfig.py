# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration

# Based on http://acode-browser1.usatlas.bnl.gov/lxr/source/athena/MuonSpectrometer/MuonConditions/MuonCondGeneral/MuonCondSvc/python/

# Looking here in particular: http://acode-browser1.usatlas.bnl.gov/lxr/source/athena/MuonSpectrometer/MuonConditions/MuonCondGeneral/MuonCondSvc/python/MuonTopCondSvcConfigRUN2.py
# FIXME - work out how to support run1
# FIXME - in general my impression is that this is overly complicated and needs a huge clean up (the code, not just the configuration)

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from MuonStationIntersectSvc.MuonStationIntersectSvcConf import MuonStationIntersectSvc
from MuonCondSvc.MuonCondSvcConf import MDTCondSummarySvc, RPCCondSummarySvc, CSCCondSummarySvc, \
    TGCCondSummarySvc, MDT_DQConditionsSvc, MDT_DCSConditionsRun2Svc, MDT_DCSConditionsSvc
from MuonCondTool.MuonCondToolConf import MDT_DCSConditionsTool, MDT_DCSConditionsRun2Tool, MDT_MapConversion
from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg, addFolders

# def MDT_DCSConditionsSvcCfg(flags, **kwargs):
#     result = ComponentAccumulator()
#     # FIXME! Finish.
#     dcs_cond_svc = MDT_DCSConditionsSvc(MDT_DQConditionsTool=mdt_dcs_cond_tool)
#     result.addService(result)
#     return result

def MDTCondSummarySvcCfg(flags, **kwargs):
    # This is probably wrong. I'm pretty confused by this for the moment.
    result = ComponentAccumulator()
    # result.merge(IOVDbSvcCfg(flags))
    if flags.Common.isOnline:
      kwargs['ConditionsServices'] = []  # COOL folders not available online
    else:    
      if flags.Input.isMC : # OFLP200
          mdt_dcs_cond_tool = MDT_DCSConditionsTool(Simulation_Setup=flags.Input.isMC)
          result.addPublicTool(mdt_dcs_cond_tool)      
          cond_svc = MDT_DCSConditionsSvc(MDT_DCSConditionsTool = mdt_dcs_cond_tool)
          folders = [ "/MDT/DCS/DROPPEDCH", "/MDT/DCS/PSLVCHSTATE" ] # FIXME got this by observing other job ... not really sure if it's correct
      else:
          # if conddb.dbdata=="CONDBR2":
          #     kwargs['ConditionsServices'] = ['MDT_DCSConditionsRun2Svc']
          # else:
          #     kwargs['ConditionsServices'] = ['MDT_DCSConditionsSvc']
          if flags.IOVDb.DatabaseInstance=='CONDBR2': # CONDBR2 - run 2 data
              map_conversion_tool = MDT_MapConversion()
              result.addPublicTool(map_conversion_tool)
          
              mdt_dcs_cond_tool=MDT_DCSConditionsRun2Tool(MDT_MapConversion=map_conversion_tool)
              result.addPublicTool(mdt_dcs_cond_tool)
              cond_svc = MDT_DCSConditionsRun2Svc(MDT_DCSConditionsRun2Tool=mdt_dcs_cond_tool)
              # folders = ['/MDT/DCS/DROPPEDCH','/MDT/DCS/PSHVMLSTATE','/MDT/DCS/PSLVCHSTATE','/MDT/DCS/JTAGCHSTATE','/MDT/DCS/PSV0SETPOINTS', '/MDT/DCS/PSV1SETPOINTS']
              folders = [ "LVFolder", "HVFolder" ]
          else: 
              # Not yet implemented. Need to try to understand what on earth is going on first.
              mdt_dcs_cond_tool = MDT_DCSConditionsTool(Simulation_Setup=flags.Input.isMC)
              result.addPublicTool(mdt_dcs_cond_tool)
              cond_svc = MDT_DCSConditionsSvc(MDT_DCSConditionsTool=mdt_dcs_cond_tool)
              folders = [ "LVFolder", "HVFolder" ]
      result.merge( addFolders(flags, folders , detDb="DCS_OFL") ) # TODO FIXME! Get correct list
      result.addService(cond_svc)
      kwargs['ConditionsServices'] = [cond_svc]  # COOL folders not available online
    cond_summary = MDTCondSummarySvc(**kwargs)
    result.addService(cond_summary)
    return result, cond_summary
 
def RPCCondSummarySvcCfg(flags,**kwargs):
    result = ComponentAccumulator()
    result.merge(IOVDbSvcCfg(flags))
    
    if flags.Common.isOnline:
      kwargs['ConditionsServices'] = []  # COOL folders not available online
    else:    
      if flags.Input.isMC:
          kwargs['ConditionsServices'] = ['RPC_STATUSConditionsSvc']
      else:
          kwargs['ConditionsServices'] = ['RPC_STATUSConditionsSvc','RPC_DCSConditionsSvc']  # COOL folders not available online
          
    return RPCCondSummarySvc(name,**kwargs)
 
def CSCCondSummarySvcCfg(flags,**kwargs):
    result = ComponentAccumulator() 
    result.merge(IOVDbSvcCfg(flags))
    
    if flags.Common.isOnline:
      kwargs['ConditionsServices'] = []  # COOL folders not available online
    return CSCCondSummarySvc(name,**kwargs)

def TGCCondSummarySvcCfg(flags,**kwargs):
    result = ComponentAccumulator()
    result.merge(IOVDbSvcCfg(flags))

    if flags.Common.isOnline:
      kwargs['ConditionsServices'] = []  # COOL folders not available online
    return TGCCondSummarySvc(name,**kwargs) 

# def MuonStationIntersectSvcCfg(flags, **kwargs):
#     # has dependencies on
#     # 'IdHelper' : PublicToolHandle('Muon::MuonIdHelperTool/MuonIdHelperTool'), # GaudiHandle
#     # 'MDTCondSummarySvc' : ServiceHandle('MDTCondSummarySvc'), # GaudiHandle
#     result=ComponentAccumulator()
    
