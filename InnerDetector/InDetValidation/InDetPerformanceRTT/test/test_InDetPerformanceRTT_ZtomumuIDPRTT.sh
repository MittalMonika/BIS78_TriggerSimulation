#!/bin/bash
# art-description: art job for InDetPerformanceRTT, ZtoMuMU
# art-type: grid
# art-input: mc15_13TeV.361107.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zmumu.recon.RDO.e3601_s2576_s2132_s2132_r7509_tid07497175_00 
# art-input-nfiles: 1
# art-include: 21.0/Athena
# art-include: master/Athena
# art-include: 21.9/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube

fileList="['${ArtInFile//,/', '}']"
echo "List of files = ", $fileList

joboptions="InDetPerformanceRTT/InDetPerformanceRTT_jobOptions.py"
set -x

get_files -jo $joboptions
ls -lR
athena.py  -c "from AthenaCommon.AthenaCommonFlags import athenaCommonFlags; athenaCommonFlags.FilesInput=$fileList; EventMax=500;" $joboptions
echo  "art-result: $? reco"

dcubeName="ZMuMu"
dcubeXml="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPerformanceRTT/dcube/config/ZMuMu_DCubeConfig.xml"
dcubeRef="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPerformanceRTT/ReferenceHistograms/InDetStandardPlots-ZMuMu.root"

bash /cvmfs/atlas.cern.ch/repo/sw/art/dcube/bin/art-dcube $dcubeName InDetStandardPlots.root $dcubeXml $dcubeRef
echo  "art-result: $? plot"

