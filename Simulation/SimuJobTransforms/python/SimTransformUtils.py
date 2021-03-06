# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

## @brief Module with Digitization transform options and substep

import logging
msg = logging.getLogger(__name__)

import PyJobTransforms.trfArgClasses as trfArgClasses

from PyJobTransforms.trfExe import athenaExecutor

### Returns the toal number of needed events
def pileUpCalc(nSignalEvts, refreshRate, nSubEvtPerBunch,nBunches):
    totalSubEvts  = nBunches*nSubEvtPerBunch
    totalSubEvts += totalSubEvts*refreshRate*nSignalEvts
    return totalSubEvts


### Preparing the list of required input PU files
import math
def makeBkgInputCol(initialList, nBkgEvtsPerCrossing, correctForEmptyBunchCrossings, logger):
    uberList = []
    refreshrate = 1.0

    nSignalEvts = 1000
    from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
    if (athenaCommonFlags.EvtMax.get_Value()>0):
        nSignalEvts = int(athenaCommonFlags.EvtMax.get_Value())
        logger.info('Number of signal events (from athenaCommonFlags.EvtMax) = %s.', nSignalEvts )
    else:
        nSignalEvts = 0
        import PyUtils.AthFile as athFile
        for inFile in athenaCommonFlags.PoolHitsInput.get_Value():
            try:
                inputFile = athFile.fopen(inFile)
                nSignalEvts += int(inputFile.nentries)
                del inputFile
            except:
                logger.warning("Unable to open file [%s]"%inFile)
                logger.warning('caught:\n%s',err)
                import traceback
                traceback.print_exc()
        logger.info('Number of signal events (read from files) = %s.', nSignalEvts )

    nBkgEventsPerFile = 5000
    try:
        import PyUtils.AthFile as athFile
        inputFile = athFile.fopen(initialList[0])
        nBkgEventsPerFile = int(inputFile.nentries)
        logger.info('Number of background events per file (read from file) = %s.', nBkgEventsPerFile )
        del inputFile
    except:
        import traceback
        traceback.print_exc()
        logger.warning('Failed to count the number of background events in %s. Assuming 5000 - if this is an overestimate the job may die.', initialList[0])

    from Digitization.DigitizationFlags import digitizationFlags
    from AthenaCommon.BeamFlags import jobproperties
    Nbunches = 1 + digitizationFlags.finalBunchCrossing.get_Value() - digitizationFlags.initialBunchCrossing.get_Value()
    nbunches = int(Nbunches)
    if correctForEmptyBunchCrossings:
        nbunches = int(math.ceil(float(nbunches) * float(digitizationFlags.bunchSpacing.get_Value())/float(jobproperties.Beam.bunchSpacing.get_Value())))
    logger.info('Simulating a maximum of %s colliding-bunch crossings (%s colliding+non-colliding total) per signal event', nbunches, Nbunches)
    nBkgEventsForJob = pileUpCalc(float(nSignalEvts), 1.0, float(nBkgEvtsPerCrossing), nbunches)
    logger.info('Number of background events required: %s. Number of background events in input files: %s', nBkgEventsForJob, (nBkgEventsPerFile*len(initialList)) )
    numberOfRepetitionsRequired =float(nBkgEventsForJob)/float(nBkgEventsPerFile*len(initialList))
    NumberOfRepetitionsRequired = 1 + int(math.ceil(numberOfRepetitionsRequired))
    for i in range(0, NumberOfRepetitionsRequired):
        uberList+=initialList
    logger.info('Expanding input list from %s to %s', len(initialList), len(uberList))
    return uberList


### Add Argument Methods
def addCommonSimDigArguments(parser):
    from SimuJobTransforms.simTrfArgs import addForwardDetTrfArgs, addCommonSimDigTrfArgs
    addCommonSimDigTrfArgs(parser)
    addForwardDetTrfArgs(parser)

def addCommonSimulationArguments(parser):
    from SimuJobTransforms.simTrfArgs import addCommonSimTrfArgs, addCosmicsTrfArgs, addTrackRecordArgs
    addCommonSimTrfArgs(parser)
    addCosmicsTrfArgs(parser)
    addTrackRecordArgs(parser)

def addPureSimulationArguments(parser):
    addCommonSimulationArguments(parser)
    from SimuJobTransforms.simTrfArgs import addSim_tfArgs
    addSim_tfArgs(parser)

