#! /usr/bin/env python

# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

## Overlay chain transform
# @version $Id: OverlayChain_tf.py 529035 2012-12-05 15:45:24Z graemes $

import sys
import time

import logging

# Setup core logging here
from PyJobTransforms.trfLogger import msg
msg.info('logging set in %s' % sys.argv[0])

from PyJobTransforms.transform import transform
from PyJobTransforms.trfExe import athenaExecutor, DQMergeExecutor
from PyJobTransforms.trfArgs import addAthenaArguments, addDetectorArguments
from PyJobTransforms.trfDecorators import stdTrfExceptionHandler, sigUsrStackTrace

import PyJobTransforms.trfArgClasses as trfArgClasses

# Prodsys hack...
ListOfDefaultPositionalKeys=['--AFPOn', '--ALFAOn', '--AMIConfig', '--AMITag', '--AddCaloDigi', '--CosmicFilterVolume', '--CosmicFilterVolume2', '--CosmicPtSlice', '--DBRelease', '--DataRunNumber', '--FwdRegionOn', '--LucidOn', '--ReadByteStream', '--ZDCOn', '--argJSON', '--asetup', '--athena', '--athenaMPMergeTargetSize', '--athenaopts', '--attempt', '--beamType', '--checkEventCount', '--command', '--conditionsTag', '--cscCondOverride', '--digiRndmSvc', '--digiSeedOffset1', '--digiSeedOffset2', '--digiSteeringConf', '--doAllNoise', '--dumpJSON', '--dumpPickle', '--enableLooperKiller', '--env', '--eventAcceptanceEfficiency', '--eventIdFile', '--execOnly', '--fSampltag', '--fileValidation', '--firstEvent', '--geometryVersion', '--ignoreErrors', '--ignoreFiles', '--ignorePatterns', '--imf', '--inputBSFile', '--inputBS_SKIMFile', '--inputEVNTFile', '--inputEVNT_CAVERNFile', '--inputEVNT_COSMICSFile', '--inputFileValidation', '--inputHITSFile', '--jobNumber', '--jobid', '--lumiBlockMapFile', '--maxEvents', '--maxFilesPerSubjob', '--muonForceUse', '--orphanKiller', '--outputBS_SKIMFile', '--outputEVNT_CAVERNTRFile', '--outputEVNT_COSMICSTRFile', '--outputFileValidation', '--outputHITSFile', '--outputRDOFile', '--outputRDO_FILTFile', '--outputRDO_SGNLFile', '--overlayConfigFile', '--parallelFileValidation', '--physicsList', '--postExec', '--postInclude', '--preExec', '--preInclude', '--randomSeed', '--reportName', '--reportType', '--runNumber', '--samplingFractionDbTag', '--showGraph', '--showPath', '--showSteps', '--simulator', '--skipEvents', '--skipFileValidation', '--skipInputFileValidation', '--skipOutputFileValidation', '--steering', '--taskid', '--tcmalloc', '--tmpRDO', '--tmpRDO_FILT', '--triggerBit', '--truthStrategy', '--useISF', '--valgrind', '--valgrindbasicopts', '--valgrindextraopts']

@stdTrfExceptionHandler
@sigUsrStackTrace
def main():

    msg.info('This is %s' % sys.argv[0])

    trf = getTransform()
    trf.parseCmdLineArgs(sys.argv[1:])
    trf.execute()
    trf.generateReport()

    msg.info("%s stopped at %s, trf exit code %d" % (sys.argv[0], time.asctime(), trf.exitCode))
    sys.exit(trf.exitCode)

def getTransform():
    executorSet = set()
    from EventOverlayJobTransforms.overlayTransformUtils import addOverlayBSFilterSubstep, addOverlay_BSSubstep, addOverlayBSFilterArguments, addOverlay_BSArguments, addOverlayChainOverrideArguments
    from SimuJobTransforms.SimTransformUtils import addSimulationSubstep, addSimulationArguments
    addOverlayBSFilterSubstep(executorSet)
    addSimulationSubstep(executorSet, overlayTransform = True)
    addOverlay_BSSubstep(executorSet)
    trf = transform(executor = executorSet, description = 'Full Overlay Chain')
    addOverlayChainOverrideArguments(trf.parser)
    addAthenaArguments(trf.parser, maxEventsDefaultSubstep='all')
    addDetectorArguments(trf.parser)
    addOverlayBSFilterArguments(trf.parser)
    addSimulationArguments(trf.parser)
    addOverlay_BSArguments(trf.parser)

    return trf

if __name__ == '__main__':
    main()
