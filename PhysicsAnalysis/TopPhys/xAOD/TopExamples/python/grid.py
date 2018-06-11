# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

# -*- coding: utf-8 -*-
import os
import sys
import distutils.spawn
import logger
import analysis
import ROOT
from PathResolver import PathResolver

try:
    import rucio.client
except:
    print logger.FAIL + 'DANGER DANGER DANGER' + logger.ENDC
    print 'Could not find rucio.client.Client. If you use setupATLAS (you should) then'
    print '"localSetupRucioClients" and run this again'
    sys.exit(1)

try:
    rucio = rucio.client.Client()
except:
    print logger.FAIL + 'DANGER DANGER DANGER' + logger.ENDC
    print 'Could not setup rucio.client.Client.'
    print 'Did you do voms-proxy-init -voms atlas ?'
    sys.exit(1)

availableDatasets = {}

class Sample:
    def __init__(self, name):
        self.name = name
        self.datasets = []

    def shortNameDatasets(self):
        output = []
        for ds in self.datasets:
            name = ds.replace('/','').split('.')
            output.append(name[1] + '.' + name[-1])
        return output

    def details(self, actuallyExists = []):
        n = len(self.datasets)

        suffix = ''
        if n != 1:
           suffix = 's'

        print logger.OKGREEN + '  %s ' % self.name + logger.ENDC + '(%d dataset%s)' % (n, suffix)
        for i, l in enumerate(self.datasets):
            note = 'Not found'
            if l in actuallyExists:
                note = 'Found'

            print '   -%2d %s %s' % (i, l, note)

def Add(name):
    availableDatasets[name] = Sample(name)
    return availableDatasets[name]

def AvailableDatasets():
    return availableDatasets

def Samples(names):
    samples = []
    for n in names:
        #removing whitespaces from concatenated lines - ANALYSISTO-553
        for ds in range(0,len(availableDatasets[n].datasets)):
            availableDatasets[n].datasets[ds]=availableDatasets[n].datasets[ds].replace(' ','')
        samples.append(availableDatasets[n])
    return samples

def basicInDSNameShortener(*args):
    # don't ask what args[0] is; just use args[1] (sorry for my lack of understanding of python)
    # if you want to use a different function defined outside this module, you don't need to bother
    inDSName = args[1]
    splitted = inDSName.split('.')

    runNumber = splitted[1]
    physicsName = splitted[2]
    if splitted[0] == "user" or splitted[0] == "group": #this is in case we run on private derivations, either produced with user or group role
        runNumber = splitted[2]
        physicsName = splitted[3]
    derivation = splitted[-2]
    tags = splitted[-1].replace('/','')

    #grid complains dataset names are too long
    #stupid grid
    if len(physicsName) > 20:
        physicsName = physicsName.split('_')[0]

    outDSName = runNumber + '.' + physicsName + '.' + derivation + '.' + tags
    return outDSName

class Config:
    code = 'top-xaod'
    cutsFile = 'nocuts.txt'

    gridUsername    = ''
    groupProduction = False
    suffix          = ''
    excludedSites   = ''
    forceSite       = ''
    noSubmit        = False
    CMake           = (os.getenv('ROOTCORE_RELEASE_SERIES')=='25') # ROOTCORE_RELEASE_SERIES variable used to identify release (CMake required for R21)
    mergeType       = 'Default' #None, Default, xAOD 
    destSE          = ''
    memory          = '2000' #in MB
    maxNFilesPerJob = ''
    otherOptions    = ''
    skipShowerCheck = False
    nameShortener   = basicInDSNameShortener # default shortener function
    customTDPFile   = None
    reuseTarBall    = False
    checkPRW        = False

    def details(self):
        cutsFileIsARealFile = checkForFile(self.settingsFile)
        txt = '(' + logger.FAIL + 'not found' + logger.ENDC + ')'
        if cutsFileIsARealFile:
            txt = '(exists)'

        print logger.OKBLUE + 'Analysis Settings:' + logger.ENDC
        print ' -Code:          ', self.code
        print ' -CutsFile:      ', self.settingsFile, txt

        print logger.OKBLUE + 'Grid Settings:' + logger.ENDC
        print ' -GridUsername:  ', self.gridUsername
        print ' -Suffix:        ', self.suffix
        print ' -ExcludedSites: ', self.excludedSites
        print ' -ForceSite:     ', self.forceSite
        print ' -NoSubmit:      ', self.noSubmit
        print ' -MergeType:     ', self.mergeType, 'out of (None, Default, xAOD)'
        print ' -memory:        ', self.memory, 'in MB'
        print ' -maxNFilesPerJob', self.maxNFilesPerJob        
        print ' -skipShowerCheck', self.skipShowerCheck        
        print ' -OtherOptions:  ', self.otherOptions 
        print ' -nameShortener: ', self.nameShortener
        print ' -reuseTarBall:  ', self.reuseTarBall
        print ' -checkPRW:      ', self.checkPRW

        txt = self.destSE
        if len(txt) == 0:
            txt = '<not set>'
        print ' -DestSE         ', txt

        print ''

