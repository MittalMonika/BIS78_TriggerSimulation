#
#  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
#



def RegSelConfig( flags ):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from RegionSelector.RegionSelectorConf import RegSelSvc
    from AthenaCommon.SystemOfUnits import mm
    acc = ComponentAccumulator()

    regSel = RegSelSvc()
    regSel.DeltaZ = 225 * mm

    # there will be ifology here enabling only the configured detectors
    from LArRegionSelector.LArRegionSelectorConf import LArRegionSelectorTable
    larTable =  LArRegionSelectorTable(name="LArRegionSelectorTable")
    acc.addPublicTool( larTable )
    regSel.LArRegionSelectorTable      = larTable


    from TileRawUtils.TileRawUtilsConf import TileRegionSelectorTable
    tileTable =  TileRegionSelectorTable(name="TileRegionSelectorTable")
    acc.addPublicTool( tileTable )
    # ??? that is puzzle, the RegSelSvc seems not to ahev such a property
    # while it is set in:RegSelSvcDefault.py
    #regSel.TileRegionSelectorTable     = tileTable

    regSel.enableCalo = True
    regSel.enableID = True
    regSel.enablePixel = True
    regSel.enableSCT = True
    regSel.enableTRT = True

    # Setup RegSelSvc for muon detector based on RegSelSvcDefault.py
    from MuonRegionSelector.MuonRegionSelectorConf import RPC_RegionSelectorTable
    rpcTable = RPC_RegionSelectorTable(name = "RPC_RegionSelectorTable")
    acc.addPublicTool( rpcTable )

    from MuonRegionSelector.MuonRegionSelectorConf import MDT_RegionSelectorTable
    mdtTable = MDT_RegionSelectorTable(name = "MDT_RegionSelectorTable")
    acc.addPublicTool( mdtTable )

    from MuonRegionSelector.MuonRegionSelectorConf import TGC_RegionSelectorTable
    tgcTable = TGC_RegionSelectorTable(name = "TGC_RegionSelectorTable")
    acc.addPublicTool( tgcTable )

    from MuonRegionSelector.MuonRegionSelectorConf import CSC_RegionSelectorTable
    cscTable = CSC_RegionSelectorTable(name = "CSC_RegionSelectorTable")
    acc.addPublicTool( cscTable )

    # ??? that is same puzzle of Calo, the RegSelSvc seems not to ahev such a property
    # while it is set in:RegSelSvcDefault.py 
    #regSel.RPC_RegionLUT_CreatorTool   = rpcTable
    #regSel.MDT_RegionLUT_CreatorTool   = mdtTable
    #regSel.TGC_RegionLUT_CreatorTool   = tgcTable
    #regSel.CSC_RegionLUT_CreatorTool   = cscTable

    regSel.enableMuon = True
    regSel.enableRPC  = True
    regSel.enableMDT  = True
    regSel.enableTGC  = True
    regSel.enableCSC  = True
    regSel.enableMM   = False   

    from InDetRegionSelector.InDetRegionSelectorConf import SiRegionSelectorTable
    pixTable = SiRegionSelectorTable(name        = "PixelRegionSelectorTable",
                                     ManagerName = "Pixel",
                                     OutputFile  = "RoITablePixel.txt",
                                     PrintHashId = True,
                                     PrintTable  = False)
    acc.addPublicTool(pixTable)

    from InDetRegionSelector.InDetRegionSelectorConf import SiRegionSelectorTable
    sctTable = SiRegionSelectorTable(name        = "SCT_RegionSelectorTable",
                                     ManagerName = "SCT",
                                     OutputFile  = "RoITableSCT.txt",
                                     PrintHashId = True,
                                     PrintTable  = False)
    acc.addPublicTool(sctTable)

    from InDetRegionSelector.InDetRegionSelectorConf import TRT_RegionSelectorTable
    trtTable = TRT_RegionSelectorTable(name = "TRT_RegionSelectorTable",
                                       ManagerName = "TRT",
                                       OutputFile  = "RoITableTRT.txt",
                                       PrintHashId = True,
                                       PrintTable  = False)
    acc.addPublicTool(trtTable)

    return acc, regSel

if __name__ == "__main__":
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    ConfigFlags.Input.Files = defaultTestFiles.RAW

    acc,regSel = RegSelConfig( ConfigFlags )
    acc.store( file( "test.pkl", "w" ) )
    print "All OK"
