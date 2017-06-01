
// Dear emacs, this is -*- c++ -*-
// $Id: ITSmeToolTester.h 299890 2014-03-29 08:54:38$
#pragma once
// Gaudi/Athena include(s):
#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "AsgTools/ToolHandle.h"
// Local include(s):
#include "InDetTrackSystematicsTools/IInDetTrackSmearingTool.h"
#include <TH1.h>


namespace InDet {

   /// Simple algorithm for using the tools in Athena
   ///
   /// This is meant to be the "Athena equivalent" of the code put into
   /// util/InDetTrackSmearingTool.cxx.
   ///
   /// @author 
   ///
   /// $Revision: $
   /// $Date: $
   ///
   class InDetTrackSmearingToolTester : public AthHistogramAlgorithm {

   public:
      /// Regular Algorithm constructor
      InDetTrackSmearingToolTester( const std::string& name, ISvcLocator* svcLoc );
      /// Function initialising the algorithm
      virtual StatusCode initialize();
      /// Function executing the algorithm
      virtual StatusCode execute();

   private:
      /// StoreGate key for the track container to investigate//--->delete in future
      std::string Track_IP;
            
      /// Connection to the smearing tool
      ToolHandle< IInDetTrackSmearingTool > m_smearTool;

     std::vector< std::string > m_systematicsNames;
     CP::SystematicSet m_systActive;

   }; // class ITSmeToolTester

} // namespace InDet

