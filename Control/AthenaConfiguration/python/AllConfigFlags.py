# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaCommon.SystemOfUnits import TeV
from AthenaConfiguration.AutoConfigFlags import GetFileMD


def _createCfgFlags():

    acf=AthConfigFlags()
#Global Flags:
#They are becoming, please use the specific flags below.
    # acf.addFlag('global.InputFiles',["_ATHENA_GENERIC_INPUTFILE_NAME_",])
    # acf.addFlag('global.isOnline',False)
    # acf.addFlag('global.GeoLayout',"atlas")
    # acf.addFlag('global.ProjectName',lambda prevFlags : GetFileMD(prevFlags.get("global.InputFiles")).get("Project","data17_13TeV"))
    # acf.addFlag('global.isMC',lambda prevFlags : GetFileMD(prevFlags.get("global.InputFiles")).get("isMC",None))
    # acf.addFlag('global.BunchSpacing',25) # Bunch spacing in ns
    # acf.addFlag("global.NumberOfCollisions",0) #"Number of collisions per beam crossing. Should be 2.3*(L/10**33)*(bunchSpacing/25 ns)
    # acf.addFlag("global.BeamType",'collisions') # "Specify data taking type: 'collisions' (default), 'singlebeam','cosmics'")
    # acf.addFlag("global.BeamEnergy",lambda prevFlags : GetFileMD(prevFlags.get("global.InputFiles")).get("BeamEnergy",7*TeV))
    # acf.addFlag("global.estimatedLuminosity",lambda prevFlags : 1E33*(prevFlags.get("global.NumberOfCollisions")/2.3)* \
    #                 (25./prevFlags.get("global.BunchSpacing")))
    # acf.addFlag("global.RunNumber",lambda prevFlags : list(GetFileMD(prevFlags.get("global.InputFiles")).get("RunNumber",None)))

# Replacement for global flags 
# people please comment here
    acf.addFlag('Input.Files', ["_ATHENA_GENERIC_INPUTFILE_NAME_",] )
    acf.addFlag('Input.isMC', lambda prevFlags : GetFileMD(prevFlags.Input.Files).get("isMC",None))
    acf.addFlag('Input.RunNumber', lambda prevFlags : list(GetFileMD(prevFlags.Input.Files).get("RunNumber",None)))
    acf.addFlag('Input.ProjectName', lambda prevFlags : GetFileMD(prevFlags.Input.Files).get("Project","data17_13TeV") )
    

    acf.addFlag('Common.isOnline', False ) #  Job runs in an online environment (access only to resources available at P1)

    # replace global.Beam*
    acf.addFlag('Beam.BunchSpacing', 25)
    acf.addFlag("Beam.NumberOfCollisions",0)
    acf.addFlag('Beam.Type', 'collisions')
    acf.addFlag('Beam.Energy', lambda prevFlags : GetFileMD(prevFlags.Input.Files).get('BeamEnergy',7*TeV))
    acf.addFlag('Beam.estimatedLuminosity', lambda prevFlags : ( 1E33*(prevFlags.Beam.NumberOfCollisions)/2.3 ) *\
        (25./prevFlags.Beam.BunchSpacing))
    
        
#Geo Model Flags:
    acf.addFlag('GeoModel.Layout', 'atlas') # replaces global.GeoLayout
    acf.addFlag("GeoModel.AtlasVersion", lambda prevFlags : GetFileMD(prevFlags.Input.Files).get("Geometry","ATLAS-R2-2016-01-00-01"))


#IOVDbSvc Flags:
    acf.addFlag("IOVDb.GlobalTag",lambda prevFlags : GetFileMD(prevFlags.Input.Files).get("ConditionsTag","CONDBR2-BLKPA-2017-05"))
    from IOVDbSvc.IOVDbAutoCfgFlags import getDatabaseInstanceDefault
    acf.addFlag("IOVDb.DatabaseInstance",getDatabaseInstanceDefault)


#LAr Flags:
    from LArCellRec.LArConfigFlags import createLArConfigFlags
    lcf=createLArConfigFlags()
    acf.join(lcf)

#CaloNoise Flags
    acf.addFlag("Calo.Noise.fixedLumiForNoise",-1)
    acf.addFlag("Calo.Noise.useCaloNoiseLumi",True)
                

#TopoCluster Flags:
    acf.addFlag("Calo.TopoCluster.doTwoGaussianNoise",True)
    acf.addFlag("Calo.TopoCluster.doTreatEnergyCutAsAbsolute",False)


# Trigger
    from TriggerJobOpts.TriggerConfigFlags import createTriggerFlags
    acf.join( createTriggerFlags() )

    return acf


ConfigFlags=_createCfgFlags()

del _createCfgFlags

if __name__=="__main__":
    import sys
    if len(sys.argv)>1:
        ConfigFlags.Input.Files = sys.argv[1:]
    else:
        ConfigFlags.Input.Files = [ "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CommonInputs/data16_13TeV.00311321.physics_Main.recon.AOD.r9264/AOD.11038520._000001.pool.root.1",]
    
    ConfigFlags.initAll()
    ConfigFlags.dump()
    