def checkMergeType(configuration):
    type = configuration.mergeType
    if type != "None" and type != "Default" and type != "xAOD":
        print "MergeType must be set to either None, Default or xAOD"
        print 'Not', type, "Since that doesn't make sense"
        sys.exit()

#If settings.txt doesn't exist your grid job will fail
def checkForFile(filename):
    return os.path.exists(filename)

def checkForPrun():
    if distutils.spawn.find_executable('prun') == None:
        print logger.FAIL + 'DANGER DANGER DANGER' + logger.ENDC
        print 'Could not find prun. If you use setupATLAS (you should) then'
        print '"localSetupPandaClient --noAthenaCheck" and run this again'
        sys.exit()

#I see a crash if I setup with rcSetup Top,1.4.0
#and don't run rc find_packages
#so let's run this by default
def findPackages():
    cmd = 'cd $ROOTCOREBIN/../; rc find_packages'

    debug = False

    if debug:
        print 'finding packages'
        print cmd

    for l in os.popen(cmd).readlines():
        if debug:
            print l.strip()

    if debug:
        print 'done'

#Given a list of datasets, the command to run and a mode (egamma, muons) this
#submits one prun job per run
#This is mostly for internal use of the code
def submit(config, allSamples):
  checkForPrun()
  checkMergeType(config)
  config.details()
  if not config.skipShowerCheck:
      checkForShowerAlgorithm(allSamples, config.settingsFile)
  if config.checkPRW:
      checkPRWFile(allSamples, config.settingsFile)

  tarfile = 'top-xaod.tar.gz'

  # Delete the old tarball if requested
  if not config.reuseTarBall:
      try:
          os.remove(tarfile)
      except OSError, e:
          #Number 2 is 'file doesn't exist' which is okay for us
          if e.errno == 2:
              pass
          else:
              raise


  #Check for cuts file
  if not checkForFile(config.settingsFile):
      print logger.FAIL    + " Error - %s does not exist in this directory "%(config.settingsFile) + logger.ENDC
      print logger.WARNING + "       - Attempt to find this file in a sensible location... " + logger.ENDC
      settingsFilePath = ROOT.PathResolver.find_file(config.settingsFile, "DATAPATH", ROOT.PathResolver.RecursiveSearch)      
      if settingsFilePath == "":
          print logger.FAIL + "DANGER DANGER. HIGH VOLTAGE" + logger.ENDC
          print '%s does not exist in this directory and cannot be found' % config.settingsFile
          print 'Please make it before submitting'
          sys.exit(1)      
      else:
          print logger.WARNING + "       - Found an appropriate file " + logger.ENDC
          print logger.WARNING + "       - Will copy " + logger.ENDC + config.settingsFile + logger.WARNING + " from " + logger.ENDC + settingsFilePath 
          print logger.WARNING + "       - Confirm this is okay before continuing " + logger.ENDC
          user_check = raw_input(logger.OKBLUE + "Type yes/y/Y in order to proceed ...: " + logger.ENDC)
          if(user_check != "yes" and user_check != "y" and user_check != "Y"):
              print logger.FAIL + " Exiting submission " + logger.ENDC
              sys.exit(2)
          print logger.OKGREEN + "       - Confirmed " + logger.ENDC
          os.system("cp %s %s"%(settingsFilePath,"./"))

  #Look in the cuts file for the output filename
  outputFilename = 'EMPTY'
  for l in open(config.settingsFile):
      #ignore text after comments
      if l.find('#') > -1:
          l = l.split('#')[0]

      if l.find('OutputFilename') > -1:
          outputFilename = l.replace('OutputFilename', '').strip()
          outputFilename = outputFilename.replace(".root","_root") + ":" + outputFilename

  if outputFilename == 'EMPTY':
      print 'OutputFilename not found in %s' % config.settingsFile
      sys.exit(1)

  #print outputFilename

  these = []
  print logger.OKBLUE + 'For these samples' + logger.ENDC
  
  #Only submit jobs if the dataset actually exists (check with rucio)
  for sample in allSamples:
      currentDatasets = sample.datasets
      actuallyExists = []
      for ds in currentDatasets:
          # doing this check only for the first sample, in case of coma-separated list of samples with same DSID and same first tag of each type
          # a priori it's not a big deal if the additional datasets don't exist; panda will take care of it
          if checkDatasetExists(getShortenedConcatenatedSample(ds)):
              actuallyExists.append(ds)

      sample.details(actuallyExists)
      these += actuallyExists

  #check if it's a release - which automatically would set rootVer and cmtConfig
  if not config.CMake:
      findPackages()

  plural = ''
  if len(these) > 1:
      plural = 's'

  print ''
  print logger.OKBLUE + 'Starting submission of %d sample%s' % (len(these), plural) + logger.ENDC
  print ''

  isfirst = True
  for i, d_concatenated in enumerate(these):
     d = getShortenedConcatenatedSample(d_concatenated) # in case of coma-separated list of samples with same DSID and same first tag of each type
     print logger.OKBLUE + 'Submitting %d of %d' % (i+1, len(these)) + logger.ENDC

     #Make the output dataset name
     #for group production it has to start with "group." and we asume that gridUsername is the name of the group (e.g. phys-top)
     if config.groupProduction:
         output = 'group.' + config.gridUsername + '.' + config.nameShortener(d) + '.' + config.suffix
     else:
         output = 'user.' + config.gridUsername + '.' + config.nameShortener(d) + '.' + config.suffix

     cmd = 'prun \\\n'
     #special care for group production - we assume that gridUsername is the name of the group (e.g. phys-top)
     if config.groupProduction:
         cmd += '--official --voms atlas:/atlas/' + config.gridUsername + '/Role=production  \\\n'
     cmd += '--inDS=' + d_concatenated + ' \\\n' # the inDS may be a comma-separated list of samples with same DSID and same first tag of each type
     cmd += '--outDS=' + output + ' \\\n'
     if config.CMake:
         CMTCONFIG = os.getenv("CMTCONFIG")
         cmd += '--useAthenaPackages --cmtConfig=%s \\\n'%(CMTCONFIG)
     else:
         cmd += '--useRootCore \\\n'
     cmd += '--writeInputToTxt=IN:in.txt \\\n'
     cmd += '--outputs=%s \\\n' % outputFilename
     cmd += '--exec="%s %s in.txt" \\\n' % (config.code, config.settingsFile)

     #you might really hate a certain site
     if len(config.excludedSites) > 0:
         cmd += '--excludedSite=' + config.excludedSites + ' \\\n'

     #you might really like a certain site
     if len(config.forceSite) > 0:
         cmd += '--site ' + config.forceSite + ' \\\n'

     #tar-up the first time only, to save time when submitting
     if isfirst:
         if checkForFile(tarfile) and config.reuseTarBall:# reuse existing tarball if it already exists
            print logger.OKBLUE + 'Reusing existing tarball %s' % (tarfile) + logger.ENDC
            cmd += '--inTarBall=%s \\\n' % tarfile
         elif config.reuseTarBall:# reuse existing tarball if it already exists
            print logger.WARNING + 'Tarball %s not found - will re-create it' % (tarfile) + logger.ENDC
            cmd += '--outTarBall=%s \\\n' % tarfile
         else:
            cmd += '--outTarBall=%s \\\n' % tarfile
         isfirst = False
     else:
         cmd += '--inTarBall=%s \\\n' % tarfile


     #maybe you don't want to submit the job?
     if config.noSubmit:
         cmd += '--noSubmit \\\n'

     #fewer files = happy grid
     if config.mergeType != "None":
         cmd += '--mergeOutput \\\n'

     #Write the output to a specific storage element?
     if len(config.destSE) > 0:
         cmd += '--destSE=\"%s\" \\\n' % config.destSE

     #xAOD merging - slide 9 https://indico.cern.ch/event/401703/contribution/2/2/material/slides/0.pdf
     #Only add TriggerMenuMetaDataTool option when xAODTriggerCnv is in the release (not in 2.3.15)
     #--mergeScript="xAODMerge -m xAODMaker::TriggerMenuMetaDataTool %OUT \`echo %IN | sed \'s/,/ /g\'\`" \\\n'
     if config.mergeType == "xAOD":
         cmd += '--mergeScript="xAODMerge %OUT \`echo %IN | sed \'s/,/ /g\'\`" \\\n'
         
     #how much memory, not sure what default is when not set
     if len(config.memory) > 0:
         cmd += '--memory=%s \\\n' % config.memory
         
     #how many files
     if len(config.maxNFilesPerJob) > 0:
         cmd += '--maxNFilesPerJob=%s \\\n' % config.maxNFilesPerJob

     #other options
     if len(config.otherOptions) > 0:
         cmd += '%s \\\n' % config.otherOptions

     #Make sure the command actually ends
     cmd += '\n'

     #show people what you're about to do
     print cmd

     #then do it
     for l in os.popen(cmd).readlines():
         print '>> ', l.strip()

