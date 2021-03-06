# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from MinderStateEngineBase import MinderStateEngineBase

class ContainerMinderStateEngine(MinderStateEngineBase):
    def __init__(self, minder, state):
        MinderStateEngineBase.__init__(self, state, minder.identifiedName)


        
        self.dispatchTable = {
            'queued'           : minder.fromQueued,
            'running'          : minder.fromRunning,
            'operatorCloseDown': minder.fromOperatorCloseDown,
            'done'             : minder.fromDone
            }

