#!/bin/bash

# art-description: DirectIOART Athena inputFile:RDO protocol=ROOT
# art-type: grid
# art-output: *.pool.root
# art-include: 21.0/Athena

set -e

Reco_tf.py --AMI q221 --inputRDOFile root://lcg-lrz-rootd.grid.lrz.de:1094/pnfs/lrz-muenchen.de/data/atlas/dq2/atlasdatadisk/rucio/mc16_13TeV/90/96/RDO.11373415._000001.pool.root.1 --outputRDO_TRIGFile art.pool.root

echo "art-result: $? DirectIOART_Athena_RecoTF_inputRDO_protocol_ROOT"