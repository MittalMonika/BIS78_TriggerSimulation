/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGL2MUONSA_PTBARRELLUTSVC_H
#define TRIGL2MUONSA_PTBARRELLUTSVC_H

#include <string>

#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/StatusCode.h"

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include "TrigL2MuonSA/PtBarrelLUT.h"

namespace TrigL2MuonSA {
  
  class PtBarrelLUTSvc : public AthService, virtual public IInterface
  {
  public:
    static const InterfaceID& interfaceID() { 
      static const InterfaceID IID(11496, 0 , 0);
      return IID;
    }

  public:
    PtBarrelLUTSvc(const std::string& name,ISvcLocator* sl);
    virtual ~PtBarrelLUTSvc() {}
    
    virtual StatusCode queryInterface(const InterfaceID& riid,void** ppvIF);
    
    virtual StatusCode initialize(void);
    virtual StatusCode finalize(void);

  private:
    Gaudi::Property< std::string > m_lut_fileName {
	this, "LUTfile", "pt_barrel.lut", ""};
    Gaudi::Property< std::string > m_lutSP_fileName {
	this, "SP_LUTfile", "pt_barrelSP_new.lut", ""};

    ToolHandle<PtBarrelLUT>  m_ptBarrelLUT;

  public:
    const ToolHandle<PtBarrelLUT>* ptBarrelLUT(void) const
    {return &m_ptBarrelLUT;};
    
  };

}
  
#endif 
