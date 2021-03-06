# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

""" Flags to control the L1 calo upgrade simulation  """

from AthenaCommon.JobProperties import JobProperty, JobPropertyContainer

# get logger for this file
from AthenaCommon.Logging import logging
log = logging.getLogger( 'TrigT1CaloFexSim.L1SimulationControlFlags' )
log.setLevel(logging.DEBUG)



_caloflags = list()
_ctpflags = list()
_topoflags = list()
_glflags = list()


class SCellType(JobProperty):
    """ String which contains the chosen approach to supercell creation: Pulse, Emulated, BCID
    Pulse:    Fully simulated supercells, from supercell pulse
    BCID:     Fully simulated supercells with applied BCID corrections (this is the only kind of supercell where we apply BCID corrections)
    Emulated: Supercells reconstructed from the ET sum of the constituent calo cells
              (this disables ApplySCQual)
    """
    statusOn = True
    allowedType = ['str']
    StoredValue = "BCID"
    #We may need this again at some point, so I leave it -- but we are now at the point where we need quality cuts during emulation, B Carlson March 27, 2020
    '''
    def _do_action(self):
        if self.get_Value()=="Emulated":
            from TrigT1CaloFexSim.L1SimulationControlFlags import L1Phase1SimFlags as simflags
            simflags.Calo.ApplySCQual=False 
    '''
_caloflags.append(SCellType)


class QualBitMask(JobProperty):
    """ int bitmask to be used for quality requirements
    0x40:  Corresponds to the peak finder (BCID maximum) approach, implemented per supercell
    0x200: Corresponds to the ET*time requirement, implemented per supercell
    """
    statusOn = True
    allowedType = ['int']
    StoredValue = 0x200
_caloflags.append(QualBitMask)


class ComputeEFexClusters(JobProperty):
    """ ComputeClusters """
    statusOn = True
    allowedType = ['bool']
    StoredValue = False
_caloflags.append(ComputeEFexClusters)


class ApplySCQual(JobProperty):
    """ ? """
    statusOn = True
    allowedType = ['bool']
    StoredValue = True
_caloflags.append(ApplySCQual)


class UseAllCalo(JobProperty):
    """ Needs to be enabled when no Trigger Towers available """
    statusOn = True
    allowedType = ['bool']
    StoredValue = False
_caloflags.append(UseAllCalo)


class RunFexAlgorithms(JobProperty):
    """ ComputeClusters """
    statusOn = True
    allowedType = ['bool']
    StoredValue = True
_caloflags.append(RunFexAlgorithms)


class StoreSuperCellsInAODFULL(JobProperty):
    """ SuperCells collections are too large to be saved in the AODFULL by default """
    statusOn = True
    allowedType = ['bool']
    StoredValue = False
_caloflags.append(StoreSuperCellsInAODFULL)


class RunTopoAlgorithms(JobProperty):
    """ Run L1Topo Simulation """
    statusOn = False
    allowedType = ['bool']
    StoredValue = False
_topoflags.append(RunTopoAlgorithms)


class RunCTPEmulation(JobProperty):
    """ Run the CTP Emulation """
    statusOn = True
    allowedType = ['bool']
    StoredValue = True
_ctpflags.append(RunCTPEmulation)


# global

class OutputHistFile(JobProperty):
    """ Location of output root file
    If this is changed to a different filename, 
    then something else than EXPERT should be used, 
    and some L1 algorithms need to change the hist stream
    """
    statusOn = True
    allowedType = ['str']
    StoredValue = "EXPERT#expert-monitoring.root"
_glflags.append(OutputHistFile)

class EnableDebugOutput(JobProperty):
    """ To enable DEBUG or VERBOSE output for specific algorithms """
    statusOn = True
    allowedType = ['bool']
    StoredValue = False
_glflags.append(EnableDebugOutput)


# end of flag definitions


class Calo(JobPropertyContainer):
    """ L1Calo Phase I simulation flags """
    pass

class CTP(JobPropertyContainer):
    """ CTP Phase I simulation flags """
    pass

class Topo(JobPropertyContainer):
    """ L1Topo Phase I simulation flags """
    pass

class L1Phase1Sim(JobPropertyContainer):
    """ L1 Phase I simulation flags for L1Calo, L1Muon, L1Topo, CTP"""
    pass

from TriggerJobOpts.TriggerFlags import TriggerFlags as tf
tf.add_Container( L1Phase1Sim )
tf.L1Phase1Sim.add_Container( Calo )
tf.L1Phase1Sim.add_Container( Topo )
tf.L1Phase1Sim.add_Container( CTP )

CTPPhase1SimFlags  = tf.L1Phase1Sim.CTP
TopoPhase1SimFlags  = tf.L1Phase1Sim.Topo
L1CaloPhase1SimFlags = tf.L1Phase1Sim.Calo
L1Phase1SimFlags = tf.L1Phase1Sim

for flag in _glflags:
    L1Phase1SimFlags.add_JobProperty(flag)

for flag in _caloflags:
    L1Phase1SimFlags.Calo.add_JobProperty(flag)

for flag in _ctpflags:
    L1Phase1SimFlags.CTP.add_JobProperty(flag)

for flag in _topoflags:
    L1Phase1SimFlags.Topo.add_JobProperty(flag)


del _caloflags
del _ctpflags
del _topoflags
del log
