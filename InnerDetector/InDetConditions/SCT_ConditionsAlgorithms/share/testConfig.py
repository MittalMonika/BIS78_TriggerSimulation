import AthenaCommon.AtlasUnixStandardJob

#--------------------------------------------------------------
# Thread-specific setup
#--------------------------------------------------------------
from AthenaCommon.ConcurrencyFlags import jobproperties
if jobproperties.ConcurrencyFlags.NumThreads() > 0:
  from AthenaCommon.AlgScheduler import AlgScheduler
  AlgScheduler.CheckDependencies( True )
  AlgScheduler.ShowControlFlow( True )
  AlgScheduler.ShowDataDependencies( True )

# use auditors
from AthenaCommon.AppMgr import ServiceMgr
from GaudiSvc.GaudiSvcConf import AuditorSvc

ServiceMgr += AuditorSvc()
theAuditorSvc = ServiceMgr.AuditorSvc
theAuditorSvc.Auditors  += [ "ChronoAuditor"]
theAuditorSvc.Auditors  += [ "MemStatAuditor" ]
theApp.AuditAlgorithms=True

#--------------------------------------------------------------
# Load Geometry
#--------------------------------------------------------------
from AthenaCommon.GlobalFlags import globalflags
globalflags.DetDescrVersion = "ATLAS-R2-2015-03-01-00"
globalflags.DetGeo="atlas"
globalflags.InputFormat="pool"
globalflags.DataSource="geant4"
print globalflags

#--------------------------------------------------------------
# Set Detector setup
#--------------------------------------------------------------
# --- switch on InnerDetector
from AthenaCommon.DetFlags import DetFlags 
DetFlags.detdescr.SCT_setOn()
DetFlags.ID_setOn()
DetFlags.Calo_setOff()
DetFlags.Muon_setOff()
DetFlags.Truth_setOff()
DetFlags.LVL1_setOff()
DetFlags.SCT_setOn()
DetFlags.TRT_setOff()

# ---- switch parts of ID off/on as follows
#switch off tasks
DetFlags.pileup.all_setOff()
DetFlags.simulate.all_setOff()
DetFlags.makeRIO.all_setOff()
DetFlags.writeBS.all_setOff()
DetFlags.readRDOBS.all_setOff()
DetFlags.readRIOBS.all_setOff()
DetFlags.readRIOPool.all_setOff()
DetFlags.writeRIOPool.all_setOff()

import AtlasGeoModel.SetGeometryVersion
import AtlasGeoModel.GeoModelInit

from AthenaCommon.AlgSequence import AlgSequence
job = AlgSequence()

# Set up SCT cabling
from AthenaCommon.Include import include
include('InDetRecExample/InDetRecCabling.py')

#--------------------------------------------------------------
# Load IOVDbSvc
#--------------------------------------------------------------
IOVDbSvc = Service("IOVDbSvc")
from IOVDbSvc.CondDB import conddb
IOVDbSvc.GlobalTag='OFLCOND-RUN12-SDR-25'
IOVDbSvc.OutputLevel = DEBUG

test='MC'
#
#NOTE: Testing with run2 requires presence of local sqlite file 'configTest.db'
# available from http://sroe.web.cern.ch/sroe/configTest.db
#
if (test=='run1'):
  #ToolSvc = ServiceMgr.ToolSvc
  conddb.addFolder("","<db>sqlite://none;schema=/tmp/sroe/mycool.db;dbname=LOCAL</db> /SCT/DAQ/Configuration/ROD <tag>SctDaqConfigurationRod-MC-06</tag><forceTimestamp>1333597198</forceTimestamp>")
  conddb.addFolder("","<db>sqlite://none;schema=/tmp/sroe/mycool.db;dbname=LOCAL</db> /SCT/DAQ/Configuration/MUR <tag>SctDaqConfigurationMur-MC-06</tag><forceTimestamp>1333597198</forceTimestamp>")
  conddb.addFolder("","<db>sqlite://none;schema=/tmp/sroe/mycool.db;dbname=LOCAL</db> /SCT/DAQ/Configuration/RODMUR <tag>SctDaqConfigurationRodmur-MC-06</tag><forceTimestamp>1333597198</forceTimestamp>")
  conddb.addFolder("","<db>sqlite://none;schema=/tmp/sroe/mycool.db;dbname=LOCAL</db> /SCT/DAQ/Configuration/Geog <tag>SctDaqConfigurationGeog-MC-06</tag><forceTimestamp>1333597198</forceTimestamp>")
  conddb.addFolder("","<db>sqlite://none;schema=/tmp/sroe/mycool.db;dbname=LOCAL</db> /SCT/DAQ/Configuration/Chip <tag>SctDaqConfigurationChip-MC-06</tag><forceTimestamp>1333597198</forceTimestamp>")
  conddb.addFolder("","<db>sqlite://none;schema=/tmp/sroe/mycool.db;dbname=LOCAL</db> /SCT/DAQ/Configuration/Module <tag>SctDaqConfigurationModule-MC-06</tag><forceTimestamp>1333597198</forceTimestamp>")
  '''
  conddb.addFolder("","<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Configuration/Chip <tag>SctDaqConfigurationChip-MC-06</tag> <forceTimestamp>1333597198</forceTimestamp>")
  conddb.addFolder("","<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Configuration/Module <tag>SctDaqConfigurationModule-MC-06</tag> <forceTimestamp>1333597198</forceTimestamp>")
  conddb.addFolder("","<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Configuration/MUR <tag>SctDaqConfigurationMur-MC-06</tag> <forceTimestamp>1333597198</forceTimestamp>")
  conddb.addFolder("","<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Configuration/ROD <tag>SctDaqConfigurationRod-MC-06</tag> <forceTimestamp>1333597198</forceTimestamp>")
  conddb.addFolder("","<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Configuration/Geog <tag>SctDaqConfigurationGeog-MC-06</tag> <forceTimestamp>1333597198</forceTimestamp>")
  conddb.addFolder("","<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Configuration/RODMUR <tag>SctDaqConfigurationRodmur-MC-06</tag> <forceTimestamp>1333597198</forceTimestamp>")
  '''
