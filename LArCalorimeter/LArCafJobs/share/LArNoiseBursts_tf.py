#!/usr/bin/env python
__doc__ = """JobTransform to run LAr Noise Burst jobs"""


import sys
from PyJobTransforms.transform import transform
from PyJobTransforms.trfExe import athenaExecutor
from PyJobTransforms.trfArgs import addAthenaArguments, addDetectorArguments
import PyJobTransforms.trfArgClasses as trfArgClasses

if __name__ == '__main__':

    executorSet = set()
    executorSet.add(athenaExecutor(name = 'LArNoiseBursts_from_raw', skeletonFile = 'LArCafJobs/skeleton.LArNoise.py',
                                   substep = 'r2e', inData = ['BS',], outData = ['NTUP_LARNOISE','NTUP_HECNOISE']))
    executorSet.add(athenaExecutor(name = 'LArNoiseBursts', skeletonFile = 'LArCafJobs/skeleton.LArNoise.py',
                                   substep = 'e2a', inData = ['ESD',], outData = ['NTUP_LARNOISE','NTUP_HECNOISE']))
   
    trf = transform(executor = executorSet) 
    addAthenaArguments(trf.parser)
    addDetectorArguments(trf.parser)
    trf.parser.add_argument('--inputESDFile', nargs='+',
                            type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='input'),
                            help='Input pool file', group='Reco Files')
   
    trf.parser.add_argument('--inputBSFile', nargs='+',
                            type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='input'),
                            help='Input BS file', group='Reco Files')
   
    trf.parser.add_argument('--outputNTUP_LARNOISEFile', nargs='+',
                            type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile, io='output'),
                            help='Output LAr Noise Burst file', group='Ntuple Files')

    trf.parser.add_argument('--outputNTUP_HECNOISEFile', nargs='+',
                            type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile, io='output'),
                            help='Output HECNoise file', group='Ntuple Files')

    trf.parseCmdLineArgs(sys.argv[1:])


    trf.execute()
    print "DataDict:"
    print trf.dataDictionary


    trf.generateReport()
