from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
from AthenaCommon.GlobalFlags import globalflags
if globalflags.DataSource()=='data':
  BTaggingFlags.CalibrationFolderRoot=BTaggingFlags.CalibrationFolderRoot.replace("/GLOBAL/BTagCalib","/GLOBAL/Onl/BTagCalib")
  connSchema="GLOBAL"
  message = "#BTAG# accessing online conditions DB"
  if not athenaCommonFlags.isOnline():
    message = message + " (via offline replica)"
  print message
else:
  connSchema="GLOBAL_OFL"
  print "#BTAG# accessing offline conditions DB"

print "#BTAG# running in reco mode -> btag calibration root folder is CalibrationFolderRoot =", BTaggingFlags.CalibrationFolderRoot

JetCollectionForCalib = []
JetCollectionForCalib+= BTaggingFlags.Jets
if "AntiKt4EMTopo" not in JetCollectionForCalib:
  JetCollectionForCalib+=["AntiKt4EMTopo"]

print "BTAG channel"
print BTaggingFlags.CalibrationChannelAliases

#IP2D
grades= [ "0HitIn0HitNInExp2","0HitIn0HitNInExpIn","0HitIn0HitNInExpNIn","0HitIn0HitNIn",
                  "0HitInExp", "0HitIn",
                  "0HitNInExp", "0HitNIn",
                  "InANDNInShared", "PixShared", "SctShared",
                  "InANDNInSplit", "PixSplit",
                  "Good"]

from JetTagCalibration.JetTagCalibrationConf import Analysis__JetTagCalibCondAlg
JetTagCalibrationCondAlg = Analysis__JetTagCalibCondAlg(
  name = "JetTagCalibrationCondAlg",
  taggers = [],
  channels = JetCollectionForCalib,
  channelAliases = BTaggingFlags.CalibrationChannelAliases,
  IP2D_TrackGradePartitions = grades,
  RNNIP_NerworkConfig = BTaggingFlags.RNNIPConfig,
  OutputLevel = BTaggingFlags.OutputLevel
)
ToolSvc += JetTagCalibrationCondAlg
theApp.Dlls+=['DetDescrCondExample','DetDescrCondTools']

from IOVDbSvc.CondDB import conddb

# with new scheme, only one actual COOL folder:
folder = BTaggingFlags.CalibrationFolderRoot + 'RUN12'

if BTaggingFlags.CalibrationFromLocalReplica:
  dbname="OFLP200"
  #For data, use COMP200 for Run 1, CONDBR2 for Run 2
  if globalflags.DataSource()=='data':
    dbname=conddb.dbdata
  conddb.addFolder("","<dbConnection>sqlite://X;schema=mycool.db;dbname="+dbname+"</dbConnection> "+folder+" <tag>"+BTaggingFlags.CalibrationTag+"</tag>")
else:
  if BTaggingFlags.CalibrationFromCERN:
    conddb.addFolder("","<dbConnection>oracle://ATLAS_COOLPROD;schema=ATLAS_COOLOFL_GLOBAL;dbname=OFLP200</dbConnection> "+folder+" <tag>"+BTaggingFlags.CalibrationTag+"</tag>") 
  else: 
    if BTaggingFlags.CalibrationTag == "":
      conddb.addFolder(connSchema,folder) 
    else:
      conddb.addFolder(connSchema,folder+" <tag>"+BTaggingFlags.CalibrationTag+"</tag>") 

if BTaggingFlags.OutputLevel < 3: 
  print JetTagCalibrationCondAlg