#Make a directory if it doesn't exist
def makeDirectory(outputDirectory):
    try:
        os.makedirs(outputDirectory)
        print 'Made directory', outputDirectory
    except:
        #directory already exists
        pass

#Loop through the samples and change the AOD-style name to a DAOD_TOPQ one
#The user needs to give derivationToUse (DAOD_TOPQ1 or DAOD_TOPQ2)
#The p-tag for the production
#And the list of samples
def convertAODtoTOPQ(derivationToUse, ptag, samples):
    for sample in samples:
        for i, ds in enumerate(sample.datasets):
            sample.datasets[i] = ds.replace('AOD', derivationToUse).replace('/', '') + '_' + ptag

#Get a list of dataset names matching some pattern with stars in, using dq2-ls
def listDatasets(theScope, datasetPattern):
    response = rucio.list_dids(scope = theScope, filters = {'name' : datasetPattern})

    names = []
    for l in response:
        names.append(l)

    return names

#Download all datasets that match the pattern, to the outputDirectory
def download(theScope, datasetPattern, outputDirectory):
    #Make sure the directory where these need to go exists
    makeDirectory(outputDirectory)

    #Get datasets matching the pattern
    intdatasets = listDatasets(theScope, datasetPattern)
    txt = ''
    if len(intdatasets) != 1:
        txt = 's'

    print 'Found %d dataset%s:' % (len(intdatasets), txt)

    for ds in intdatasets:
        print '    %s' % ds

    for j, d in enumerate(intdatasets):
        cmd = 'rucio download %s:%s --dir %s' % (theScope, d, outputDirectory)
        print logger.OKBLUE + 'Dataset %d of %d: %s' % (j+1, len(intdatasets), cmd) + logger.ENDC

        #Run the dq2 command
        os.system(cmd)