def addPureDigitizationArguments(parser):
    from SimuJobTransforms.simTrfArgs import addBasicDigiArgs, addPileUpTrfArgs
    addBasicDigiArgs(parser)
    addPileUpTrfArgs(parser)

def addSimulationArguments(parser):
    addCommonSimDigArguments(parser)
    addPureSimulationArguments(parser)

def addAtlasG4Arguments(parser):
    addCommonSimDigArguments(parser)
    addCommonSimulationArguments(parser)

def addHITSMergeArguments(parser):
    from SimuJobTransforms.simTrfArgs import addHITSMergeArgs
    addHITSMergeArgs(parser)

def addDigitizationArguments(parser):
    addCommonSimDigArguments(parser)
    addPureDigitizationArguments(parser)

def addHITSValidArguments(parser):
    from SimuJobTransforms.simTrfArgs import addHITSValidArgs
    addHITSValidArgs(parser)

def addRDOValidArguments(parser):
    from SimuJobTransforms.simTrfArgs import addRDOValidArgs
    addRDOValidArgs(parser)

### Add Sub-step Methods
## @brief Add ISF transform substep
#  @param overlayTransform If @c True use the tweaked version of in/outData for an overlay job
def addSimulationSubstep(executorSet, overlayTransform = False):
    TRExe = athenaExecutor(name = 'TRtoHITS', skeletonFile = 'SimuJobTransforms/skeleton.EVGENtoHIT_ISF.py',
                           substep = 'simTRIn', tryDropAndReload = False, perfMonFile = 'ntuple.pmon.gz',
                           inData=['EVNT_TR'],
                           outData=['HITS','NULL'] )
    executorSet.add(TRExe)
    SimExe = athenaExecutor(name = 'EVNTtoHITS', skeletonFile = 'SimuJobTransforms/skeleton.EVGENtoHIT_ISF.py',
                                   substep = 'sim', tryDropAndReload = False, perfMonFile = 'ntuple.pmon.gz',
                                   inData=['NULL','EVNT'],
                                   outData=['EVNT_TR','HITS','NULL'] )
    if overlayTransform:
        from PyJobTransforms.trfUtils import releaseIsOlderThan
        if releaseIsOlderThan(20,3):
            SimExe.inData = [('EVNT', 'BS_SKIM')]
        else:
            SimExe.inData = [('EVNT','TXT_EVENTID')]
        SimExe.outData = ['HITS']
        SimExe.inputDataTypeCountCheck = ['EVNT']
    executorSet.add(SimExe)

def addAtlasG4Substep(executorSet):
    executorSet.add(athenaExecutor(name = 'AtlasG4TfTRIn', skeletonFile = 'SimuJobTransforms/skeleton.EVGENtoHIT_MC12.py',
                                   substep = 'simTRIn', tryDropAndReload = False,
                                   inData=['EVNT_TR'],
                                   outData=['HITS','NULL'] ))
    executorSet.add(athenaExecutor(name = 'AtlasG4Tf', skeletonFile = 'SimuJobTransforms/skeleton.EVGENtoHIT_MC12.py',
                                   substep = 'sim', tryDropAndReload = False,
                                   inData=['NULL','EVNT'],
                                   outData=['EVNT_TR','HITS','NULL'] ))

def addConfigurableSimSubstep(executorSet, confName, extraSkeleton, confSubStep, confInData, confOutData, confExtraRunargs, confRuntimeRunargs):
    executorSet.add(athenaExecutor(name = confName, skeletonFile = extraSkeleton + ['SimuJobTransforms/skeleton.EVGENtoHIT_MC12.py'],
                                   substep = confSubStep, tryDropAndReload = False,
                                   inData = confInData,
                                   outData = confOutData, extraRunargs = confExtraRunargs, runtimeRunargs = confRuntimeRunargs ))

def addStandardHITSMergeSubstep(executorSet):
    executorSet.add(athenaExecutor(name = 'HITSMerge', substep="hitsmerge", skeletonFile = 'SimuJobTransforms/skeleton.HITSMerge.py',
                                              tryDropAndReload = False, inputDataTypeCountCheck = ['HITS']))

