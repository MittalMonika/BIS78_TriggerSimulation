/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGL2MUONSA_ALIGNMENTBARRELLUTSVC_H
#define TRIGL2MUONSA_ALIGNMENTBARRELLUTSVC_H

#include <string>
#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/StatusCode.h"

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include "TrigL2MuonSA/AlignmentBarrelLUT.h"

namespace TrigL2MuonSA {
  
  class AlignmentBarrelLUTSvc : public AthService, virtual public IInterface
  {
  public:
    static const InterfaceID& interfaceID() { 
      static const InterfaceID IID(11498, 0 , 0);
      return IID;
    }

  public:
    AlignmentBarrelLUTSvc(const std::string& name,ISvcLocator* sl);
    virtual ~AlignmentBarrelLUTSvc() {}
    
    virtual StatusCode queryInterface(const InterfaceID& riid,void** ppvIF);
    
    virtual StatusCode initialize(void);
    virtual StatusCode finalize(void);

  private:
    Gaudi::Property< std::string > m_lut_fileName {
	this, "LUTfile", "dZ_barrel.lut", ""};

    ToolHandle<AlignmentBarrelLUT>   m_alignmentBarrelLUT;
    
  public:
    const ToolHandle<AlignmentBarrelLUT>* alignmentBarrelLUT(void) const
    { return &m_alignmentBarrelLUT; };
  };

}
  
#endif 
