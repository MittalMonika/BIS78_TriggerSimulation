# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from IOVDbSvc.CondDB import conddb

# Inner Detector alignment
conddb.addFolderSplitOnline("INDET","/Indet/Onl/Align","/Indet/Align")
conddb.addFolderSplitOnline("TRT","/TRT/Onl/Align","/TRT/Align")
conddb.addFolderSplitOnline("TRT","/TRT/Onl/Calib/DX","/TRT/Calib/DX")

# Pixel module distortions
conddb.addFolderSplitOnline("INDET","/Indet/Onl/PixelDist","/Indet/PixelDist")