else:
  '''
  conddb.addFolder("","<db>sqlite://none;schema=/tmp/sroe/mycool.db;dbname=OFLP200</db> /SCT/DAQ/Config/ROD <tag>SctDaqConfigRod-MC-06</tag><forceRunNumber>20</forceRunNumber>")
  conddb.addFolder("","<db>sqlite://none;schema=/tmp/sroe/mycool.db;dbname=OFLP200</db> /SCT/DAQ/Config/MUR <tag>SctDaqConfigMur-MC-06</tag><forceRunNumber>20</forceRunNumber>")
  conddb.addFolder("","<db>sqlite://none;schema=/tmp/sroe/mycool.db;dbname=OFLP200</db> /SCT/DAQ/Config/RODMUR <tag>SctDaqConfigRodmur-MC-06</tag><forceRunNumber>20</forceRunNumber>")
  conddb.addFolder("","<db>sqlite://none;schema=/tmp/sroe/mycool.db;dbname=OFLP200</db> /SCT/DAQ/Config/Geog <tag>SctDaqConfigGeog-MC-06</tag><forceRunNumber>20</forceRunNumber>")
  conddb.addFolder("","<db>sqlite://none;schema=/tmp/sroe/mycool.db;dbname=OFLP200</db> /SCT/DAQ/Config/Chip <tag>SctDaqConfigChip-MC-06</tag><forceRunNumber>20</forceRunNumber>")
  conddb.addFolder("","<db>sqlite://none;schema=/tmp/sroe/mycool.db;dbname=OFLP200</db> /SCT/DAQ/Config/Module <tag>SctDaqConfigModule-MC-06</tag><forceRunNumber>20</forceRunNumber>")
  
  conddb.addFolder("","<db>sqlite://none;schema=/tmp/sroe/mycool7.db;dbname=OFLP200</db> /SCT/DAQ/Config/ROD <tag>SctDaqConfigRod-MC-06</tag><forceRunNumber>200805</forceRunNumber>")
  conddb.addFolder("","<db>sqlite://none;schema=/tmp/sroe/mycool7.db;dbname=OFLP200</db> /SCT/DAQ/Config/MUR <tag>SctDaqConfigMur-MC-06</tag><forceRunNumber>200805</forceRunNumber>")
  conddb.addFolder("","<db>sqlite://none;schema=/tmp/sroe/mycool7.db;dbname=OFLP200</db> /SCT/DAQ/Config/RODMUR <tag>SctDaqConfigRodmur-MC-06</tag><forceRunNumber>200805</forceRunNumber>")
  conddb.addFolder("","<db>sqlite://none;schema=/tmp/sroe/mycool7.db;dbname=OFLP200</db> /SCT/DAQ/Config/Geog <tag>SctDaqConfigGeog-MC-06</tag><forceRunNumber>200805</forceRunNumber>")
  conddb.addFolder("","<db>sqlite://none;schema=/tmp/sroe/mycool7.db;dbname=OFLP200</db> /SCT/DAQ/Config/Chip <tag>SctDaqConfigChip-MC-06</tag><forceRunNumber>200805</forceRunNumber>")
  conddb.addFolder("","<db>sqlite://none;schema=/tmp/sroe/mycool7.db;dbname=OFLP200</db> /SCT/DAQ/Config/Module <tag>SctDaqConfigModule-MC-06</tag><forceRunNumber>200805</forceRunNumber>")
  
  conddb.addFolder("","<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Config/Chip <tag>SctDaqConfigChip-MC-06</tag><forceRunNumber>200805</forceRunNumber>")
  conddb.addFolder("","<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Config/Module <tag>SctDaqConfigModule-MC-06</tag><forceRunNumber>200805</forceRunNumber>")
  conddb.addFolder("","<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Config/MUR <tag>SctDaqConfigMur-MC-06</tag><forceRunNumber>200805</forceRunNumber>")
  conddb.addFolder("","<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Config/ROD <tag>SctDaqConfigRod-MC-06</tag><forceRunNumber>200805</forceRunNumber>")
  conddb.addFolder("","<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Config/Geog <tag>SctDaqConfigGeog-MC-06</tag><forceRunNumber>200805</forceRunNumber>")
  conddb.addFolder("","<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Config/RODMUR <tag>SctDaqConfigRodmur-MC-06</tag><forceRunNumber>200805</forceRunNumber>")
  
  #test 'perfect' local db
  conddb.addFolder("","<db>sqlite://none;schema=output.db;dbname=CONDBR2</db> /SCT/DAQ/Config/ROD <tag>SctDaqConfigRod-Oct2016_00</tag><forceRunNumber>20</forceRunNumber>")
  conddb.addFolder("","<db>sqlite://none;schema=output.db;dbname=CONDBR2</db> /SCT/DAQ/Config/MUR <tag>SctDaqConfigMur-Oct2016_00</tag><forceRunNumber>20</forceRunNumber>")
  conddb.addFolder("","<db>sqlite://none;schema=output.db;dbname=CONDBR2</db> /SCT/DAQ/Config/RODMUR <tag>SctDaqConfigRodmur-Oct2016_00</tag><forceRunNumber>20</forceRunNumber>")
  conddb.addFolder("","<db>sqlite://none;schema=output.db;dbname=CONDBR2</db> /SCT/DAQ/Config/Geog <tag>SctDaqConfigGeog-Oct2016_00</tag><forceRunNumber>20</forceRunNumber>")
  conddb.addFolder("","<db>sqlite://none;schema=output.db;dbname=CONDBR2</db> /SCT/DAQ/Config/Chip <tag>SctDaqConfigChip-Oct2016_00</tag><forceRunNumber>20</forceRunNumber>")
  conddb.addFolder("","<db>sqlite://none;schema=output.db;dbname=CONDBR2</db> /SCT/DAQ/Config/Module <tag>SctDaqConfigModule-Oct2016_00</tag><forceRunNumber>20</forceRunNumber>")
  '''

  #test perfect DB on server
  # conddb.addFolder("","<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Config/Chip <tag>SctDaqConfigChip-PERFECT-Oct2016_00</tag><forceRunNumber>200805</forceRunNumber>", className="CondAttrListVec")
  # conddb.addFolder("","<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Config/Module <tag>SctDaqConfigModule-PERFECT-Oct2016_00</tag><forceRunNumber>200805</forceRunNumber>", className="CondAttrListVec")
  # conddb.addFolder("","<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Config/MUR <tag>SctDaqConfigMur-PERFECT-Oct2016_00</tag><forceRunNumber>200805</forceRunNumber>", className="CondAttrListVec")
  if not conddb.folderRequested("/SCT/DAQ/Config/ROD"):
    conddb.addFolder("","<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Config/ROD <tag>SctDaqConfigRod-PERFECT-Oct2016_00</tag><forceRunNumber>200805</forceRunNumber>")
  if not conddb.folderRequested("/SCT/DAQ/Config/Geog"):
    conddb.addFolder("","<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Config/Geog <tag>SctDaqConfigGeog-PERFECT-Oct2016_00</tag><forceRunNumber>200805</forceRunNumber>")
  if not conddb.folderRequested("/SCT/DAQ/Config/RODMUR"):
    conddb.addFolder("","<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Config/RODMUR <tag>SctDaqConfigRodmur-PERFECT-Oct2016_00</tag><forceRunNumber>200805</forceRunNumber>")

