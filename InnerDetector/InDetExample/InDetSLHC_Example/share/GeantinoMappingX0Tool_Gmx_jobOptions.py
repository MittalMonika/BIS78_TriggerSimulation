#==============================================================
#
#
#		This job option runs the G4 simulation
#		of the ATLAS detector and the GeantinoMapping.
#		It can be run using athena.py
# 
# written by Jike Wang
# originated from Noemi Calace 
#==============================================================




from AthenaCommon.GlobalFlags import globalflags
globalflags.DetDescrVersion = 'ATLAS-P2-ITK-05-01-00'


#--- Algorithm sequence ---------------------------------------
from AthenaCommon.AlgSequence import AlgSequence
topSeq = AlgSequence()

#--- Output threshold (DEBUG, INFO, WARNING, ERROR, FATAL) ----
#from AthenaCommon.AppMgr import ServiceMgr
ServiceMgr.MessageSvc.OutputLevel  = INFO
ServiceMgr.MessageSvc.defaultLimit = 20000

#--- Detector flags -------------------------------------------
#from AthenaCommon.DetFlags import DetFlags
#from AthenaCommon.GlobalFlags import globalflags

import random
import time

### pass arguments with athena -c "..." ...jobOptions.py:
if 'myMomentum' not in dir() :
    myMomentum = 10000

if 'myRandomOffset' not in dir() :
    myRandomOffset = int(random.uniform(0,time.time())*0.001 )

if 'myRandomSeed1' not in dir() :
    myRandomSeed1 = int(random.uniform(0,time.time()))

if 'myRandomSeed2' not in dir() :
    myRandomSeed2 = int(random.uniform(0,time.time()))

if 'myMaxEvent' not in dir() :
    # myMaxEvent = 100000
   myMaxEvent = 1000

if 'myPt' not in dir() :
    myPt = 'pt'  # values are 'p' or 'pt'

if 'myGeo' not in dir() :
    myGeo = 'ATLAS-P2-ITK-05-01-00_VALIDATION'

print 'Random seeds and offset as calcluated by jobOptions ', myRandomSeed1, ' ', myRandomSeed2, ' offset - ', myRandomOffset




include("InDetSLHC_Example/preInclude.SLHC.py")
include("InDetSLHC_Example/preInclude.SiliconOnly.py")
include("InDetSLHC_Example/preInclude.SLHC_Setup_ExtBrl_32.py")
include("InDetSLHC_Example/preInclude.SLHC_Setup_Strip_GMX.py")



# the global flags
#globalflags.ConditionsTag = 'OFLCOND-MC15c-SDR-06'
#print globalflags.ConditionsTag


#--- AthenaCommon flags ---------------------------------------
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
athenaCommonFlags.PoolEvgenInput.set_Off()   ### is this necessary?
athenaCommonFlags.PoolHitsOutput = 'Hits.pool.root'
athenaCommonFlags.EvtMax = myMaxEvent

#--- Simulation flags -----------------------------------------
from G4AtlasApps.SimFlags import SimFlags
SimFlags.load_atlas_flags() # Going to use an ATLAS layout
SimFlags.SimLayout = myGeo
SimFlags.EventFilter.set_Off()

myMinEta = -6.0
myMaxEta = 6.0

myPDG    = 999 # 999 = Geantinos, 13 = Muons

include("GeneratorUtils/StdEvgenSetup.py")
theApp.EvtMax = myMaxEvent

import ParticleGun as PG
pg = PG.ParticleGun(randomSvcName=SimFlags.RandomSvc.get_Value(), randomStream="SINGLE")
pg.sampler.pid = myPDG
pg.sampler.mom = PG.EEtaMPhiSampler(energy=10000, eta=[myMinEta,myMaxEta])
topSeq += pg

SimFlags.RandomSeedOffset = myRandomOffset

### new rel17 (check Simulation/G4Atlas/G4AtlasApps/python/SimFlags.py for details)
SimFlags.RandomSeedList.addSeed( "SINGLE", myRandomSeed1, myRandomSeed2 )

from RngComps.RngCompsConf import AtRndmGenSvc 
myAtRndmGenSvc = AtRndmGenSvc()
myAtRndmGenSvc.Seeds = ["SINGLE "+str(myRandomSeed1)+" "+str(myRandomSeed2) ] 
myAtRndmGenSvc.OutputLevel 	= VERBOSE
myAtRndmGenSvc.EventReseeding   = False
ServiceMgr += myAtRndmGenSvc


if hasattr(SimFlags, 'UseV2UserActions') and SimFlags.UseV2UserActions():
    SimFlags.OptionalUserActionList.addAction('G4UA::LengthIntegratorTool',['BeginOfEvent','EndOfEvent','Step'])

else: 
## Add an action
    try:
       # Post UserAction Migration (ATLASSIM-1752)
       # NB the migrated MaterialStepRecorder does not have any special
       # properties, hence these are not set here.
       from G4AtlasServices.G4AtlasUserActionConfig import UAStore
       # by Jike 
       UAStore.addAction('MaterialStepRecorder',['BeginOfRun','EndOfRun','BeginOfEvent','EndOfEvent','Step'])
       UAStore.addAction('LengthIntegrator',['BeginOfEvent','EndOfEvent','Step'])
    except:
       # Pre UserAction Migration
       def geantino_action():
           from G4AtlasApps import AtlasG4Eng,PyG4Atlas
           GeantinoAction = PyG4Atlas.UserAction('TrkG4UserActions','MaterialStepRecorder', ['BeginOfRun','EndOfRun','BeginOfEvent','EndOfEvent','Step'])
           GeantinoAction.set_Properties({ "verboseLevel" : "1",
                                           "recordELoss"  : "1",
                                           "recordMSc"    : "1" })
           AtlasG4Eng.G4Eng.menu_UserActions.add_UserAction(GeantinoAction)

       SimFlags.InitFunctions.add_function('preInitG4', geantino_action)

############### The Material hit collection ##################

from AthenaPoolCnvSvc.WriteAthenaPool import AthenaPoolOutputStream
# --- check dictionary
ServiceMgr.AthenaSealSvc.CheckDictionary   = True
# --- commit interval (test)
ServiceMgr.AthenaPoolCnvSvc.OutputLevel = DEBUG
ServiceMgr.AthenaPoolCnvSvc.CommitInterval = 10
MaterialStream              = AthenaPoolOutputStream ( 'MaterialStream' )
# by Jike
MaterialStream.OutputFile   =   "MaterialStepCollection.root"
MaterialStream.ItemList    += [ 'Trk::MaterialStepCollection#*']

from GaudiSvc.GaudiSvcConf import THistSvc
ServiceMgr += THistSvc("THistSvc")
ServiceMgr.THistSvc.Output  = ["lengths DATAFILE='rad_intLength.histo.root' OPT='RECREATE'"]

##############################################################

## Populate alg sequence
from G4AtlasApps.PyG4Atlas import PyG4AtlasAlg
topSeq += PyG4AtlasAlg()

include("InDetSLHC_Example/postInclude.SLHC_Setup_ExtBrl_32.py")

#--- End jobOptions.GeantinoMapping.py file  ------------------------------

