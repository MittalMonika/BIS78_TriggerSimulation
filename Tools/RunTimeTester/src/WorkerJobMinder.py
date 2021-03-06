# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

"""
This file contains:
an intermediate base class         -   WorkerJobMinder

- Has a the command string.
- has a 'fire' method to submit the jobs
- has a popen method to monitor the subshell used to run the job.

The base class also specifies common tasks that must be performed across job mode
boundaries:

- database manipulations for jobs which store reference files
- various protection manipulations to allow other mebers of the same UNIX group
  to manipulate files on future runs.
- copying files to be stored at the end of the job
- actions to be taken of there is a forced shutdown
  (triggered by receiving a signal 15)
PS
"""


from ShellCommand          import ShellCommand
from formatCollection      import formatCollection
from Minder                import Minder
from Defs                  import RTTdefs

import os, shutil, sys, string, copy
# ------------------------------------------------------------------------

class WorkerJobMinder(Minder):    

    def __init__(self, argBag, jDescriptor):
        """
        resDir     - results directory: recieves the logfile
        status
        """

        # Minder __init__ needs a JobGroupDirectoryMaker to have been
        # instantiated
        Minder.__init__(self, argBag, jDescriptor)

        self.jobWeight      = 1 # These jobs are counted in the Launcher stats.
        self.subLog         = [] # to hold reply from the submission command
        self.logChecker     = copy.copy(argBag.jobGroupKit.logChecker)
        self.logResult      = 'unavailable' # should be cached in logChecker
        
        # extend the Minder checks to perform. Kit was obtained by base class
        self.checks.extend(argBag.jobGroupKit.getItem('checkers'))
        self.logChecker.logger = self.logger
        for c in self.checks: c.logger = self.logger
        # Add the log file to the keep files
        # directory depends on the submit command.
        # The name is stored in the job Descriptor at submission time        
        destLogName = self.fullResultsFileName(self.log)
        srcFile     = os.path.join(self.runPath,self.log)

        # add the log file as a keep file
        infoString = "Job Log file"
        displayColor = "#cc3333"

        self.keepFiles[srcFile] = {'keepFileString' : destLogName,
                                   'infoString'     : infoString,
                                   'displayColor'   : displayColor}

       
    # ------------------------------------------------------------------------

    def collectScripts(self, scriptWriter):
        Minder.collectScripts(self, scriptWriter)

        script               = scriptWriter.makeScript()

        dict = {'runScript': {'script':       script,
                              'scriptName':  'runScript.sh',
                              'cmd':          self.submitCommand # set in subclasses
                              }
                }
        
        self.scripts.update(dict)
       
    # ------------------------------------------------------------------------

    def checkLog(self):
        self.logResult        = RTTdefs.status(self.logChecker.check(self))
        self.processingResult = self.logResult # determines is post processing should be done.
        # log checker determines how many of the chained jobs succeeded.
        self.chainSuccesses   = self.logChecker.nSuccesses
        self.chainLength      = self.logChecker.chainLength
    # ------------------------------------------------------------------------
    
    def fire(self, what):
        """
        Submits the job by spawning a subprocess.
        The Popen objects captures std error, and std out
        'what' is a a key in to tthe self.scripts dictionary
        """

        cmd = self.scripts[what]['cmd']

        m =  'fire commandL\n: %s' % cmd
        self.logger.debug(m)

        sc = ShellCommand([cmd], self.logger)
        self.subLog = sc.getReply()


    # ------------------------------------------------------------------------

    def dataForMonitoring(self):
        dict = Minder.dataForMonitoring(self)
        dict['jobSuccess'] = self.isSuccess()
        dict['jobFailure'] = self.isFailure()
        return dict

    # ------------------------------------------------------------------------

    def isSuccess(self): return self.logResult=='success'

    # ------------------------------------------------------------------------

    def isFailure(self): return self.logResult=='error'

    # ------------------------------------------------------------------------

    def status(self):


        if self.logResult == 'unavailable': return self.stateEngine.state.state
        return self.logResult

    # ------------------------------------------------------------------------

    def __str__(self):

        s = Minder.__str__(self)
        s += '----------- Worker Minder ---------------\n'
        s += ' checks:              %s' % formatCollection(self.checks)
        return s
    

    def dump(self):
        self.logger.debug('|-------------------------------------------|')
        self.logger.debug('|                                           |')
        self.logger.debug('|        Worker Job Minder  dump            |')
        self.logger.debug('|                                           |')
        self.logger.debug('|-------------------------------------------|')

        self.logger.debug(self.__str__())
        
# ==================================================================
