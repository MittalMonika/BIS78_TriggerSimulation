#!/bin/bash
# art-description: art job for InDetPerformanceRTT, Single Pion 100GeV
# art-type: grid
# art-input: mc15_13TeV.422015.ParticleGun_single_pion_Pt100.recon.RDO.e4459_s2726_r7059_tid07275536_00
# art-input-nfiles: 10
# art-include: 21.0/Athena
# art-include: master/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube
# art-output: *.png

fileList="['${ArtInFile//,/', '}']"
echo "List of files = ", $fileList

joboptions="InDetPerformanceRTT/InDetPerformanceRTT_jobOptions.py"
set -x

get_files -jo $joboptions
ls -lR

athena.py  -c "from AthenaCommon.AthenaCommonFlags import athenaCommonFlags; athenaCommonFlags.FilesInput=$fileList; EventMax=20000;" $joboptions
echo  "art-result: $? reco"

dcubeName="Pi100"
dcubeXml="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPerformanceRTT/dcube/config/Pi100_DCubeConfig.xml"
dcubeRef="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPerformanceRTT/ReferenceHistograms/InDetStandardPlots-SinglePi100000.root"

bash /cvmfs/atlas.cern.ch/repo/sw/art/dcube/bin/art-dcube $dcubeName InDetStandardPlots.root $dcubeXml $dcubeRef
echo  "art-result: $? plot"

plotmacro="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPerformanceRTT/scripts/makeplots.C"
root -b -q $plotmacro
echo "art-result: $? root"
