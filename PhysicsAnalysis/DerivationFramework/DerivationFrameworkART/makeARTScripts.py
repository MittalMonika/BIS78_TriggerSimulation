import os

makeDataDAODs=True
makeMCDAODs=True
makeTruthDAODs=True
makeTrains=True

formatList = [#'PHYSVAL', 
              'TOPQ1', 'TOPQ2', 'TOPQ3', 'TOPQ4', 'TOPQ5', 
              'HIGG1D1', 'HIGG1D2',  
              'HIGG2D1', 'HIGG2D2', 'HIGG2D4', 'HIGG2D5', 
              'HIGG3D1', 'HIGG3D2', 'HIGG3D3', 
              'HIGG4D1', 'HIGG4D2', 'HIGG4D3', 'HIGG4D4', 'HIGG4D5','HIGG4D6', 
              'HIGG5D1', 'HIGG5D2', 'HIGG5D3', 
              'HIGG6D1', 'HIGG6D2', 
              'HIGG8D1',  
              'STDM2', 'STDM3', 'STDM4', 'STDM5', 'STDM6', 'STDM7', 'STDM8', 'STDM9', 
              'TAUP1', 'TAUP2', 'TAUP3', 'TAUP4', 
              'SUSY1', 'SUSY2', 'SUSY3', 'SUSY4', 'SUSY5', 'SUSY6', 'SUSY7', 'SUSY8', 'SUSY10', 'SUSY11', 'SUSY12', 'SUSY13', 'SUSY14', 'SUSY15', 'SUSY16', 'SUSY17', 'SUSY18', 
              'EXOT0', 'EXOT2', 'EXOT3', 'EXOT4', 'EXOT5', 'EXOT6', 'EXOT7', 'EXOT8', 'EXOT9', 'EXOT10', 'EXOT12', 'EXOT13', 'EXOT14', 'EXOT15', 'EXOT17', 'EXOT18', 'EXOT19', 'EXOT20', 'EXOT21', 
              'JETM1', 'JETM2', 'JETM3', 'JETM4', 'JETM5', 'JETM6', 'JETM7', 'JETM8', 'JETM9', 'JETM10', 'JETM11', 
              'IDTR1', 
              'EGAM1', 'EGAM2', 'EGAM3', 'EGAM4', 'EGAM5', 'EGAM6', 'EGAM7', 'EGAM8', 'EGAM9', 
              'FTAG1', 'FTAG2', 'FTAG3', 'FTAG4',  
              'BPHY1', 'BPHY2', 'BPHY3', 'BPHY4', 'BPHY5', 'BPHY6', 'BPHY7', 'BPHY8', 'BPHY9', 'BPHY10', 'BPHY11', 'BPHY12', 
              'MUON0', 'MUON1', 'MUON2', 'MUON3', 'MUON4', 
              'TCAL1' 
]

truthFormatList = ['TRUTH0', 'TRUTH1', 'TRUTH3']

trainList = [ ["HIGG2D5","FTAG3","TCAL1","SUSY14"], # < 0.1%
              ["JETM10","BPHY10","EGAM4","EXOT10","STDM5","MUON2"], # 0.1-0.1%
              ["SUSY13","HIGG1D2","HIGG3D2","EXOT12","EGAM9"], # 0.16-0.2%
              ["JETM3","SUSY12","HIGG4D4","EGAM3","SUSY2","EXOT0","EXOT17"], # 0.2-0.3%
              ["JETM7","BPHY5","MUON3","EXOT6","EGAM2","HIGG4D6"], # 0.35-0.4%
              ["BPHY4","BPHY7","HIGG4D5","EXOT15","EXOT9","BPHY1","EGAM7"], # 0.4-0.5%
              ["SUSY16","EXOT20","HIGG1D1","STDM3","TOPQ2"], # 0.5-0.6%
              ["SUSY18","JETM4","HIGG4D1","HIGG6D1","SUSY7"], # 0.6-0.7%
              ["HIGG4D3","EXOT18","STDM2","STDM7","SUSY3","EXOT5"], # 0.8-1%
              ["JETM2","SUSY11","EXOT3","SUSY4","JETM6","EXOT19","EGAM1"], # 1-1.3%
              ["JETM1","HIGG2D4","EGAM8","SUSY1","SUSY5","HIGG8D1","EXOT21"], # 1.3-1.6%
              ["STDM9","HIGG5D3","BPHY9","HIGG5D1","BPHY6"], # 1.6-1.8%
              ["EGAM5","JETM11","SUSY8","HIGG2D2","EXOT14","SUSY10"], # 1.9-2.1%
              ["HIGG3D3","EXOT4","TAUP1","EXOT13","EXOT2","HIGG4D2"], # 2.1-2.7%
              ["MUON1","SUSY6","JETM9","EXOT8","FTAG1","TAUP4"], # 2.9-3.4%
              ["HIGG3D1","STDM4","HIGG2D1","HIGG5D2"], # 3.6-3.7%
              ["TOPQ1","FTAG2","TOPQ3","FTAG4"], # 4-5%
              ["HIGG6D2","MUON0","TAUP3","TOPQ4"] ] # 5-7%
            

