# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

## this is test for clusterization configuration

#import TrackReco 	   as TrackReco
#import PixelGeoModelConfig      as PixelGeoModelConfig
#import SCT_GeoModelConfig	as SCT_GeoModelConfig

#arg_TrackingCuts             = 'TrackingCuts'
#arg_TrackCollectionKeys      = 'tracks'
#arg_TrackCollectionTruthKeys = 'truth'

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

# the requires a single additional layer where the algorithm returned by getInDetPixelClusterization is collected:
def main(flags) :
    from AthenaCommon.Configurable import Configurable
    Configurable.configurableRun3Behavior = 1
    from ClusterizationConfig import InDetClusterizationAlgorithmsCfg
    result = InDetClusterizationAlgorithmsCfg(flags)
    ### configuration of pixel and sct clusterization algorithms
    return result


if __name__ == "__main__":
    import IDTestFlags
    flags = IDTestFlags.IDTestFlags_q221()
    ##from AthenaConfiguration.AllConfigFlags import ConfigFlags
    ##flags = ConfigFlags
    flags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-16"
    flags.Input.isMC = True
    flags.Detector.SimulatePixel = False
    flags.Detector.SimulateSCT   = False
    flags.Detector.SimulateTRT   = False
    flags.GeoModel.Align.Dynamic    = False
    #######flags.InDetFlags.doPixelClusterSplitting = True

    flags.Input.Files=["myRDO.pool.root"]
    flags.lock()

    acc = main(flags)
    ##acc.setAppProperty("EvtMax",25)
    ##acc.store(open("test_SiClusterization.pkl", "w"))
    acc.run(25)
    #with open('test4.pkl', mode="wb") as f:
    #   dill.dump(acc, f)
    acc.store(open("test00.pkl", "w"))
