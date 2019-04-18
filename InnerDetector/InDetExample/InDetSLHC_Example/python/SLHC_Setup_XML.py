""" SLHC_Setup
    Python module to hold storegate keys of InDet objects.
"""

__author__ =   "A. Salzburger"
__version__=   "$Revision: 1.13 $"
__doc__    =   "SLHC_PathSetting"
__all__    = [ "SLHC_PathSetting" ]

import os
from os.path import exists, join
from InDetSLHC_Example.SLHC_JobProperties import SLHC_Flags

from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import AtlasGeoDBInterface

class SLHC_Setup_XMLReader :
    # constructor requires the SLHC_Flags

    def __init__(self,**kwargs):

        from InDetTrackingGeometryXML.XMLReaderJobProperties import XMLReaderFlags
        print dir(XMLReaderFlags)
        if XMLReaderFlags.Initialized(): return

        import os, shutil
        from PyJobTransformsCore.envutil import find_file_env

        if SLHC_Flags.UseLocalGeometry():
            print "########################################"
            print "#######Using Local Geometry Files#######"
            print "########Only for private testing########"
            print "########################################"

        print "*******************************************************************************************"
        print "*******************************************************************************************"
        print "*******************************************************************************************"

        for key in kwargs:
            print "another keyword arg: %s: %s" % (key, kwargs[key])
        
        #--------------------------------------------------------------
        # XML reader
        #-------------------------------------------------------------

        ###### Setup ITk XML reader ######
        from InDetTrackingGeometryXML.InDetTrackingGeometryXMLConf import InDet__XMLReaderSvc
        xmlReader = InDet__XMLReaderSvc(name='InDetXMLReaderSvc')

        dictionaryFileName = ""
        if kwargs.has_key("dictionaryFileName"): dictionaryFileName = kwargs["dictionaryFileName"]
        createXML = False
        if kwargs.has_key("createXML"): createXML = kwargs["createXML"]
        doPix = kwargs["doPix"]
        doSCT = kwargs["doSCT"]
        isGMX = kwargs["isGMX"]
        AddBCL = False
        if kwargs.has_key("addBCL"): AddBCL = kwargs["addBCL"]
        
        PIXMODULEFILE = "ITK_PixelModules.xml"
        PIXSTAVEFILE  = str(kwargs["PixelLayout"]) + "_PixelStave.xml"
        PIXBARRELFILE = str(kwargs["PixelLayout"]) + "_PixelBarrel.xml"
        PIXENDCAPFILE = str(kwargs["PixelLayout"]) + "_PixelEndcap.xml"
        if kwargs.has_key("PixelEndcapLayout"): PIXENDCAPFILE = str(kwargs["PixelEndcapLayout"]) + "_PixelEndcap.xml"
        
        SCTMODULEFILE = "ITK_SCTModules.xml"
        SCTSTAVEFILE  = str(kwargs["SCTLayout"]) + "_SCTStave.xml"
        SCTBARRELFILE = str(kwargs["SCTLayout"]) + "_SCTBarrel.xml"
        SCTENDCAPFILE = str(kwargs["SCTLayout"]) + "_SCTEndcap.xml"

        ## ###### Setup dictionary file to use  ######
        ## dictDir = os.path.dirname(dictionaryFileName)
        ## if not os.path.exists(dictDir):
        ##     os.makedirs(dictDir)

        # Check if PIXXD node is define din the DB
        readXMLfromDB_PIXXDD = False
        from AthenaCommon.GlobalFlags import globalflags
        geoTagName = globalflags.DetDescrVersion()
        dbGeomCursor = AtlasGeoDBInterface.AtlasGeoDBInterface(geoTagName,False)
        dbGeomCursor.ConnectAndBrowseGeoDB()
        dbId,dbXDD,dbParam = dbGeomCursor.GetCurrentLeafContent("PIXXDD")
        if len(dbId)>0 and not SLHC_Flags.UseLocalGeometry(): 
            readXMLfromDB_PIXXDD = True
            #For reference
            #*********************** pixDbId
            #[37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48]
            #*********************** pixDbParam
            #['PIXXDD_DATA_ID', 'GEOTYPE', 'VERSION', 'KEYWORD', 'XMLCLOB']
            # Create local XML file correspondng to the XML CLOB defined in the geometry DB
            keywordIndex = dbParam.index("KEYWORD")
            clobIndex = dbParam.index("XMLCLOB")
            pathName = "/".join([os.environ["PWD"],"XML-"+geoTagName])
            if not os.path.exists(pathName): os.mkdir( pathName, 0755 )
            for key in dbXDD.keys():
                v = dbXDD[key]
                fileName = pathName+"/"+v[keywordIndex]+".xml"
                f=open(fileName,"w")
                f.write(v[clobIndex])
                f.close()

            pixBarrelLayout = pathName+"/"+str(PIXBARRELFILE)
            pixEndcapLayout = pathName+"/"+str(PIXENDCAPFILE)
            stripBarrelLayout = pathName+"/"+str(SCTBARRELFILE)
            stripEndcapLayout = pathName+"/"+str(SCTENDCAPFILE)

        else:

            pixBarrelLayout = find_file_env(str(PIXBARRELFILE),'DATAPATH')
            pixEndcapLayout = find_file_env(str(PIXENDCAPFILE),'DATAPATH')
            stripBarrelLayout = find_file_env(str(SCTBARRELFILE),'DATAPATH')
            stripEndcapLayout = find_file_env(str(SCTENDCAPFILE),'DATAPATH')

            fileList = [pixBarrelLayout,pixEndcapLayout,stripBarrelLayout,stripEndcapLayout]

            if None in fileList:
                if SLHC_Flags.UseLocalGeometry():
                    print 'ERROR: UseLocalGeometry flag set but Xml files not found - did you checkout and compile ITKLayouts?'
                else:
                    print 'ERROR: No DB CLOB or local geometry description Xml files found. You are probably running an unsupported layout! Please check your GeoTag or checkout ITKLayouts if you want to develop a new layout locally'
                exit(1)

        ###### Setup XMLreader flags
        print "**** FLAGS **************************************************"
        # need to set these in the interface to use kwargs syntax
        XMLReaderFlags.setValuesFromSetup( PixelBarrelLayout = pixBarrelLayout,
                                           PixelEndcapLayout = pixEndcapLayout,
                                           SCTBarrelLayout = stripBarrelLayout,
                                           SCTEndcapLayout = stripEndcapLayout,
                                           doPix = kwargs["doPix"],
                                           doSCT = kwargs["doSCT"],
					   isGMX = kwargs["isGMX"],
					   addBCL = AddBCL,
                                           readXMLfromDB = readXMLfromDB_PIXXDD
                                           )
        XMLReaderFlags.dump() #
        print "******************************************************"

        ###### Setup XML builders properties ######
        xmlReader.dictionaryFileName  = kwargs["dictionaryFileName"]
        xmlReader.createXMLDictionary = False
        if kwargs.has_key("createXML"): xmlReader.createXMLDictionary = kwargs["createXML"]
        xmlReader.doPix = XMLReaderFlags.doPix()
        xmlReader.doSCT = XMLReaderFlags.doSCT()
        xmlReader.isGMX = XMLReaderFlags.isGMX()
        xmlReader.readXMLfromDB = XMLReaderFlags.readXMLfromDB()

        if not XMLReaderFlags.readXMLfromDB():

            ###### Setup XML filesfor pixels ######
            xmlReader.XML_PixelModules      = find_file_env(str(PIXMODULEFILE),'DATAPATH')
            xmlReader.XML_PixelStaves       = find_file_env(str(PIXSTAVEFILE),'DATAPATH')
            xmlReader.XML_PixelBarrelLayers = XMLReaderFlags.PixelBarrelLayout()
            if XMLReaderFlags.PixelEndcapLayout()!='UNDEFINED' :
                xmlReader.XML_PixelEndcapLayers = XMLReaderFlags.PixelEndcapLayout()
            ###### Setup XML files for SCT ######
            xmlReader.XML_SCTModules      = find_file_env(str(SCTMODULEFILE),'DATAPATH')
            xmlReader.XML_SCTStaves       = find_file_env(str(SCTSTAVEFILE),'DATAPATH')
            xmlReader.XML_SCTBarrelLayers = XMLReaderFlags.SCTBarrelLayout()
            xmlReader.XML_SCTEndcapLayers = XMLReaderFlags.SCTEndcapLayout()
            
        else:

            ###### Setup XML filesfor pixels ######
            xmlReader.XML_PixelModules      = pathName+"/"+str(PIXMODULEFILE)
            xmlReader.XML_PixelStaves       = pathName+"/"+str(PIXSTAVEFILE)
            xmlReader.XML_PixelBarrelLayers = pathName+"/"+str(PIXBARRELFILE)
            if XMLReaderFlags.PixelEndcapLayout()!='UNDEFINED' :
                xmlReader.XML_PixelEndcapLayers = pathName+"/"+str(PIXENDCAPFILE)
            ###### Setup XML files for SCT ######
            xmlReader.XML_SCTModules      = pathName+"/"+str(SCTMODULEFILE)
            xmlReader.XML_SCTStaves       = pathName+"/"+str(SCTSTAVEFILE)
            xmlReader.XML_SCTBarrelLayers = pathName+"/"+str(SCTBARRELFILE)
            xmlReader.XML_SCTEndcapLayers = pathName+"/"+str(SCTENDCAPFILE)
            
        from AthenaCommon.AppMgr import theApp
        from AthenaCommon.AppMgr import ServiceMgr as svcMgr
        svcMgr += xmlReader
        theApp.CreateSvc.insert(0,"InDet::XMLReaderSvc/InDetXMLReaderSvc")
