# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.AlgSequence import AthSequencer
from MuonCondAlg.MuonTopCondAlgConfigRUN2 import MdtCondDbAlg

condSequence = AthSequencer("AthCondSeq")
if not hasattr(condSequence,"MdtCondDbAlg"):
    condSequence += MdtCondDbAlg("MdtCondDbAlg")

