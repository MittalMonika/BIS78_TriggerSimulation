# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

#
#  provisionally a copy of InDetRecExample
#

from AthenaCommon.Include import include

include.block("InDetTrigRecExample/EFInDetConfig.py")

include("InDetTrigRecExample/InDetTrigRec_jobOptions.py") # this is needed to get InDetTrigFlags

from AthenaCommon.Logging import logging 
log = logging.getLogger("TrigInDetSequence.py")

from InDetTrigRecExample.InDetTrigConfigRecPreProcessing import *

from InDetTrigRecExample.InDetTrigConfigRecNewTracking import \
     SiTrigTrackFinder_EF, SiTrigTrackSeededFinder_EF, SiTrigSimpleTrackFinder_EF, \
     TrigAmbiguitySolver_EF,TRTTrackExtAlg_EF,\
     TrigExtProcessor_EF, InDetTrigDetailedTrackTruthMaker_EF

from InDetTrigRecExample.InDetTrigConfigRecNewTracking import SimpleTrigTrackCollMerger_EF
  
from InDetTrigRecExample.InDetTrigConfigRecBackTracking import *
from InDetTrigRecExample.InDetTrigConfigRecPostProcessing import *

from InDetTrigRecExample.InDetTrigRecLowPtTracking \
     import SiTrigSpacePointFinderLowPt_EF, SiTrigTrackFinderLowPt_EF, \
     TrigAmbiguitySolverLowPt_EF

from TrigInDetConf.RoiManipulators import IDTrigRoiUpdater
from TrigInDetConf.TrackingAlgCfgble import TrigFastTrackFinder

from InDetTrigRecExample.InDetTrigFlags import InDetTrigFlags


class TrigInDetSequenceBase:
  """
  """
  def __init__(self):
    self.__sequence__ = list()
  def getSequence(self):
    return self.__sequence__

