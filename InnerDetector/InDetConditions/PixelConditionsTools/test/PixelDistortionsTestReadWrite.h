/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// PixelDistortionsTestReadWrite.h
// 
// An example algorithm to test PixelDistortionsTool 
//
// author Grant Gorfine <Grant.Gorfine@cern.ch>
//
//****************************************************************************



#ifndef PixelDistortionsTestReadWrite_H
#define PixelDistortionsTestReadWrite_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include "PixelConditionsTools/IModuleDistortionsTool.h"

namespace InDetDD
{
  class PixelDetectorManager;
}


class PixelDistortionsTestReadWrite: public AthAlgorithm{

 public: 
  
  PixelDistortionsTestReadWrite(const std::string& name, ISvcLocator* pSvcLocator);
  ~PixelDistortionsTestReadWrite();  

  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();


 private:

  //mutable MsgStream m_log;
  const InDetDD::PixelDetectorManager * m_detManager;

  PublicToolHandle<IModuleDistortionsTool >  m_pixelDistoTool
     {this,"PixelDistortionsTool","PixelDistortionsTool",""};
  bool m_fullTest;

};

#endif // PixelDistortionsTestReadWrite_H
