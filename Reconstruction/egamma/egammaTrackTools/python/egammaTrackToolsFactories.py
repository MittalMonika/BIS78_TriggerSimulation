# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration

__doc__ = "ToolFactories to instantiate all egammaCaloTools with default configuration"
__author__ = "Bruno Lenzi , Christos Anastopoulos"
from egammaRec.Factories import FcnWrapper, ToolFactory
import egammaTrackToolsConf
from egammaRec import egammaKeys
from egammaTools.InDetTools import egammaExtrapolator
import AthenaCommon.CfgMgr as CfgMgr

EMLastCaloExtensionTool =  ToolFactory (CfgMgr.Trk__ParticleCaloExtensionTool,
                                           name="EMLastCaloExtensionTool",
                                           Extrapolator = egammaExtrapolator)

EMParticleCaloExtensionTool =  ToolFactory (CfgMgr.Trk__ParticleCaloExtensionTool, 
                                            name="EMParticleCaloExtensionTool",
                                            Extrapolator = egammaExtrapolator,
                                            ParticleType="muon",
                                            StartFromPerigee = True)

EMExtrapolationTools = ToolFactory( egammaTrackToolsConf.EMExtrapolationTools,
                                    name ="EMExtrapolationTools",
                                    LastCaloExtentionTool=EMLastCaloExtensionTool,
                                    PerigeeCaloExtentionTool = EMParticleCaloExtensionTool,
                                    Extrapolator=egammaExtrapolator,
                                    useCaching=False)

EMExtrapolationToolsCache = ToolFactory( egammaTrackToolsConf.EMExtrapolationTools,
                                         name ="EMExtrapolationToolsCache",
                                         LastCaloExtentionTool=EMLastCaloExtensionTool,
                                         PerigeeCaloExtentionTool = EMParticleCaloExtensionTool,
                                         Extrapolator=egammaExtrapolator,
                                         GSFPerigeeCache='GSFPerigeeCaloExtension',
                                         GSFLastCache='GSFLastCaloExtension',
                                         useCaching=True)