from SCT_ConditionsTools.SCT_ConfigurationConditionsToolSetup import SCT_ConfigurationConditionsToolSetup
sct_ConfigurationConditionsToolSetup = SCT_ConfigurationConditionsToolSetup()
sct_ConfigurationConditionsToolSetup.setChannelFolderDb("<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Config/Chip <tag>SctDaqConfigChip-PERFECT-Oct2016_00</tag><forceRunNumber>200805</forceRunNumber>")
sct_ConfigurationConditionsToolSetup.setModuleFolderDb("<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Config/Module <tag>SctDaqConfigModule-PERFECT-Oct2016_00</tag><forceRunNumber>200805</forceRunNumber>")
sct_ConfigurationConditionsToolSetup.setMurFolderDb("<db>COOLOFL_SCT/OFLP200</db> /SCT/DAQ/Config/MUR <tag>SctDaqConfigMur-PERFECT-Oct2016_00</tag><forceRunNumber>200805</forceRunNumber>")
sct_ConfigurationConditionsToolSetup.setDbInstance("")
sct_ConfigurationConditionsToolSetup.setup()

from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConf import SCT_ConfigurationConditionsTestAlg
MyConfigTestAlg = SCT_ConfigurationConditionsTestAlg("MyConfigTestAlg")
MyConfigTestAlg.SCT_ConfigurationConditionsTool = sct_ConfigurationConditionsToolSetup.getTool()
job += MyConfigTestAlg

import AthenaCommon.AtlasUnixGeneratorJob
ServiceMgr.EventSelector.RunNumber =200805
ServiceMgr.EventSelector.InitialTimeStamp=1333597198
theApp.EvtMax = 1

ServiceMgr.MessageSvc.Format = "% F%40W%S%7W%R%T %0W%M"
ServiceMgr.MessageSvc.OutputLevel = DEBUG
