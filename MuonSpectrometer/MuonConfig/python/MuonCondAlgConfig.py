# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

## Configuration Access to OFFLINE DB (COMP200)

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from MuonCondAlg.MuonCondAlgConf import MdtCondDbAlg,RpcCondDbAlg,CscCondDbAlg ##,TgcCondDbAlg
from IOVDbSvc.IOVDbSvcConfig import addFolders

def MdtCondDbAlgCfg(flags, **kwargs):
    result  = ComponentAccumulator()
    folders = []
    if flags.Common.isOnline:
        return result ## avoid adding algo to the component accumulator
        kwargs["isOnline"] = True
    else:
        kwargs["isOnline"] = False
        if flags.Input.isMC:
            kwargs['isData'] = False
            folders          = ["/MDT/DCS/DROPPEDCH", "/MDT/DCS/PSLVCHSTATE"]
        else:
            kwargs['isData'] = True
            kwargs['isRun1'] = flags.IOVDb.DatabaseInstance == 'COMP200'
            kwargs['useRun1SetPoints'] = False
            if kwargs['isRun1'] and kwargs['useRun1SetPoints']:
                folders = ["/MDT/DCS/PSV0SETPOINTS", "/MDT/DCS/PSV1SETPOINTS"]
            if kwargs['isRun1']:
                folders = ["/MDT/DCS/PSHVMLSTATE", "/MDT/DCS/PSLVCHSTATE", "/MDT/DCS/DROPPEDCH"]
            else:
                folders = ["/MDT/DCS/HV", "/MDT/DCS/LV"]
    alg = MdtCondDbAlg(**kwargs)
    result.merge( addFolders(flags, folders , detDb="DCS_OFL", className='CondAttrListCollection') )
    result.addCondAlgo(alg)
    return result

def RpcCondDbAlgCfg(flags, **kwargs):
    result  = ComponentAccumulator()
    folders = []
    if flags.Common.isOnline:
        return result ## avoid adding algo to the component accumulator
        kwargs["isOnline"] = True
    else:
        kwargs["isOnline"] = False
        if flags.Input.isMC:
            kwargs['isData'] = False
        else:
            kwargs['isData'] = True
            kwargs['isRun1'] = flags.IOVDb.DatabaseInstance == 'COMP200'
            folders          = ["/RPC/DCS/DeadRopanels", "/RPC/DCS/OffRopanels"]
    alg = RpcCondDbAlg(**kwargs)
    result.merge( addFolders(flags, folders                     , detDb="DCS_OFL", className='CondAttrListCollection') )
    result.merge( addFolders(flags, ["/RPC/DQMF/ELEMENT_STATUS"], detDb="RPC_OFL", className='CondAttrListCollection') )
    result.addCondAlgo(alg)
    return result

def CscCondDbAlgCfg(flags, **kwargs):
    result  = ComponentAccumulator()
    folders = ["/CSC/STAT"]
    if flags.Common.isOnline:
        return result ## avoid adding algo to the component accumulator
        kwargs["isOnline"] = True
    else:
        kwargs["isOnline"] = False
        if flags.Input.isMC:
            kwargs['isData'] = False
        else:
            kwargs['isData'] = True
            kwargs['isRun1'] = flags.IOVDb.DatabaseInstance == 'COMP200'
    alg = CscCondDbAlg(**kwargs)
    result.merge( addFolders(flags, folders , detDb="CSC_OFL", className='CondAttrListCollection') )
    result.addCondAlgo(alg)
    return result

###def TgcCondDbAlgCfg(flags, **kwargs):
###    result  = ComponentAccumulator()
###    folders = [] # which folders?
###    if flags.Common.isOnline:
###        return result ## avoid adding algo to the component accumulator
###        kwargs["isOnline"] = True
###    else:
###        kwargs["isOnline"] = False
###        if flags.Input.isMC:
###            kwargs['isData'] = False
###        else:
###            kwargs['isData'] = True
###            kwargs['isRun1'] = flags.IOVDb.DatabaseInstance == 'COMP200'
###    alg = TgcCondDbAlg(**kwargs)
###    result.merge( addFolders(flags, folders , detDb="DCS_OFL", className='CondAttrListCollection') )
###    result.addCondAlgo(alg)
###    return result