#Prun submits to datasets that don't exist, so do a quick check first...
def checkDatasetExists(name):
    theScope = name.split('.')[0]
    #name = 'data15_13TeV.00266904.physics_Main.merge.DAOD_TOPQ1.f594_m1435_p2361'
    if theScope == "user" or theScope == "group":
           theScope = name.split('.')[0] + "." + name.split('.')[1]
    reply = listDatasets(theScope, name)
    #print reply
    return len(reply) == 1

if __name__ == '__main__':
    print "You don't run this directly!"
    print 'For an example, see 01SubmitToGrid.py'


def checkForShowerAlgorithm(Samples, cutfile):    
    noShowerDatasets = []
    customTDPFile = None
    tmp = open(cutfile, "r")
    for line in tmp.readlines():
        if "TDPPath" not in line:
            continue
        else:
            customTDPFile = line.strip().split("TDPPath")[1]
            break
    print customTDPFile
    if customTDPFile:
        tdpFile = ROOT.PathResolver.find_file(customTDPFile, "PATH", ROOT.PathResolver.RecursiveSearch)
    else:
        tdpFile = ROOT.PathResolver.find_file("dev/AnalysisTop/TopDataPreparation/XSection-MC15-13TeV.data", "CALIBPATH", ROOT.PathResolver.RecursiveSearch)
    # Load the file
    print tdpFile
    tdp = analysis.TopDataPreparation(tdpFile)
    for TopSample in availableDatasets.values():
        for List in Samples:
            SublistSamples = List.datasets
            for sample_concatenated in SublistSamples:
                sample=getShortenedConcatenatedSample(sample_concatenated) # in the case of comma-separated samples with same DSIDs and same first tags (it's the same sample)
                scope = sample.split('.')[0]
                if 'mc' not in scope:
                    continue
                dsid = sample.split('.')[1]
                dsid = int(dsid)
                hasShower = tdp.getShower(dsid) in ['sherpa','sherpa21','pythia','pythia8','herwigpp']
                #print hasShower,"   ",sample
                if not tdp.hasID(dsid) or not hasShower:
                    noShowerDatasets += [dsid]

    if len(noShowerDatasets) > 0:
        print 'The following datasets do not have a showering algorithm defined in TopDataPreparation and will fail on the grid. Please ask for this to be fixed in TopDataPreparation!'
        for ds in set(noShowerDatasets):
            print ds
        raise RuntimeError("Datasets without shower.")

