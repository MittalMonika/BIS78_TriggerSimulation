#!/bin/bash
# art-description: Generation test Pythia8 dijets 
# art-include: 21.6/AthGeneration
# art-type: build
# art-output: *.root
# art-output: log.generate

## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
Generate_tf.py --ecmEnergy=13000 --runNumber=361000 --maxEvents=5 \
    --jobConfig=/cvmfs/atlas.cern.ch/repo/sw/Generators/MC15JobOptions/latest/share/DSID361xxx/MC15.361000.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ0.py \
    --outputEVNTFile=test_jets.EVNT.pool.root \

echo "art-result: $? generate"
