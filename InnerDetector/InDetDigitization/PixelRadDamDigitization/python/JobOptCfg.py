# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

""" PixelRadDamDigitization/python/JobOptCfg.py
    Wrapper class to PixelRadDamDigitization generated by genconf.
"""
__version__ = ''
__author__  = ''
__doc__     = "PixelRadDamDigitization"
__all__ = [ 'CustomPixelRadDamDigitization' ]

# import genconf class
from GaudiKernel.GaudiHandles import ServiceHandle
from PixelRadDamDigitization.PixelRadDamDigitizationConf import PixelRadDamDigitization,PixelRadDamDigitizationTool
from AthenaCommon.OldStyleConfig import *
from AthenaCommon.Logging import logging
from AthenaCommon.AppMgr import theApp

# make custom class
class CustomPixelRadDamDigitization( PixelRadDamDigitization ):
   """Wrapper class to PixelRadDamDigitization"""
   __slots__ = []                                          # enforce no new properties
   __logger__ = logging.getLogger( 'PixelRadDamDigitization' )   # logger
   __iovdbsvc__ = Service("IOVDbSvc")
   #
   def __init__( self, name = 'CustomPixelRadDamDigitization' ): # sets default name
      """Constructor. Sets all properties to their default values"""
      super( PixelRadDamDigitization, self ).__init__( name )    # have to call base init
      #
      # set all default attributes
      #
      self.setAllDefaults()
      #
      return
      
   # setDefaults is enforced to be a class method; because it is a class method,
   # the defaults can be savely queried by other tools, without touching any part
   # of the configuration as would happen otherwise
   def setDefaults( self, handle ):
      """Set default values - NOT YET IMPLEMENTED."""
      # these values will override the C++ (i.e. developer) default values, not
      # any python (i.e. user) values
      #self.__logger__.info("setDefaults() called")
      #print handle
      return
   
   def setAllDefaults( self ):
      """
      Set all properties to their default values.
      """
      # this will set all properties to default value
      
      #### handle.RndmFactor = 42.      # best, is considered default
      
      from AthenaCommon.Configurable import ConfigurableAlgorithm
      # make sure one tool is always selected
      self.__logger__.info("setting all default values")
      allprops = self.getDefaultProperties()
      for k,v in allprops.items():
         if k in ConfigurableAlgorithm.__slots__:
            continue
         setattr( self, k, v )
      # set i/o object names - they're set above, but to the default from SiDigitization
      rname = getattr(self,"RndmEngine")
      if len(rname)<1:
         defengine = "PixelRadDamDigitization"
         exec '__logger__.warning("no random engine name specified! Setting engine name to %s")' % defengine
         self.setattr("RndmEngine",defengine)
      # set random number seed
###      self.setRndSeed( self.getDefaultProperty("RndmSeed1"), self.getDefaultProperty("RndmSeed2") )
      
      return

   def setIOVonce( self ) :
      """IOV is infinite: regenerate a simulated special pixel map only once"""
      self.IOVFlag = 0
      return

   def setIOVall( self ) :
      """IOV is one: regenerate a simulated special pixel map every event"""
      self.IOVFlag = 1
      return
   
   def setIOVstep( self, step ) :
      """IOV is <step> : regenerate a simulated special pixel map at every <step> event"""
      self.IOVFlag  = 2
      self.EventIOV = step
      return

   def setIOVrnd( self, step ) :
      """IOV is randomly chosen in [0,step] : regenerate a simulated special pixel map at random events"""
      self.IOVFlag = 3
      self.EventIOV = step
      return

#
# not sure if this one is relevant.
# the idea was to 
#
   def dbfolderContains( self, tag ):
      """Check if a certain tag is already in IOVDbSvc.Folders"""
      name = tag.strip()
      allFolders = self.__iovdbsvc__.Folders
      nFolders   = len(allFolders)
      done     = (nFolders < 1)
      index    = 0
      found    = False
      while not done:
         folder = allFolders[index].strip()
         found = (folder==name)
         if not found:
            index = index + 1
         done = ( index==nFolders ) or ( found )
         #
      if not found:
         index = -1
      #
      return index

