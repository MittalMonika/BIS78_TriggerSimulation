#!/bin/sh
#
# art-description: Generate SQL files for muon spectrometer description, just AMDB part from amdc_simrec
#
# art-type: grid
# art-include: master/Athena
# art-include: 21.3/Athena
#
# art-output: *.txt

art.py createpoolfile

set -x

wget http://atlas.web.cern.ch/Atlas/GROUPS/MUON/AMDB/amdb_simrec.r.08.01
athena.py AmdcAth/AmdcAth_GeometryTasks.py -c "input_amdb_simrec='amdb_simrec.r.08.01';DoAMDBTables=True;"

echo "art-result: $?"
