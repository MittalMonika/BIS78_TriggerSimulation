#
# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration.
#
# File: TileByteStream/TileLaserObjByteStreamCnv_test.py
# Author: scott snyder
# Date: Oct, 2018
# Brief: Test TileLaserObjByteStreamCnv.
#

from __future__ import print_function

import os

refpaths = [os.environ.get ('ATLAS_REFERENCE_DATA', None),
            '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art',
            '/afs/cern.ch/atlas/maxidisk/d33/referencefiles']
def find_file (fname):
    for p in refpaths:
        if p:
            path = os.path.join (p, fname)
            if os.path.exists (path):
                return path
    print ('ERROR: Cannot find file: ', fname)
    return None

# Find reference and input files.
input_base = 'data18_tilecomm.00363899.calibration_tile.daq.RAW._lb0000._TileREB-ROS._0005-200ev.data'
if not globals().has_key ('ATLAS_REFERENCE_TAG'):
    ATLAS_REFERENCE_TAG = os.environ.get ('ATLAS_REFERENCE_TAG',
                                          'TileByteStream-01-00-00')
from AthenaCommon.Utils.unixtools import find_datafile
r = find_datafile (os.path.join ('TileByteStream', ATLAS_REFERENCE_TAG))
refdir = None
if r:
    refdir = os.path.join (r, 'TileLaserDumps')
    input_fname = os.path.join (r, input_base)
if not refdir or not os.path.exists (refdir):
    refdir = find_file (os.path.join ('TileByteStream',
                                      ATLAS_REFERENCE_TAG,
                                      'TileLaserDumps'))
    input_fname = find_file (os.path.join ('TileByteStream',
                                           ATLAS_REFERENCE_TAG,
                                           input_base))


from AthenaCommon.DetFlags      import DetFlags
DetFlags.detdescr.Tile_setOn()
DetFlags.detdescr.LAr_setOn()

import glob
import sys
import string
import ROOT
import math
from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit
from AtlasGeoModel import SetupRecoGeometry

from AthenaCommon.AthenaCommonFlags  import athenaCommonFlags
athenaCommonFlags.BSRDOInput = [input_fname]
from ByteStreamCnvSvc import ReadByteStream
svcMgr.EventSelector.Input = [input_fname]
from AthenaCommon.GlobalFlags import globalflags
globalflags.InputFormat.set_Value_and_Lock('bytestream')

svcMgr.ByteStreamAddressProviderSvc.TypeNames += [
    'TileLaserObject/TileLaserObj',
    ]

include('TileConditions/TileConditions_jobOptions.py')

from GeoModelSvc.GeoModelSvcConf import GeoModelSvc
ServiceMgr += GeoModelSvc()
theApp.CreateSvc += [ "GeoModelSvc"]
from AtlasGeoModel import TileGM
from AtlasGeoModel import LArGM   #LAr needed to get MBTS DD.

from IOVDbSvc.IOVDbSvcConf import IOVDbSvc
IOVDbSvc().GlobalTag = 'OFLCOND-RUN12-SDR-35'

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

theApp.EvtMax=100

from AthenaCommon.ConcurrencyFlags import jobproperties as jp
dumpdir = 'TileLaserDumps-%d' % jp.ConcurrencyFlags.NumThreads()

from TileRecUtils.TileRecUtilsConf import TileLaserObjectDumper
topSequence += TileLaserObjectDumper ('TileLaserObjectDumper',
                                      TileLaserObject = 'TileLaserObj',
                                      Prefix = dumpdir + '/')


os.system ('rm -rf ' + dumpdir)
os.system ('mkdir -p ' + dumpdir)

from AthenaPython.PyAthenaComps import Alg, StatusCode
class Finalizer (Alg):
    def finalize (self):
        dumps = glob.glob (os.path.join (refdir, '*.dump'))
        for f in dumps:
            localdump = os.path.join (dumpdir, os.path.basename (f))
            os.system ('diff -u %s %s' % (f, localdump))
        print ('Finalize: compared %d dumps' % len(dumps))
        return StatusCode.Success
topSequence += Finalizer ('Finalizer')