#   def setUseCalibCondDB( self ):
#      """Set calib db folder"""
      
   #
   # The following handle the random number seed list in the rnd service.
   # They will most likely be obsolete in the near future.
   #
   def removeRndSeed( self ):
      """Remove the random seed with the set name (RndmEngine)"""
      # remove a seed associated with the digitization
      name = getattr(self,"RndmEngine")
      rndmSvc = Service(self.RndmSvc.getName())
      rndmSvc.Seeds += []
      allSeeds = rndmSvc.Seeds
      #
      nseeds   = len(allSeeds)
      done     = (nseeds < 1)
      index    = 0
      found    = False
      while not done:
         seed = allSeeds[index]
         found = seed.startswith(name+" ")
         index = index + 1
         done = ( index==nseeds ) or ( found )
         #
      if found:
         allSeeds.remove(allSeeds[index-1])
         rndmSvc.Seeds = allSeeds
      return
   
   def setRndSeed( self, seed1, seed2 ):
      """Set random seed using the name given by RndmEngine"""
      # set a random number seed - OBSOLETE
      name = getattr(self,"RndmEngine")
      self.removeRndSeed()
      newseed = name + " " + str(seed1) + " " + str(seed2)
      rndmSvc = Service(self.RndmSvc.getName())
      rndmSvc.Seeds += [ newseed ]
#      setattr(self,"RndmSeed1",seed1)
#      setattr(self,"RndmSeed2",seed2)
      return

   def printRndSeed( self ):
      """print random seed setting"""
      name = getattr(self,"RndmEngine")
      rndmSvc = Service(self.RndmSvc.getName())
      rndmSvc.Seeds += []
      allSeeds = rndmSvc.Seeds
      #
      nseeds   = len(allSeeds)
      done     = (nseeds < 1)
      index    = 0
      found    = False
      while not done:
         seed = allSeeds[index]
         found = seed.startswith(name+" ")
         index = index + 1
         done = ( index==nseeds ) or ( found )
         #
      if found:
         exec 'self.__logger__.info("Random number service : %s")' % self.RndmSvc.getName()
         exec 'self.__logger__.info("Random number seeds   : %s")' % allSeeds[index-1]
      else:
         exec 'self.__logger__.info("No random number seeds for key : %s")' % name
      return
      

# make custom class
class CustomPixelRadDamDigitizationTool( PixelDigitizationTool ):
   """Wrapper class to PixelDigitizationTool"""
   __slots__ = []                                          # enforce no new properties
   __logger__ = logging.getLogger( 'PixelDigitizationTool' )   # logger
   __iovdbsvc__ = Service("IOVDbSvc")
   #
   def __init__( self, name = 'CustomPixelDigitizationTool' ): # sets default name
      """Constructor. Sets all properties to their default values"""
      super( PixelDigitizationTool, self ).__init__( name )    # have to call base init
      #
      # set all default attributes
      #
      self.setAllDefaults()
      #
      return
      
   # setDefaults is enforced to be a class method; because it is a class method,
   # the defaults can be savely queried by other tools, without touching any part
   # of the configuration as would happen otherwise
   def setDefaults( self, handle ):
      """Set default values - NOT YET IMPLEMENTED."""
      # these values will override the C++ (i.e. developer) default values, not
      # any python (i.e. user) values
      #self.__logger__.info("setDefaults() called")
      #print handle
      return
   
   def setAllDefaults( self ):
      """
      Set all properties to their default values.
      """
      # this will set all properties to default value
      
      #### handle.RndmFactor = 42.      # best, is considered default
      
      from AthenaCommon.Configurable import ConfigurableAlgorithm
      # make sure one tool is always selected
      self.__logger__.info("setting all default values")
      allprops = self.getDefaultProperties()
      for k,v in allprops.items():
         if k in ConfigurableAlgorithm.__slots__:
            continue
         setattr( self, k, v )
      # set i/o object names - they're set above, but to the default from SiDigitization
      rname = getattr(self,"RndmEngine")
      if len(rname)<1:
         defengine = "PixelDigitization"
         exec '__logger__.warning("no random engine name specified! Setting engine name to %s")' % defengine
         self.setattr("RndmEngine",defengine)
      # set random number seed
