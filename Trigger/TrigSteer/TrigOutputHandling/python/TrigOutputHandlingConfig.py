# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

def HLTResultMTMakerCfg():
   from TrigOutputHandlingConf import HLTResultMTMaker
   from AthenaMonitoring.GenericMonitoringTool import GenericMonitoringTool, defineHistogram

   m = HLTResultMTMaker()
   m.MonTool = GenericMonitoringTool('MonTool', HistPath='HLTResultMTMaker')
   m.MonTool.Histograms = [ defineHistogram( 'TIME_build', path='EXPERT', type='TH1F', title='Time of result construction in;[micro seccond]',
                                             xbins=100, xmin=0, xmax=1000 ),
                            defineHistogram( 'nstreams', path='EXPERT', type='TH1F', title='number of streams',
                                             xbins=60, xmin=0, xmax=60 ),
                            defineHistogram( 'nfrags', path='EXPERT', type='TH1F', title='number of HLT results',
                                             xbins=10, xmin=0, xmax=10 ),
                            defineHistogram( 'sizeMain', path='EXPERT', type='TH1F', title='Main (physics) HLT Result size;4B words',
                                             xbins=100, xmin=-1, xmax=999 ) ] # 1000 k span
   return m

def TriggerEDMSerialiserToolCfg(name):
   # Configuration helper methods
   def fullResultID(self):
      return 0

   def addCollection(self, typeNameAux, moduleIds):
      self.CollectionsToSerialize[typeNameAux] = moduleIds

   def addCollectionToMainResult(self, typeNameAux):
      self.addCollection(typeNameAux,moduleIds=[self.fullResultID()])

   def addCollectionListToResults(self, typeNameAuxList, moduleIds):
      for typeNameAux in typeNameAuxList:
         self.addCollection(typeNameAux, moduleIds)

   def addCollectionListToMainResult(self, typeNameAuxList):
      self.addCollectionListToResults(typeNameAuxList,moduleIds=[self.fullResultID()])

   # Add the helper methods to the TriggerEDMSerialiserTool python class
   from TrigOutputHandlingConf import TriggerEDMSerialiserTool
   TriggerEDMSerialiserTool.fullResultID = fullResultID
   TriggerEDMSerialiserTool.addCollection = addCollection
   TriggerEDMSerialiserTool.addCollectionToMainResult = addCollectionToMainResult
   TriggerEDMSerialiserTool.addCollectionListToResults = addCollectionListToResults
   TriggerEDMSerialiserTool.addCollectionListToMainResult = addCollectionListToMainResult

   # Create and return a serialiser tool object
   serialiser = TriggerEDMSerialiserTool(name)
   from collections import OrderedDict
   class OD(OrderedDict):
      """Purpose of this class is to present map (ordered by insertion order) interface on python side, 
      whereas the property to look like vector of such strings
      "type#key;id0,id1"
      when it gets to setting the serialiser property
      """
      def __repr__(self):
         return '[' +','.join( ['"'+str(typekey)+';'+','.join(map( lambda _:str(_), ids) )+'"'  for typekey,ids in self.iteritems()] ) + ']'
      def __str__(self):
         return self.__repr__()

   serialiser.CollectionsToSerialize = OD()
   return serialiser
