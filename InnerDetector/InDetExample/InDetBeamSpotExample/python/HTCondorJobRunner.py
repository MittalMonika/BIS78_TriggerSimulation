#!/usr/bin/env python

# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
"""
LSFJobRunner is a JobRunner for running jobs with the HTCondor batch system.

Written by Anthony Morley in 2019.
"""
__author__  = 'Anthony Morley'
__version__ = '$Id: HTCondorJobRunner.py 216126 2009-09-29 16:12:59Z atlidbs $'


from JobRunner import JobRunner
import os

condorScriptTemplate="""executable            = %(scriptfile)s
arguments             = $(ClusterID) $(ProcId)
output                = %(logfile)s.out 
error                 = %(logfile)s.err
log                   = %(logfile)s.log
universe              = vanilla
+JobFlavour           = %(batchqueue)s  
queue
"""

class HTCondorJobRunner(JobRunner):
    """HTCondorJobRunner - run jobs using the HTCondor batch system"""

    def __init__(self,**params):
        """Constructor (takes any number of parameters as an argument)."""
        JobRunner.__init__(self)

        # Set user-specified parameters only after HTCondorJobRunner defaults
        self.setParam('batchqueue','workday','Batch queue')
        for k in params.keys():
            self.setParam(k,params[k])
        self.checkParams()

    def submitJob(self,jobConfig):
        """Submit a JobRunner job as a LSF batch job."""
        condorScript = condorScriptTemplate % jobConfig 
        print condorScript
        script = open('condorSubmit.sub','w')
        script.write(condorScript)
        script.close()
        os.chmod('condorSubmit.sub',0755)
        #batchCmd = 'bsub -L /bin/bash -q %(batchqueue)s -J %(jobname)s -o %(logfile)s %(scriptfile)s' % jobConfig
        batchCmd = 'condor_submit condorSubmit.sub'
        print batchCmd 
        os.system(batchCmd)
        return None
