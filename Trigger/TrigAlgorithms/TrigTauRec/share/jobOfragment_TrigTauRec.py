# jobOptions file for tau Reconstruction

theApp.Dlls += [ "tauRec", "TrigTauRec" ]
theApp.Dlls += [ "CaloUtils"]


#--------------------------------------------------------------
# Algorithms Private Options
#--------------------------------------------------------------
#tower Maker sub-algorithms:
TrigTauRec_h5_EF = Algorithm( "TrigTauRec_h5_EF" )

TrigTauRec_h5_EF.toolNames = [
    "tauSeedBuilder/TrigTauSeeds",
    "tauTrack/TrigTauTracks",
    "tauCellBuilder/TrigTauCells",		
    "tauCalibrateWeightTool/TrigTauCalibrate",
    ]


				
# For data with noise, threshold on cells
TrigTauRec_h5_EF.TrigTauSeeds.seedContainerName      = "RoIEMCalo"
TrigTauRec_h5_EF.TrigTauSeeds.UseCaloNoiseTool       = FALSE
TrigTauRec_h5_EF.TrigTauSeeds.UsePileUpNoise         = FALSE
TrigTauRec_h5_EF.TrigTauSeeds.useCaloCellList        = 1

# which tracks to use
TrigTauRec_h5_EF.TrigTauTracks.TrackContainer        = "TrigTauTrackCandidate"
TrigTauRec_h5_EF.TrigTauTracks.VxPrimaryCandidate    = "TrigTauVtxPrimaryCandidate"
TrigTauRec_h5_EF.TrigTauTracks.TrackPTmin            = 2000.0
TrigTauRec_h5_EF.TrigTauTracks.TrackDist             = 0.3

# parameters required by tauCellBuilder tool:
TrigTauRec_h5_EF.TrigTauCells.CellEthreshold         = 200.0
TrigTauRec_h5_EF.TrigTauCells.StripEthreshold        = 200.0
TrigTauRec_h5_EF.TrigTauCells.EMSumThreshold         = 500.0
TrigTauRec_h5_EF.TrigTauCells.EMSumRadius            = 0.2

#
# select G4-jet weights
TrigTauRec_h5_EF.TrigTauCalibrate.CellWeightTool="H1WeightToolCSC12"
TrigTauRec_h5_EF.TrigTauCalibrate.pTNumberOfBins = 3
TrigTauRec_h5_EF.TrigTauCalibrate.etaNumberOfBins = 3
TrigTauRec_h5_EF.TrigTauCalibrate.pTPoints = [ 15000, 35000, 150000 ]
TrigTauRec_h5_EF.TrigTauCalibrate.etaPoints = [  0.25, 1.0, 2.0 ]
TrigTauRec_h5_EF.TrigTauCalibrate.pTetaCorrectionsNtr1= [
    ## made with histo pT(tauMC)/pT(tauRec) vs pT vs eta, llh>2, ntr==1
    ##eta=0.0..0.5  0.5..1.5  1.5..2.5
    0.8339,   0.8125,   0.8254,   ## pt=-5 .. 30
    0.8747,   0.8583,   0.8594,   ## pt=30 .. 40
    0.9088,   0.9013,   0.8922    ## pt=40 .. 1000
    ]
TrigTauRec_h5_EF.TrigTauCalibrate.pTetaCorrectionsNtr23= [
    ## made with histo pT(tauMC)/pT(tauRec) vs pT vs eta, llh>2, ntr==2||3
    ##eta=0.0..0.5  0.5..1.5  1.5..2.5
    0.9000,   0.8593,   0.9034,   ## pt=-5 .. 30
    0.9208,   0.8791,   0.9132,   ## pt=30 .. 40
    0.9359,   0.9231,   0.9033    ## pt=40 .. 1000
    ]
TrigTauRec_h5_EF.TrigTauCalibrate.FudgeFactor        = 1.011

# Cut values in TrigTauCuts to select tau's
#TrigTauRec_h5_EF.TrigTauCuts.NumTrackMin             = 1        # Min. number tracks
#TrigTauRec_h5_EF.TrigTauCuts.NumTrackMax             = 3        # Max. number tracks
#TrigTauRec_h5_EF.TrigTauCuts.EMRadiusMax             = 0.15     # Max. EM radius
#TrigTauRec_h5_EF.TrigTauCuts.IsolationFracMax        = 0.3      # Max. isolation fraction
#TrigTauRec_h5_EF.TrigTauCuts.EMFractionMin           = 0.6      # Min. fraction of Et in the EM layers
#TrigTauRec_h5_EF.TrigTauCuts.PtMaxTrackMin           = 10000.0  # Min. Pt of the hardest track


TrigTauRec_h5_EF.TrigTauTracks.tauRecTTCExtrapolator.Extrapolator='Trk::Extrapolator/InDetExtrapolator'
TrigTauRec_h5_EF.TrigTauTracks.tauRecTTCExtrapolator.CaloDepthTool.DepthChoice = "entrance"

from TrkVertexBilloirTools.TrkVertexBilloirToolsConf import Trk__FastVertexFitter
TauRecFitterTool = Trk__FastVertexFitter(
    Extrapolator = "Trk::Extrapolator/InDetExtrapolator"
    )

ToolSvc += TauRecFitterTool
print TauRecFitterTool




