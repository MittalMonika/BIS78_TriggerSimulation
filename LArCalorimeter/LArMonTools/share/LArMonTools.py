# Define ONE LArBadChannelMask for all Monitoring tools
from LArBadChannelTool.LArBadChannelToolConf import LArBadChannelMasker
theLArBadChannelsMasker=LArBadChannelMasker("LArBadChannelsMasker")
theLArBadChannelsMasker.DoMasking=True
theLArBadChannelsMasker.ProblemsToMask=[
    "deadReadout","deadPhys","short","almostDead",
    "highNoiseHG","highNoiseMG","highNoiseLG","sporadicBurstNoise"
    ]
ToolSvc+=theLArBadChannelsMasker

include("LArMonTools/LArFEBMon_jobOptions.py") # Data integrity checks - Includes LArFEBMon tool and LArEventInfo
if Type == 'Pedestal':
    #include("LArMonTools/LArDigitNoiseMonTool_jobOptions.py")       # Monitoring of digits (noise, pedestal, correlation...)
    #include("LArMonTools/LArOddCellsMonTool_jobOptions.py")         # Monitoring of digits compared with DB pedestal / noise
#    include("LArMonTools/LArRawChannelNoiseMonTool_jobOptions.py") # Monitoring of raw channels (noise, pedestal, correlation...)
#    include("LArMonTools/LArScaNoiseMonTool_jobOptions.py")        # Monitoring of fixed pattern noise
#    include("LArMonTools/LArFebNoiseMonTool_jobOptions.py")        # Monitoring of noise shape (using FEB as a scope)
#    include("LArMonTools/LArFebNoiseMonToolAlt_jobOptions.py")     # Monitoring of noise shape (using FEB as a scope)
    coherent_noise_calibration_run=True
    include("LArMonTools/LArNoiseCorrelationMon_jobOptions.py")    # Coherent noise plots


elif Type == 'Delay':
    include("LArMonTools/LArDelayMon_jobOptions.py") # includes LArWaveMon tool and LArJitterMon
#    include("LArMonTools/LArXtalk_jobOptions.py")    # includes LArWaveXtalk tool
elif Type == 'Ramp':
    include("LArMonTools/LArRampMon_jobOptions.py")
elif Type == 'Cosmic':
    include("LArMonTools/LArDigitMon_jobOptions.py")
else:
    print "Unknown Type !!!"