mcLabel = "mc16"
dataLabel = "data17"
truthLabel = "mc15"
mcFile = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.11866988._000378.pool.root.1"
mcFileBPHY8 = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.11455791._000001.pool.root.1"
dataFile = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/data17_13TeV.00327342.physics_Main.merge.AOD.f838_m1824._lb0300._0001.1"
truthFile = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/EVNT.05192704._020091.pool.root.1"
dataPreExec = "--preExec \'rec.doApplyAODFix.set_Value_and_Lock(True);from BTagging.BTaggingFlags import BTaggingFlags;BTaggingFlags.CalibrationTag = \"BTagCalibRUN12Onl-08-40\" \' "
mcPreExec = "--preExec \'rec.doApplyAODFix.set_Value_and_Lock(True);from BTagging.BTaggingFlags import BTaggingFlags;BTaggingFlags.CalibrationTag = \"BTagCalibRUN12-08-40\" \' "

def generateText(formatName,label,inputFile,isTruth,isMC):
   outputFileName = "test_"+label+formatName+".sh" 
   outputFile = open(outputFileName,"w")
   outputFile.write("#!/bin/sh"+"\n")
   outputFile.write("\n")
   outputFile.write("# art-description: DAOD building "+formatName+" "+label+"\n")
   outputFile.write("# art-type: grid"+"\n")
   outputFile.write("# art-output: *.pool.root"+"\n")
   outputFile.write("\n")
   outputFile.write("set -e"+"\n")
   outputFile.write("\n")
   if ((isTruth==False) and (isMC==False) ): outputFile.write("Reco_tf.py --inputAODFile "+inputFile+" --outputDAODFile art.pool.root --reductionConf "+formatName+" --maxEvents 8000 "+dataPreExec+"\n")
   if ((isTruth==False) and (isMC==True) ): outputFile.write("Reco_tf.py --inputAODFile "+inputFile+" --outputDAODFile art.pool.root --reductionConf "+formatName+" --maxEvents 10000 "+mcPreExec+"\n")
   if (isTruth==True): outputFile.write("Reco_tf.py --inputEVNTFile "+inputFile+" --outputDAODFile art.pool.root --reductionConf "+formatName+" --maxEvents 1000"+"\n")
   outputFile.write("\n")
   if (isTruth==False): outputFile.write("DAODMerge_tf.py --maxEvents 5 --inputDAOD_"+formatName+"File DAOD_"+formatName+".art.pool.root --outputDAOD_"+formatName+"_MRGFile art_merged.pool.root"+"\n")
   if (isTruth==True): outputFile.write("DAODMerge_tf.py --maxEvents 5 --inputDAOD_"+formatName+"File DAOD_"+formatName+".art.pool.root --outputDAOD_"+formatName+"_MRGFile art_merged.pool.root"+" --autoConfiguration ProjectName RealOrSim BeamType ConditionsTag DoTruth InputType BeamEnergy LumiFlags TriggerStream --athenaopts=\"-s\" "+"\n")
   outputFile.write("checkFile.py DAOD_"+formatName+".art.pool.root > checkFile.txt")
   outputFile.close()
   os.system("chmod +x "+outputFileName)

def generateTrains(formatList,label,inputFile,isMC):
   outputFileName = "test_"+label+"_".join(formatList)+".sh"
   outputFile = open(outputFileName,"w")
   outputFile.write("#!/bin/sh"+"\n")
   outputFile.write("\n")
   outputFile.write("# art-description: DAOD building "+" ".join(formatList)+" "+label+"\n")
   outputFile.write("# art-type: grid"+"\n")
   outputFile.write("# art-output: *.pool.root"+"\n")
   outputFile.write("\n")
   outputFile.write("set -e"+"\n")
   outputFile.write("\n")
   if (isMC == False): outputFile.write("Reco_tf.py --inputAODFile "+inputFile+" --outputDAODFile art.pool.root --reductionConf "+" ".join(formatList)+" --maxEvents 500 "+dataPreExec+" --passThrough True "+"\n")
   if (isMC == True): outputFile.write("Reco_tf.py --inputAODFile "+inputFile+" --outputDAODFile art.pool.root --reductionConf "+" ".join(formatList)+" --maxEvents 500 "+mcPreExec+" --passThrough True "+"\n")
   os.system("chmod +x "+outputFileName)


if (makeDataDAODs or makeMCDAODs):
   for formatName in formatList:
      if (makeDataDAODs): generateText(formatName,dataLabel,dataFile,False,False)
      if (makeMCDAODs):
         if formatName=="BPHY8":
            generateText(formatName,mcLabel,mcFileBPHY8,False,True)      
         else: generateText(formatName,mcLabel,mcFile,False,True)

if (makeTruthDAODs):
   for formatName in truthFormatList:
      generateText(formatName,truthLabel,truthFile,True,False)

if (makeTrains):
   for train in trainList:
      generateTrains(train,dataLabel,dataFile,False)
      #generateTrains(train,mcLabel,mcFile,True)   



