#!/bin/sh
# art-type: build
# art-ci: master

athena.py --threads=1 --evtMax=10 --filesInput="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data17_13TeV.00327265.physics_EnhancedBias.merge.RAW._lb0100._SFO-1._0001.1" -c 'doCalo=False' TrigUpgradeTest/IDCalo.py
