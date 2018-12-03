#!/usr/bin/env python

# art-description: DxAOD MC16a TOPQ1 - top-xaod validation-cuts.txt
# art-type: grid
# art-output: output.root
# art-include: 21.2/AnalysisTop

import ROOT
from PathResolver import PathResolver
import subprocess, sys, shlex, random, shutil, os

# Create empty PoolFileCatalog
os.system('art.py createpoolfile')

# -- Settings --
cutfilename   = "validation-cuts.txt"
inputfilename = "/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/AnalysisTop/ContinuousIntegration/MC/p3390/mc16_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_TOPQ1.e6337_e5984_s3126_r9364_r9315_p3390/DAOD_TOPQ1.12720282._000339.pool.root.1"

# -- Move to a unique directory --
rundir = str(random.randrange(10**8))
os.mkdir(rundir)
os.chdir(rundir)

# -- Get the validation file path from the most recent location --
cutfilepath   = ROOT.PathResolver.find_file(cutfilename,
                                            "DATAPATH", 
                                            ROOT.PathResolver.RecursiveSearch)

# -- Print the file location for debugging --
print "ART Test : Using cutfile (%s) from %s"%(cutfilename, cutfilepath)
print "ART Test : Using inputfile %s"%(inputfilename)
print "Running on full statistics"

# -- Copy the cutfile locally to be updated -- 
shutil.copyfile(cutfilepath, cutfilename)

# -- Write the input file path to a temporary file --
inputfilepath = open("input.txt","w")
inputfilepath.write(inputfilename+"\n")
inputfilepath.close()

# -- Run top-xaod --
cmd  = "top-xaod %s input.txt"%(cutfilename)
proc = subprocess.Popen(shlex.split(cmd))
proc.wait()

# -- Print the art-result return code
print "art-result: " + str(proc.returncode)

# -- Check the return code and exit this script with that --
sys.exit( proc.returncode )

