/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SCT_CablingCondAlgFromText_H
#define SCT_CablingCondAlgFromText_H
/**   
 *   @file SCT_CablingCondAlgFromText.h
 *
 *   @brief Fills an SCT cabling object from a plain text source
 *
 *   @author Susumu Oda
 *   @date 20/08/2018
 */

//STL includes
#include <string>

//Gaudi includes
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ICondSvc.h"

//Athena includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "SCT_Cabling/SCT_CablingData.h"
#include "StoreGate/WriteCondHandleKey.h"

/**
 *    @class SCT_CablingCondAlgFromText
 *    @brief Service which fill the SCT Cabling from plain text (a file).
 *
 */

class SCT_CablingCondAlgFromText: public AthAlgorithm {
 public:

  SCT_CablingCondAlgFromText(const std::string& name, ISvcLocator* svc);
  virtual ~SCT_CablingCondAlgFromText() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;
  
private:

  bool insert(const IdentifierHash& hash, const SCT_OnlineId& onlineId, const SCT_SerialNumber& sn, SCT_CablingData* data);
  std::string m_source;
  SG::WriteCondHandleKey<SCT_CablingData> m_writeKey{this, "WriteKey", "SCT_CablingData", "Key of output (derived) conditions folder"};
  ServiceHandle<ICondSvc> m_condSvc;

};//end of class

#endif // SCT_CablingCondAlgFromText_H
