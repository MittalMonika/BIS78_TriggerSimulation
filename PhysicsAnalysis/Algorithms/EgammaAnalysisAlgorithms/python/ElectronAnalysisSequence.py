# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration

# Framework import(s):
import ROOT

# AnaAlgorithm import(s):
from AnaAlgorithm.AnaAlgSequence import AnaAlgSequence
from AnaAlgorithm.DualUseConfig import createAlgorithm, addPrivateTool

def makeElectronAnalysisSequence( dataType,
                                  isolationWP = 'GradientLoose',
                                  likelihoodWP = 'LooseLHElectron',
                                  recomputeLikelihood = False,
                                  chargeIDSelection = False ):
    """Create an electron analysis algorithm sequence

    Keyword arguments:
      dataType -- The data type to run on ("data", "mc" or "afii")
      isolationWP -- The isolation selection working point to use
      likelihoodWP -- The likelihood selection working point to use
      recomputeLikelihood -- Whether to rerun the LH. If not, use derivation flags
      chargeIDSelection -- Whether or not to perform charge ID/flip selection
    """

    # Make sure we received a valid data type.
    if not dataType in [ 'data', 'mc', 'afii' ]:
        raise ValueError( 'Invalid data type: %' % dataType )

    # Create the analysis algorithm sequence object:
    seq = AnaAlgSequence( "ElectronAnalysisSequence" )

    # Variables keeping track of the selections being applied.
    _selectionDecorNames = []
    _selectionDecorCount = []

    # Set up the an eta-cut on all electrons prior to everything else
    alg = createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronEtaCutAlg' )
    alg.selectionDecoration = 'selectEta'
    addPrivateTool( alg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
    alg.selectionTool.maxEta = 2.47
    alg.selectionTool.useClusterEta = True
    seq.append( alg, inputPropName = 'particles',
                outputPropName = 'particlesOut' )
    _selectionDecorNames.append( alg.selectionDecoration )
    _selectionDecorCount.append( 2 )

    # Set up the likelihood ID selection algorithm
    # It is safe to do this before calibration, as the cluster E is used
    alg = createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronLikelihoodAlg' )
    alg.selectionDecoration = 'selectLikelihood'
    _selectionDecorNames.append( alg.selectionDecoration )
    if recomputeLikelihood:
        # Rerun the likelihood ID
        addPrivateTool( alg, 'selectionTool', 'AsgElectronLikelihoodTool' )
        alg.selectionTool.primaryVertexContainer = 'PrimaryVertices'
        alg.selectionTool.WorkingPoint = likelihoodWP
        _selectionDecorCount.append( 7 )
    else:
        # Select from Derivation Framework flags
        addPrivateTool( alg, 'selectionTool', 'CP::AsgFlagSelectionTool' )
        dfFlag = "DFCommonElectronsLH" + likelihoodWP.split('LH')[0]
        alg.selectionTool.selectionFlags = [dfFlag]
        _selectionDecorCount.append( 1 )
    seq.append( alg, inputPropName = 'particles',
                outputPropName = 'particlesOut' )

    # Select electrons only with good object quality.
    alg = createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronObjectQualityAlg' )
    alg.selectionDecoration = 'goodOQ'
    addPrivateTool( alg, 'selectionTool', 'CP::EgammaIsGoodOQSelectionTool' )
    alg.selectionTool.Mask = ROOT.xAOD.EgammaParameters.BADCLUSELECTRON
    seq.append( alg, inputPropName = 'particles',
                outputPropName = 'particlesOut' )
    _selectionDecorNames.append( alg.selectionDecoration )
    _selectionDecorCount.append( 1 )

    # Only run subsequent processing on the objects passing all of these cuts.
    # Since these are independent of the electron calibration, and this speeds
    # up the job.
    alg = createAlgorithm( 'CP::AsgViewFromSelectionAlg',
                           'ElectronPreSelViewFromSelectionAlg' )
    alg.selection = _selectionDecorNames
    seq.append( alg, inputPropName = 'input', outputPropName = 'output' )

    # Set up the calibration and smearing algorithm:
    alg = createAlgorithm( 'CP::EgammaCalibrationAndSmearingAlg',
                           'ElectronCalibrationAndSmearingAlg' )
    addPrivateTool( alg, 'calibrationAndSmearingTool',
                    'CP::EgammaCalibrationAndSmearingTool' )
    alg.calibrationAndSmearingTool.ESModel = 'es2017_R21_PRE'
    alg.calibrationAndSmearingTool.decorrelationModel = '1NP_v1'
    if dataType == 'afii':
        alg.calibrationAndSmearingTool.useAFII = 1
    else:
        alg.calibrationAndSmearingTool.useAFII = 0
        pass
    seq.append( alg, inputPropName = 'egammas', outputPropName = 'egammasOut',
                affectingSystematics = '(^EG_RESOLUTION_.*)|(^EG_SCALE_.*)' )

    # Set up the isolation correction algorithm:
    alg = createAlgorithm( 'CP::EgammaIsolationCorrectionAlg',
                           'ElectronIsolationCorrectionAlg' )
    addPrivateTool( alg, 'isolationCorrectionTool',
                    'CP::IsolationCorrectionTool' )
    if dataType == 'data':
        alg.isolationCorrectionTool.IsMC = 0
    else:
        alg.isolationCorrectionTool.IsMC = 1
        pass
    seq.append( alg, inputPropName = 'egammas', outputPropName = 'egammasOut' )

    # Set up the isolation selection algorithm:
    alg = createAlgorithm( 'CP::EgammaIsolationSelectionAlg',
                           'ElectronIsolationSelectionAlg' )
    addPrivateTool( alg, 'selectionTool', 'CP::IsolationSelectionTool' )
    alg.selectionTool.ElectronWP = isolationWP
    seq.append( alg, inputPropName = 'egammas', outputPropName = 'egammasOut' )

    # Set up the track selection algorithm:
    alg = createAlgorithm( 'CP::AsgLeptonTrackSelectionAlg',
                           'ElectronTrackSelectionAlg' )
    alg.selectionDecoration = 'trackSelection'
    alg.maxD0Significance = 5
    alg.maxDeltaZ0SinTheta = 0.5
    seq.append( alg, inputPropName = 'particles',
                outputPropName = 'particlesOut' )
    _selectionDecorNames.append( 'trackSelection' )
    _selectionDecorCount.append( 3 )

    # Select electrons only if they don't appear to have flipped their charge.
    if chargeIDSelection:
        alg = createAlgorithm( 'CP::AsgSelectionAlg',
                               'ElectronChargeIDSelectionAlg' )
        alg.selectionDecoration = 'chargeID'
        addPrivateTool( alg, 'selectionTool',
                        'AsgElectronChargeIDSelectorTool' )
        alg.selectionTool.TrainingFile = \
          'ElectronPhotonSelectorTools/ChargeID/ECIDS_20180731rel21Summer2018.root'
        alg.selectionTool.WorkingPoint = 'Loose'
        alg.selectionTool.CutOnBDT = -0.337671 # Loose 97%
        seq.append( alg, inputPropName = 'particles',
                    outputPropName = 'particlesOut' )
        _selectionDecorNames.append( alg.selectionDecoration )
        _selectionDecorCount.append( 1 )
        pass

    # Set up the electron efficiency correction algorithm:
    alg = createAlgorithm( 'CP::ElectronEfficiencyCorrectionAlg',
                           'ElectronEfficiencyCorrectionAlg' )
    addPrivateTool( alg, 'efficiencyCorrectionTool',
                    'AsgElectronEfficiencyCorrectionTool' )
    alg.efficiencyCorrectionTool.MapFilePath = \
        'ElectronEfficiencyCorrection/2015_2017/rel21.2/Summer2017_Prerec_v1/map0.txt'
    alg.efficiencyCorrectionTool.RecoKey = "Reconstruction"
    alg.efficiencyCorrectionTool.CorrelationModel = "TOTAL"
    alg.efficiencyCorrectionTool.CorrelationModel = "TOTAL"
    alg.efficiencyDecoration = 'effCor'
    if dataType == 'afii':
        alg.efficiencyCorrectionTool.ForceDataType = \
          ROOT.PATCore.ParticleDataType.Fast
    elif dataType == 'mc':
        alg.efficiencyCorrectionTool.ForceDataType = \
          ROOT.PATCore.ParticleDataType.Full
        pass
    alg.outOfValidity = 2 #silent
    alg.outOfValidityDeco = 'bad_eff'
    if dataType != 'data':
        seq.append( alg, inputPropName = 'electrons',
                    outputPropName = 'electronsOut',
                    affectingSystematics = '(^EL_EFF_.*)' )
        _selectionDecorNames.append( 'bad_eff' )
        _selectionDecorCount.append( 1 )
        pass

    # Set up an algorithm used for debugging the electron selection:
    alg = createAlgorithm( 'CP::ObjectCutFlowHistAlg',
                           'ElectronCutFlowDumperAlg' )
    alg.histPattern = 'electron_cflow_%SYS%'
    alg.selection = _selectionDecorNames
    alg.selectionNCuts = _selectionDecorCount
    seq.append( alg, inputPropName = 'input' )

    # Set up an algorithm that makes a view container using the selections
    # performed previously:
    alg = createAlgorithm( 'CP::AsgViewFromSelectionAlg',
                           'ElectronViewFromSelectionAlg' )
    alg.selection = _selectionDecorNames
    seq.append( alg, inputPropName = 'input', outputPropName = 'output' )

    # Set up an algorithm dumping the properties of the electrons, for
    # debugging:
    alg = createAlgorithm( 'CP::KinematicHistAlg',
                           'ElectronKinematicDumperAlg' )
    alg.histPattern = 'electron_%VAR%_%SYS%'
    seq.append( alg, inputPropName = 'input' )

    # Return the sequence:
    return seq
