# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

# File: AthExJobOptions/CustomToolUsingTool.py
# Author: Wim Lavrijsen (WLavrijsen@lbl.gov)

__version__ = '1.0.0'
__author__  = 'Wim Lavrijsen (WLavrijsen@lbl.gov)'

__all__ = [ 'CustomToolUsingTool' ]


from .AthExJobOptionsConf import ToolUsingTool


# Customized class, enable through derivation
class CustomToolUsingTool( ToolUsingTool ):
   __slots__ = []                  # enforce no new properties

   def __init__( self, name = 'CustomToolUsingTool' ):      # sets default name
      super( CustomToolUsingTool, self ).__init__( name )   # have to call base init

 # setDefaults is enforced to be a class method; because it is a class method,
 # the defaults can be savely queried by other tools, without touching any part
 # of the configuration as would happen otherwise
   def setDefaults( cls, handle ):
    # these values will override the C++ (i.e. developer) default values, not
    # any python (i.e. user) values
      handle.Factor = 42.          # best, is considered default

      if not hasattr( handle, 'TheToolTool' ):
         from .AthExJobOptionsConf import ConcreteTool
         handle.TheToolTool = ConcreteTool( 'ToolTool' )