def addAFII_HITSMergeSubstep(executorSet):
    executorSet.add(athenaExecutor(name = 'HITSMerge', substep="hitsmerge", skeletonFile = 'SimuJobTransforms/skeleton.HITSMerge.py',
                                   tryDropAndReload = False,
                                   extraRunargs = {'preInclude': ['FastSimulationJobTransforms/jobConfig.v14_Parametrisation.py','FastCaloSimHit/preInclude.AF2Hit.py'],
                                                   'postInclude': ['FastCaloSimHit/postInclude.AF2FilterHitItems.py','FastSimulationJobTransforms/jobConfig.FastCaloSim_ID_cuts.py','FastSimulationJobTransforms/jobConfig.egamma_lateral_shape_tuning.config20.py']} ))

def addDigitizationSubstep(executorSet):
    executorSet.add(athenaExecutor(name = 'HITtoRDO', skeletonFile = 'SimuJobTransforms/skeleton.HITtoRDO.py',
                                              substep = 'h2r', tryDropAndReload = False,
                                              inData = ['HITS'], outData = ['RDO','RDO_FILT'], runtimeRunargs =
                                              {'LowPtMinbiasHitsFile' : 'runArgs.inputLowPtMinbiasHitsFile',
                                               'HighPtMinbiasHitsFile' : 'runArgs.inputHighPtMinbiasHitsFile',
                                               'cavernHitsFile' : 'runArgs.inputCavernHitsFile',
                                               'beamHaloHitsFile' : 'runArgs.inputBeamHaloHitsFile',
                                               'beamGasHitsFile' : 'runArgs.inputBeamGasHitsFile',}))

def addSimValidationSubstep(executorSet):
    executorSet.add(athenaExecutor(name = 'SimValidation',
                                           skeletonFile = 'SimuJobTransforms/skeleton.HITStoHIST_SIM.py',
                                           inData = ['HITS'], outData = ['HIST_SIM'],))

def addDigiValidationSubstep(executorSet):
    executorSet.add(athenaExecutor(name = 'DigiValidation',
                                           skeletonFile = 'SimuJobTransforms/skeleton.RDOtoHIST_DIGI.py',
                                           inData = ['RDO'], outData = ['HIST_DIGI'],))

### Append Sub-step Methods
def appendSimulationSubstep(trf):
    executor = set()
    addSimulationSubstep(executor)
    trf.appendToExecutorSet(executor)

def appendAtlasG4Substep(trf):
    executor = set()
    addAtlasG4Substep(executor)
    trf.appendToExecutorSet(executor)

def appendConfigurableSimTRInSubstep(trf, confName = 'AtlasG4TfTRIn',
                                 extraSkeleton = [], confSubstep = 'simTRIn',
                                 confInData=['EVNT_TR'],
                                 confOutData=['HITS','NULL'],
                                 confExtraRunargs=None, confRuntimeRunargs=None ):
    executor = set()
    addConfigurableSimSubstep(executor, confName, extraSkeleton, confSubStep, confInData, confOutData, confExtraRunargs, confRuntimeRunargs )
    trf.appendToExecutorSet(executor)

def appendConfigurableSimSubstep(trf, confName = 'AtlasG4Tf',
                                 extraSkeleton = [], confSubstep = 'sim',
                                 confInData=['NULL','EVNT'],
                                 confOutData=['EVNT_TR','HITS','NULL'],
                                 confExtraRunargs=None, confRuntimeRunargs=None ):
    executor = set()
    addConfigurableSimSubstep(executor, confName, extraSkeleton, confSubStep, confInData, confOutData, confExtraRunargs, confRuntimeRunargs )
    trf.appendToExecutorSet(executor)

def appendHITSMergeSubstep(trf):
    executor = set()
    addStandardHITSMergeSubstep(executor)
    trf.appendToExecutorSet(executor)

def appendAFII_HITSMergeSubstep(trf):
    executor = set()
    addAFII_HITSMergeSubstep(executor)
    trf.appendToExecutorSet(executor)

def appendDigitizationSubstep(trf):
    executor = set()
    addDigitizationSubstep(executor)
    trf.appendToExecutorSet(executor)

#def appendSimValidationSubstep(trf):
#   executor = set()
#  addSimValidationSubstep(executor)
# trf.appendSimVAlidationSubstep(executor)

#def appendDigiValidationSubstep(trf):
#   executor = set()
#  addDigiValidationSubstep(executor)
# trf.appendDigiVAlidationSubstep(executor)
