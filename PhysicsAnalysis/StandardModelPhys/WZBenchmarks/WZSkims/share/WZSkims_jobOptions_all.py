# WZSkims Athena Job Options file
# Homero Martinez, Bernardo Resende
# 30/12/10

# Input file
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
athenaCommonFlags.FilesInput= ["AOD.pool.root"]

# Skim selection
from D3PDMakerConfig.D3PDProdFlags import prodFlags
prodFlags.WriteSMDYEED3PD = True
prodFlags.WriteSMDYEED3PD.FileName = 'WZSkimsD3PD_Jpsiee.root'
prodFlags.WriteSMDYEED3PD.lock()

prodFlags.WriteSMZEED3PD = True
prodFlags.WriteSMZEED3PD.FileName = 'WZSkimsD3PD_Zee.root'
prodFlags.WriteSMZEED3PD.lock()

prodFlags.WriteSMWENUD3PD = True
prodFlags.WriteSMWENUD3PD.FileName = 'WZSkimsD3PD_Wenu.root'
prodFlags.WriteSMWENUD3PD.lock()

prodFlags.WriteSMBKGED3PD = True
prodFlags.WriteSMBKGED3PD.FileName = 'WZSkimsD3PD_BackgroundElec.root'
prodFlags.WriteSMBKGED3PD.lock()

prodFlags.WriteSMDYMUMUD3PD = True
prodFlags.WriteSMDYMUMUD3PD.FileName = 'WZSkimsD3PD_Jpsimumu.root'
prodFlags.WriteSMDYMUMUD3PD.lock()

prodFlags.WriteSMZMUMUD3PD = True
prodFlags.WriteSMZMUMUD3PD.FileName = 'WZSkimsD3PD_Zmumu.root'
prodFlags.WriteSMZMUMUD3PD.lock()

prodFlags.WriteSMWMUNUD3PD = True
prodFlags.WriteSMWMUNUD3PD.FileName = 'WZSkimsD3PD_Wmunu.root'
prodFlags.WriteSMWMUNUD3PD.lock()

prodFlags.WriteSMBKGMUD3PD = True
prodFlags.WriteSMBKGMUD3PD.FileName = 'WZSkimsD3PD_BackgroundMuon.root'
prodFlags.WriteSMBKGMUD3PD.lock()

prodFlags.WriteSMDILEPD3PD = True
prodFlags.WriteSMDILEPD3PD.FileName = 'WZSkimsD3PD_TwoLeptons.root'
prodFlags.WriteSMDILEPD3PD.lock()

prodFlags.WriteSMTRILEPD3PD = True
prodFlags.WriteSMTRILEPD3PD.FileName = 'WZSkimsD3PD_ThreeLeptons.root'
prodFlags.WriteSMTRILEPD3PD.lock()

# Configure RecExCommon
from RecExConfig.RecFlags import rec
rec.doCBNT.set_Value_and_Lock(False)
rec.doWriteTAG.set_Value_and_Lock(False)

# Needed for MSMgr
rec.DPDMakerScripts.append("PhysicsD3PDMaker/SMDYEED3PD_prodJobOFragment.py")
rec.DPDMakerScripts.append("PhysicsD3PDMaker/SMWENUD3PD_prodJobOFragment.py")
rec.DPDMakerScripts.append("PhysicsD3PDMaker/SMZEED3PD_prodJobOFragment.py")
rec.DPDMakerScripts.append("PhysicsD3PDMaker/SMBKGED3PD_prodJobOFragment.py")
rec.DPDMakerScripts.append("PhysicsD3PDMaker/SMDYMUMUD3PD_prodJobOFragment.py")
rec.DPDMakerScripts.append("PhysicsD3PDMaker/SMWMUNUD3PD_prodJobOFragment.py")
rec.DPDMakerScripts.append("PhysicsD3PDMaker/SMZMUMUD3PD_prodJobOFragment.py")
rec.DPDMakerScripts.append("PhysicsD3PDMaker/SMBKGMUD3PD_prodJobOFragment.py")
rec.DPDMakerScripts.append("PhysicsD3PDMaker/SMDILEPD3PD_prodJobOFragment.py")
rec.DPDMakerScripts.append("PhysicsD3PDMaker/SMTRILEPD3PD_prodJobOFragment.py")

# Main jobOption - must always be included!!!
include ("RecExCommon/RecExCommon_topOptions.py")

# Number of events to be processed
theApp.EvtMax = -1

# Stops writing of monitoring ntuples
from PerfMonComps.PerfMonFlags import jobproperties as jp
jp.PerfMonFlags.doMonitoring = False
jp.PerfMonFlags.doFastMon = False