###      self.setRndSeed( self.getDefaultProperty("RndmSeed1"), self.getDefaultProperty("RndmSeed2") )
      
      return

   def setIOVonce( self ) :
      """IOV is infinite: regenerate a simulated special pixel map only once"""
      self.IOVFlag = 0
      return

   def setIOVall( self ) :
      """IOV is one: regenerate a simulated special pixel map every event"""
      self.IOVFlag = 1
      return
   
   def setIOVstep( self, step ) :
      """IOV is <step> : regenerate a simulated special pixel map at every <step> event"""
      self.IOVFlag  = 2
      self.EventIOV = step
      return

   def setIOVrnd( self, step ) :
      """IOV is randomly chosen in [0,step] : regenerate a simulated special pixel map at random events"""
      self.IOVFlag = 3
      self.EventIOV = step
      return

#
# not sure if this one is relevant.
# the idea was to 
#
   def dbfolderContains( self, tag ):
      """Check if a certain tag is already in IOVDbSvc.Folders"""
      name = tag.strip()
      allFolders = self.__iovdbsvc__.Folders
      nFolders   = len(allFolders)
      done     = (nFolders < 1)
      index    = 0
      found    = False
      while not done:
         folder = allFolders[index].strip()
         found = (folder==name)
         if not found:
            index = index + 1
         done = ( index==nFolders ) or ( found )
         #
      if not found:
         index = -1
      #
      return index

#   def setUseCalibCondDB( self ):
#      """Set calib db folder"""
      
   #
   # The following handle the random number seed list in the rnd service.
   # They will most likely be obsolete in the near future.
   #
   def removeRndSeed( self ):
      """Remove the random seed with the set name (RndmEngine)"""
      # remove a seed associated with the digitization
      name = getattr(self,"RndmEngine")
      rndmSvc = Service(self.RndmSvc.getName())
      rndmSvc.Seeds += []
      allSeeds = rndmSvc.Seeds
      #
      nseeds   = len(allSeeds)
      done     = (nseeds < 1)
      index    = 0
      found    = False
      while not done:
         seed = allSeeds[index]
         found = seed.startswith(name+" ")
         index = index + 1
         done = ( index==nseeds ) or ( found )
         #
      if found:
         allSeeds.remove(allSeeds[index-1])
         rndmSvc.Seeds = allSeeds
      return
   
   def setRndSeed( self, seed1, seed2 ):
      """Set random seed using the name given by RndmEngine"""
      # set a random number seed - OBSOLETE
      name = getattr(self,"RndmEngine")
      self.removeRndSeed()
      newseed = name + " " + str(seed1) + " " + str(seed2)
      rndmSvc = Service(self.RndmSvc.getName())
      rndmSvc.Seeds += [ newseed ]
#      setattr(self,"RndmSeed1",seed1)
#      setattr(self,"RndmSeed2",seed2)
      return

   def printRndSeed( self ):
      """print random seed setting"""
      name = getattr(self,"RndmEngine")
      rndmSvc = Service(self.RndmSvc.getName())
      rndmSvc.Seeds += []
      allSeeds = rndmSvc.Seeds
      #
      nseeds   = len(allSeeds)
      done     = (nseeds < 1)
      index    = 0
      found    = False
      while not done:
         seed = allSeeds[index]
         found = seed.startswith(name+" ")
         index = index + 1
         done = ( index==nseeds ) or ( found )
         #
      if found:
         exec 'self.__logger__.info("Random number service : %s")' % self.RndmSvc.getName()
         exec 'self.__logger__.info("Random number seeds   : %s")' % allSeeds[index-1]
      else:
         exec 'self.__logger__.info("No random number seeds for key : %s")' % name
      return
      

