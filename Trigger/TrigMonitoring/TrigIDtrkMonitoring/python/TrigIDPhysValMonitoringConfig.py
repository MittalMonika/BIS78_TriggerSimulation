# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

def TrigIDPhysValMonitoringTool( legacy_monitoring=False ):

  from AthenaCommon.Constants import INFO,ERROR,FALSE,TRUE,DEBUG,VERBOSE

  # dataTypes: userDefined = 0, monteCarlo, collisions, cosmics
  if not 'DQMonFlags' in dir():
    from AthenaMonitoring.DQMonFlags import DQMonFlags
  dataType = DQMonFlags.monManDataType()

  # disable everything
  outputlist = []

  if not 'rec' in dir():
    from RecExConfig.RecFlags  import rec
    
  from TriggerJobOpts.HLTTriggerResultGetter import EDMDecodingVersion
  from TriggerJobOpts.TriggerFlags import TriggerFlags
      
  EDMDecodingVersion()
      
  mt_chains = True
  if ( TriggerFlags.EDMDecodingVersion < 3 or legacy_monitoring ) :
    mt_chains = False
        
  if rec.doInDet:
    from TrigInDetAnalysisExample.TrigInDetAnalysisExampleConf import TrigTestPhysValMon
    from AthenaCommon.AppMgr import release_metadata
    d = release_metadata()

    def makePhysvalMon(name, pdgid, chainnames, useHighestPT, cosmic = False, useOffline = False, doFS=False ):

      Monname = "TestIDPhysValMon" + name
      TestIDPhysValMon = TrigTestPhysValMon(name=Monname)
      TestIDPhysValMon.SliceTag = "HLT/IDMon/" + name
      TestIDPhysValMon.OutputLevel = INFO
      TestIDPhysValMon.UseHighestPT = useHighestPT

      try:
        TestIDPhysValMon.EnableLumi = False
      except:
        pass

      TestIDPhysValMon.buildNtuple = False
      TestIDPhysValMon.AnalysisConfig = "Tier0" #T0 Analysis
      # TestIDPhysValMon.AnalysisConfig = "nTuple" #nTuple Analysis

      if (useOffline or rec.doTruth == False):
        TestIDPhysValMon.mcTruth = False
        TestIDPhysValMon.ntupleChainNames = ['Offline',name]
