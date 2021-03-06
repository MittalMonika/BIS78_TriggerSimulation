// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: TrigMissingETCnvAlg.h 592539 2014-04-11 10:17:29Z krasznaa $
#ifndef xAODTrigMissingETCNV_TrigMissingETCNVALG_H
#define xAODTrigMissingETCNV_TrigMissingETCNVALG_H

// System include(s):
#include <string>

// Gaudi/Athena include(s):
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

// Local include(s):
#include "xAODTrigMissingETCnv/ITrigMissingETCnvTool.h"

namespace xAODMaker {

   /**
    *  @short Algorithm for creating xAOD::TrigMissingETContainer from an AOD
    *
    *         This algorithm can be used to create an xAOD muon RoI container
    *         out of the objects found in an AOD file.
    *
    */
   class TrigMissingETCnvAlg : public AthAlgorithm {

   public:
      /// Regular Algorithm constructor
      TrigMissingETCnvAlg( const std::string& name, ISvcLocator* svcLoc );

      /// Function initialising the algorithm
      virtual StatusCode initialize();
      /// Function executing the algorithm
      virtual StatusCode execute();

   private:
      /// StoreGate key of the input container
      std::string m_aodKey;
      /// StoreGate key for the output container
      std::string m_xaodKey;

      /// Handle to the converter tool
      ToolHandle< ITrigMissingETCnvTool > m_cnvTool;

   }; // class TrigMissingETCnvAlg

} // namespace xAODMaker

#endif // xAODTrigMissingETCNV_TrigMissingETCNVALG_H