#        print svcMgr

        
        from AthenaCommon.AppMgr import theApp
        from AthenaCommon.AppMgr import ServiceMgr as svcMgr
        ReadStripXMLFromDB = False
        pathToGMX = ""
        gmxFileName = "ITkStrip.gmx"
        stripDbId,stripDbXDD,stripDbParam = dbGeomCursor.GetCurrentLeafContent("ITKXDD")
        if len(stripDbId)>0 and not SLHC_Flags.UseLocalGeometry():
            ReadStripXMLFromDB = True
            clobIndex = stripDbParam.index("XMLCLOB")
            pathName = "/".join([os.environ["PWD"],"XML-"+geoTagName])
            if not os.path.exists(pathName): os.mkdir( pathName, 0755 )
            for key in stripDbXDD.keys():
                #For reference
                #*********************** stripDbId
                #[6]
                #*********************** stripDbParam
                #['ITKXDD_DATA_ID', 'GEOTYPE', 'VERSION', 'XMLCLOB']
                v = stripDbXDD[key]
                pathToGMX = pathName+"/"+gmxFileName
                f=open(pathToGMX,"w")
                f.write(v[clobIndex])
                f.close()
            from shutil import copyfile
            dtdFileName = "geomodel.dtd"
            #dtd file is part of release, not CLOB, so need to find it
            #from InstallArea and copy it to expected location
            #which is specified in GMX file (not ideal... to be looked into)
            dtdFilePath = find_file_env(str(dtdFileName),'DATAPATH')
            dtdFileCopyPath = pathName+"/"+dtdFileName
            copyfile(dtdFilePath,dtdFileCopyPath)
        else:
            stripLocalGmx = find_file_env(str(gmxFileName),'DATAPATH')
            if stripLocalGmx is None:
                if SLHC_Flags.UseLocalGeometry():
                    print 'ERROR: UseLocalGeometry flag set but Gmx files not found - did you checkout and compile ITKLayouts?'
                else:
                    'ERROR: No DB CLOB or local geometry description Gmx files found. You are probably running an unsupported layout! Please check your GeoTag or checkout ITKLayouts if you want to develop a new layout locally'
                exit(1)
        from InDetTrackingGeometryXML.InDetTrackingGeometryXMLConf import InDet__GMXReaderSvc
        
        
        gmxReader = InDet__GMXReaderSvc(name='InDetGMXReaderSvc')
        gmxReader.dictionaryFileName=xmlReader.dictionaryFileName
        gmxReader.addBCL= XMLReaderFlags.addBCL()
        gmxReader.readGMXfromDB = ReadStripXMLFromDB
        gmxReader.pathToGMXFile = pathToGMX
        svcMgr += gmxReader
        theApp.CreateSvc.insert(1,"InDet::GMXReaderSvc/InDetGMXReaderSvc")