#       use default values ? 
#       TestIDPhysValMon.sctHitsOffline = 1
#       TestIDPhysValMon.pixHitsOffline = 1
        if (doFS == True):
          TestIDPhysValMon.sctHitsOffline = 6
          TestIDPhysValMon.pixHitsOffline = 4
          TestIDPhysValMon.blayerHitsOffline = 1
          TestIDPhysValMon.pixHolesOffline = 1
      elif (rec.doTruth == True):        
        TestIDPhysValMon.mcTruth = True
        TestIDPhysValMon.ntupleChainNames = ['Truth']
        TestIDPhysValMon.sctHitsOffline = -1
        TestIDPhysValMon.pixHitsOffline = -1
        if (pdgid != 0): TestIDPhysValMon.SelectTruthPdgId = pdgid
      elif (cosmic):
        TestIDPhysValMon.mcTruth = False
        TestIDPhysValMon.ntupleChainNames = ['Offline']
        TestIDPhysValMon.sctHitsOffline = -1
        TestIDPhysValMon.pixHitsOffline = -1

      TestIDPhysValMon.ntupleChainNames += chainnames
      TestIDPhysValMon.releaseMetaData = d['nightly name'] + " " + d['nightly release'] + " " + d['date'] + " " + d['platform'] + " " + d['release']

      #from AthenaCommon.AppMgr import ToolSvc
      #ToolSvc += TestIDPhysValMon
      #Monname = "TrigTestPhysValMon/" + Monname
      return TestIDPhysValMon

    ############### Electrons ###############
    name = "Electron"
    pdgid = 11
    useHighestPT = True
    if mt_chains:
      chainnames = [
        "HLT_e.*idperf.*:key=HLT_xAODTracks_Electron",
        "HLT_e.*etcut.*:key=HLT_xAODTracks_Electron"
      ]
    else:
      chainnames = [
        "HLT_e.*idperf.*:InDetTrigTrackingxAODCnv_Electron_IDTrig",
        "HLT_e.*idperf.*:InDetTrigTrackingxAODCnv_Electron_FTF"
      ]
    outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT )]

    name = "Electron_offline"
    pdgid = 11
    useHighestPT = True
    cosmic=False
    useOffline=True
    if mt_chains:
      chainnames = [
        "HLT_e.*idperf.*:key=HLT_xAODTracks_Electron",
        "HLT_e.*etcut.*:key=HLT_xAODTracks_Electron"
      ]
    else:      
      chainnames = [
        "HLT_e.*idperf.*:InDetTrigTrackingxAODCnv_Electron_IDTrig",
        "HLT_e.*idperf.*:InDetTrigTrackingxAODCnv_Electron_FTF"
      ]
    outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT, cosmic, useOffline )]



    ############### Muons ###############
    name = "Muon"
    pdgid = 13
    useHighestPT = True
    if mt_chains:
      chainnames = [
        "HLT_mu.*idperf.*:key=HLT_xAODTracks_Muon"
      ]
    else:
      chainnames = [
        "HLT_mu.*idperf.*:InDetTrigTrackingxAODCnv_Muon_IDTrig",
        "HLT_mu.*idperf.*:InDetTrigTrackingxAODCnv_Muon_FTF"
      ]

    outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT )]


    name = "Muon_offline"
    pdgid = 13
    useHighestPT = True
    cosmic=False
    useOffline=True
    if mt_chains:
      chainnames = [
        "HLT_mu.*idperf.*:key=HLT_xAODTracks_Muon"
      ]
    else:
      chainnames = [
        "HLT_mu.*idperf.*:InDetTrigTrackingxAODCnv_Muon_IDTrig",
        "HLT_mu.*idperf.*:InDetTrigTrackingxAODCnv_Muon_FTF"
      ]
    outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT, cosmic, useOffline )]


    ############### Taus ###############
    name = "Tau"
    pdgid = 15
    useHighestPT = True
    if mt_chains:
      chainnames = [
        "HLT_tau.*tracktwo.*idperf.*:key=HLT_xAODTracks_TauCore",
        "HLT_tau.*tracktwo.*idperf.*:key=HLT_xAODTracks_TauIso",
        "HLT_tau.*tracktwo.*idperf.*:key=HLT_xAODTracks_Tau"
      ]
    else:
      chainnames = [
        "HLT_tau.*idperf.*:key=InDetTrigTrackingxAODCnv_Tau_IDTrig:roi=forID3",
        "HLT_tau.*idperf.*:key=InDetTrigTrackingxAODCnv_Tau_FTF:roi=forID",
        "HLT_tau.*idperf.*:key=InDetTrigTrackingxAODCnv_TauCore_FTF:roi=forID1",
        "HLT_tau.*idperf.*:key=InDetTrigTrackingxAODCnv_TauIso_FTF:roi=forID3"
      ]
    outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT )]


    ############### Taus ###############
    name = "Tau_offline"
    pdgid = 15
    useHighestPT = True
    cosmic=False
    useOffline=True
    if mt_chains:
      chainnames = [
        "HLT_tau.*tracktwo.*idperf.*:key=HLT_xAODTracks_TauCore",
        "HLT_tau.*tracktwo.*idperf.*:key=HLT_xAODTracks_TauIso",
        "HLT_tau.*tracktwo.*idperf.*:key=HLT_xAODTracks_Tau"
      ]
    else:
      chainnames = [
        "HLT_tau.*idperf.*:key=InDetTrigTrackingxAODCnv_Tau_IDTrig:roi=forID3",
        "HLT_tau.*idperf.*:key=InDetTrigTrackingxAODCnv_Tau_FTF:roi=forID",
        "HLT_tau.*idperf.*:key=InDetTrigTrackingxAODCnv_TauCore_FTF:roi=forID1",
        "HLT_tau.*idperf.*:key=InDetTrigTrackingxAODCnv_TauIso_FTF:roi=forID3"
      ]
    outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT, cosmic, useOffline )]

    if not mt_chains:
      ############### Bjets ###############
      name = "Bjet"
      pdgid = 5
      useHighestPT = False
      chainnames = [
        "HLT_j.*b.*perf_split:key=InDetTrigTrackingxAODCnv_BjetPrmVtx_FTF:roi=TrigSuperRoi",
        "HLT_j.*b.*perf_split:InDetTrigTrackingxAODCnv_Bjet_IDTrig",
        "HLT_j.*b.*perf_split:InDetTrigTrackingxAODCnv_Bjet_FTF",
        "HLT_mu.*b.*perf_dr05:key=InDetTrigTrackingxAODCnv_BjetPrmVtx_FTF:roi=TrigSuperRoi",
        "HLT_mu.*b.*perf_dr05:InDetTrigTrackingxAODCnv_Bjet_IDTrig",
        "HLT_mu.*b.*perf_dr05:InDetTrigTrackingxAODCnv_Bjet_FTF"
      ]
      outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT )]
      
      ############### Bjets ###############
      name = "Bjet_offline"
      pdgid = 5
      useHighestPT = False
      cosmic=False
      useOffline=True
      chainnames = [
        "HLT_j.*b.*perf_split:key=InDetTrigTrackingxAODCnv_BjetPrmVtx_FTF:roi=TrigSuperRoi",
        "HLT_j.*b.*perf_split:InDetTrigTrackingxAODCnv_Bjet_IDTrig",
        "HLT_j.*b.*perf_split:InDetTrigTrackingxAODCnv_Bjet_FTF",
        "HLT_mu.*b.*perf_dr05:key=InDetTrigTrackingxAODCnv_BjetPrmVtx_FTF:roi=TrigSuperRoi",
        "HLT_mu.*b.*perf_dr05:InDetTrigTrackingxAODCnv_Bjet_IDTrig",
        "HLT_mu.*b.*perf_dr05:InDetTrigTrackingxAODCnv_Bjet_FTF"
      ]
      outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT, cosmic, useOffline )]
      
      ############### Bphys ###############
      name = "Bphys"
      pdgid = 0 # Doesn't make sense
      useHighestPT = False
      chainnames = [
        "HLT_.*Bmumux.*:InDetTrigTrackingxAODCnv_Bphysics_IDTrig",
        "HLT_.*Bmumux.*:InDetTrigTrackingxAODCnv_Bphysics_FTF"
      ]
      outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT )]
      
      ############### Bphys ###############
      name = "Bphys_offline"
      pdgid = 0 # Doesn't make sense
      useHighestPT = False
      cosmic=False
      useOffline=True
      chainnames = [
        "HLT_.*Bmumux.*:InDetTrigTrackingxAODCnv_Bphysics_IDTrig",
        "HLT_.*Bmumux.*:InDetTrigTrackingxAODCnv_Bphysics_FTF"
      ]
      outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT, cosmic, useOffline )]
      
      ############### Cosmics ###############
      name = "Cosmic"
      useHighestPT = False
      pdgid = 0 # Not used for cosmic
      chainnames = [
        'HLT_.*id.*cosmic.*:InDetTrigTrackingxAODCnvIOTRT_CosmicsN_EFID',
        'HLT_.*id.*cosmic.*:InDetTrigTrackingxAODCnv_CosmicsN_EFID'
      ]
      outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT, cosmic=True)]
      
  return outputlist
