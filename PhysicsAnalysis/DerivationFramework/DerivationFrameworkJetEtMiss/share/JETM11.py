#====================================================================
# JETM11.py 
# reductionConf flag JETM11 in Reco_tf.py   
#====================================================================
from DerivationFrameworkCore.DerivationFrameworkMaster import *
from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkJetEtMiss.ExtendedJetCommon import *
from DerivationFrameworkJetEtMiss.METCommon import *
from DerivationFrameworkJetEtMiss import TriggerLists
from DerivationFrameworkEGamma.EGammaCommon import*
from DerivationFrameworkMuons.MuonsCommon import*
from DerivationFrameworkJetEtMiss.METTriggerDerivationContent import METTriggerDerivationContentManager

#======================================================================================================================
# SKIMMING TOOL
#======================================================================================================================
from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__TriggerSkimmingTool
# cutExpression = "(count(Electrons.DFCommonElectronsLHLoose && Electrons.DFCommonElectrons_pt > (20 * GeV) && abs(Electrons.DFCommonElectrons_eta) < 2.47) + count(Muons.DFCommonMuonsPreselection && Muons.pt > (20*GeV) && abs(Muon.eta) < 2.47) ) >= 1"
singleElTriggers = TriggerLists.single_el_Trig()
singleMuTriggers = TriggerLists.single_mu_Trig()
cutExpression = "(count(Electrons.DFCommonElectronsLHLoose && Electrons.pt > (24 * GeV) && abs(Electrons.eta) < 2.47) + count(Muons.DFCommonMuonsPreselection && Muons.pt > (24*GeV) && abs(Muons.eta) < 2.47) ) >= 1"
JETM11StringSkimmingTool = DerivationFramework__xAODStringSkimmingTool(
    name       = "JETM11StringSkimmingTool",
    expression = cutExpression)
JETM11TriggerSkimmingTool = DerivationFramework__TriggerSkimmingTool(
    name          = "JETM11TriggerSkimmingTool",
    TriggerListOR = singleElTriggers + singleMuTriggers)
ToolSvc += JETM11StringSkimmingTool
ToolSvc += JETM11TriggerSkimmingTool


#======================================================================================================================
# SET UP STREAM
#======================================================================================================================
streamName = derivationFlags.WriteDAOD_JETM11Stream.StreamName
fileName = buildFileName( derivationFlags.WriteDAOD_JETM11Stream )
JETM11Stream = MSMgr.NewPoolRootStream( streamName, fileName )
JETM11Stream.AcceptAlgs(['JETM11Kernel'])

contentManager = METTriggerDerivationContentManager("JETM11", JETM11Stream, trackThreshold=10)

# contentManager.thinningHelper.AppendToStream( JETM11Stream )
for tool in contentManager.thinningTools:
  ToolSvc += tool

#======================================================================================================================
# CREATE PRIVATE SEQUENCE
#======================================================================================================================
jetm11Seq = CfgMgr.AthSequencer("jetm11Seq")
DerivationFrameworkJob += jetm11Seq

#=======================================
# CREATE THE DERIVATION KERNEL ALGORITHM   
#=======================================
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
jetm11Seq += CfgMgr.DerivationFramework__DerivationKernel('JETM11Kernel',
                                                          SkimmingTools = [JETM11StringSkimmingTool, JETM11TriggerSkimmingTool],
                                                          ThinningTools = contentManager.thinningTools)

contentManager.slimmingHelper.AppendContentToStream(JETM11Stream)