class TrigInDetSequence(TrigInDetSequenceBase):
  """

  Create InDet reconstruction sequences from a textual input rather
  than one class per sequence. It also allows users to create multiple
  instances of a sequence without modifying this file The class has
  three input parameters
  
  """

  def _get_class_name(self, algname):
    efidclasses = [ "PixelClustering","SCTClustering", "TRTDriftCircleMaker",
                    "InDetTrigPRD_MultiTruthMaker", 
                    "SiTrigSpacePointFinder", "SiTrigTrackFinder", "TrigAmbiguitySolver",
                    "TRTTrackExtAlg", "TrigExtProcessor",
                    "InDetTrigTrackSlimmer",  "InDetTrigTrackingxAODCnv",
                    "InDetTrigDetailedTrackTruthMaker",
                    "TrigVxPrimary",
                    "InDetTrigParticleCreation",
                    "InDetTrigTrackParticleTruthMaker",
                    "InDetTrigVertexxAODCnv"
                    ]

    clname = algname
    if algname in efidclasses:
      clname =  "%s_EF" % algname

    return clname



  def _item_line(self, algName, instance):
    seqType = self.__sequenceType__
    seqName = self.__signatureName__
    if instance=="":
      if "Fast" in seqType:
        _inst = algName+'_%s_FTF'
      elif "L2Star" in seqType:
        _inst = algName+'_%s_L2ID'
      elif "IDTrig" in seqType:
        _inst = algName+'_%s_IDTrig'
      else:
        _inst = algName+'_%s_EFID'
    else:
      _inst = instance

    if '%s' in  _inst:
      seqinstance = _inst % seqName
    else:
      seqinstance = _inst

    pyline = _line = "%s(\"%s\",\"%s\")" % (self._get_class_name(algName),seqinstance,self.__signature__)
    return pyline



  def removeTruth(self):
    pass

  def generatePyCode(self, algos):
    #create python code to be executed
    alglist = "algseq = ["
    for i in algos:
      #self.__algos__.append(i)
      algline = self._item_line(i[0], i[1])
      alglist += algline+',' 
    alglist += "]"

    #this should be avoided by a higher level steering
    #    modify the sequence acoording to triggerflags
    from TriggerJobOpts.TriggerFlags  import TriggerFlags

    if not ( TriggerFlags.doEF() or TriggerFlags.doHLT() ) or not TriggerFlags.doFEX():
      from TrigSteeringTest.TrigSteeringTestConf import PESA__dummyAlgo as dummyAlgo_disabledByTriggerFlags_EFID
      dummyAlgEFID = dummyAlgo_disabledByTriggerFlags_EFID("doEF_or_doFEX_False_no_EFID")
      alglist = 'algseq = [dummyAlgEFID]'

    algseq = []
    try:
      exec alglist
    except:
      from sys import exc_info
      (a,reason,c) = exc_info()
      print reason
      log.error("Cannot create EFID sequence %s, leaving empty" % alglist)
      import traceback
      print traceback.format_exc()

      #raise Exception

    print 'algseq from generate ', algseq  

    self.__sequence__.append(algseq)
    pass


  def __init__(self,
               signatureName="Electron",
               signature="electron", 
               sequenceType="IDTrig",
               sequenceFlavour="Fast"):

    TrigInDetSequenceBase.__init__(self)
    self.__signatureName__ = signatureName
    self.__signature__     = signature
    self.__sequenceType__  = sequenceType
    self.__sequenceFlavour__  = sequenceFlavour
    self.__step__ = [signature]

    if self.__sequenceFlavour__ =="2step":
      if self.__signature__ == "tau":
        self.__step__ = ["tauCore","tauIso","tau"]; 
      elif self.__signature__ == "muon":
        self.__step__ = ["muonCore","muonIso","muon"]; 
      self.__step__.reverse()

    fullseq = list()

    log.info("TrigInDetSequence  sigName=%s seqFlav=%s sig=%s sequenceType=%s" % (signatureName, sequenceFlavour, signature, sequenceType))

    dataprep = [
      ("PixelClustering", "PixelClustering_IDTrig"),
      ("SCTClustering",   "SCTClustering_IDTrig"),
      ("SiTrigSpacePointFinder","SiTrigSpacePointFinder_IDTrig"),
      ]

    ftfname = ""
    roiupdater = ""
    if sequenceFlavour=="2step":
      ftfname = "TrigFastTrackFinder_%sCore"
      roiupdater = "IDTrigRoiUpdater_%sCore_IDTrig"



    if sequenceType=="IDTrig":

      algos = list()

      if sequenceFlavour != "FTF":
        algos += [("IDTrigRoiUpdater", roiupdater)]

      algos += dataprep
      algos += [("TrigFastTrackFinder",ftfname),
                ("InDetTrigTrackingxAODCnv","InDetTrigTrackingxAODCnv_%s_FTF"),
                ]
      fullseq.append(algos)
 

      if sequenceFlavour=="2step":
        algos = [("IDTrigRoiUpdater", "IDTrigRoiUpdater_%sIso_IDTrig")]
        algos += dataprep
        algos += [("TrigFastTrackFinder","TrigFastTrackFinder_%sIso"),
                  ("InDetTrigTrackingxAODCnv","InDetTrigTrackingxAODCnv_%s_FTF"),
                  ]
        fullseq.append(algos)


      if sequenceFlavour != "FTF":
        algos = [("TrigAmbiguitySolver",""),
                 ("TRTDriftCircleMaker",""),
                 ("InDetTrigPRD_MultiTruthMaker",""), 
                 ("TRTTrackExtAlg",""),
                 ("TrigExtProcessor",""),
                 ("InDetTrigTrackSlimmer",""),
                 ("InDetTrigTrackingxAODCnv",""),
                 ("InDetTrigDetailedTrackTruthMaker",""),
                 ("TrigVxPrimary",""),
                 #("InDetTrigParticleCreation",""),
                 #("InDetTrigTrackParticleTruthMaker",""),
                 ("InDetTrigVertexxAODCnv","")
                 ]
        fullseq.append(algos)

      
    elif sequenceType=="FastxAOD":
      algos = [("InDetTrigTrackingxAODCnv",""),
               ]
      fullseq.append(algos)
    elif sequenceType=="L2StarxAOD":
      algos = [("InDetTrigTrackingxAODCnv",""),
               ]
      fullseq.append(algos)
    else:
      pass

    log.info("Full sequence has %d items" % len(fullseq) )
    print fullseq
    log.info("generate python now")

    for i in fullseq:
      #print i
      print 'next item ', i
      if self.__step__:
        self.__signature__ = self.__step__.pop()
      self.generatePyCode(i)
      print self.__sequence__


