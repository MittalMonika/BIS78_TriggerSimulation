/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CSCConditionsTestAlgMT_H
#define CSCConditionsTestAlgMT_H 

//STL
#include <string>
#include <sstream>

//Gaudi
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/StatusCode.h"

//Athena
#include "Identifier/IdentifierHash.h"
#include "MuonCondData/CscCondDbData.h"
#include "MuonCondSvc/MuonHierarchy.h"


//Forward declarations
class ISvcLocator;
class StatusCode;
class CscCondDbData;



///Example class to show calling the SCT_ConditionsSummarySvc
class CSCConditionsTestAlgMT : public AthAlgorithm {

 public:
   CSCConditionsTestAlgMT(const std::string &name,ISvcLocator *pSvcLocator) ;
   virtual ~CSCConditionsTestAlgMT();

   virtual StatusCode initialize();
   virtual StatusCode execute();
   virtual StatusCode finalize();
   
 private:
   SG::ReadCondHandleKey<CscCondDbData> m_readKey{this, "ReadKey", "CscCondDbData", "Key of CscCondDbData"};

}; //end of class

#endif