def checkPRWFile(Samples, cutfile):
    # Some imports
    import subprocess, shlex

    # We need to find the PRW files being used and make use of the checkPRW 
    # checkPRW.py --inDsTxt=my.datasets.txt  path/to/prwConfigs/*.root
    # First, find the PRW names from cutfile
    print logger.OKBLUE + " - Processing checks for PRWConfig" + logger.ENDC
    tmp = open(cutfile, "r")
    PRWConfig = None
    for line in tmp.readlines():
        if "PRWConfigFiles" not in line:
            continue
        else:
            PRWConfig = [ ROOT.PathResolver.find_file( x, "CALIBPATH", ROOT.PathResolver.RecursiveSearch ) for x in line.strip().split()[1:] ]
            PRWConfig.extend( [ ROOT.PathResolver.find_file( x, "DATAPATH", ROOT.PathResolver.RecursiveSearch ) for x in line.strip().split()[1:] ]  )
            PRWConfig.extend( [ ROOT.PathResolver.find_file( x, "PATH", ROOT.PathResolver.RecursiveSearch ) for x in line.strip().split()[1:] ]  )

    if not PRWConfig:
        print logger.FAIL + " - Error reading PRWConfigFiles from cutfile" + logger.ENDC
        return 
    # Print the PRW files
    print logger.OKGREEN + "\n".join(PRWConfig) + logger.ENDC
    # Create a temporary sample list
    tmpFileName = "samplesforprwcheck.txt"
    tmpOut = open(tmpFileName,"w")
    for List in Samples:
        SublistSamples = List.datasets
        for sample_concatenated in SublistSamples: # the listed samples may be comma-separated list of samples
            for sample in sample_concatenated.split(','): # we need to check all of them, not just the first one
                tmpOut.write(sample+"\n")
    tmpOut.close()
    # Make a command
    cmd = "checkPRW.py --inDsTxt %s %s"%(tmpFileName, " ".join(PRWConfig))
    print logger.OKBLUE + " - Running command : " + cmd + logger.ENDC
    # Run
    proc = subprocess.Popen(shlex.split(cmd))
    proc.wait()
    # At the moment, just print the output, but we need to learn what to catch also

## gets the first AMI tag of a kind
def getFirstTag(tags,letter):
    tagList = tags.split('_')
    first = ''
    for tag in tagList:
        if tag.find(letter,0,1) != -1 and tag[1:].isdigit() and first == '':
            first = tag
    return first


# In MC16, a given DSID can have been "split" into several datasets, were some have more ami-tags
# This function takes as input a coma-separated list of datasets, and returns the name of the first sample if we are in such case
# It throws an error if the DSID of these datasets is different, or if the first ami-tag of each time is different for the different datasets
def getShortenedConcatenatedSample(sampleName):
    samples = sampleName.split(',')
    if len(samples) == 1: # the simplest case
        return samples[0]
    
    # check if the DSIDs are all the same
    DSID = samples[0].split('.')[1]
    firstTagOfFirstSample = { 'e':'', 's':'', 'a':'', 'r':'', 'f':'', 'm':'', 'p':'', }
    isFirstSample = True
    for s in samples:
        if s.split('.')[1] != DSID:
            print logger.FAIL + " Issue with this concatenated sample: " + sampleName + logger.ENDC
            print logger.FAIL + " This syntax can only work if all dataset containers have the same DSID " + logger.ENDC
            raise RuntimeError("Issue with contatenated samples.")
        AmiTags = s.split('.')[-1]
        for tagType in firstTagOfFirstSample:
            if firstTagOfFirstSample[tagType] == '' and isFirstSample:
                firstTagOfFirstSample[tagType] = getFirstTag(AmiTags,tagType)
            elif firstTagOfFirstSample[tagType] != getFirstTag(AmiTags,tagType):
                print logger.FAIL + " Issue with this concatenated sample: " + sampleName + logger.ENDC
                print logger.FAIL + " This syntax can only work if all dataset containers have the same first tag of each type " + logger.ENDC
                print logger.FAIL + " And it seems there are two samples in this list, one with " + firstTagOfFirstSample[tagType] + " and one with " + getFirstTag(AmiTags,tagType) + " as first " + tagType + "-tag" + logger.ENDC
                raise RuntimeError("Issue with contatenated samples.")
        isFirstSample = False
    return samples[0] # if we survived all the tests, return the first of the list
