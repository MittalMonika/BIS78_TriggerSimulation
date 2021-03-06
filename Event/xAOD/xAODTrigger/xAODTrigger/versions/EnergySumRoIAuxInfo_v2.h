// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: EnergySumRoIAuxInfo_v2.h 631149 2014-11-26 12:26:18Z krasznaa $
#ifndef XAODTRIGGER_VERSIONS_ENERGYSUMROIAUXINFO_V2_H
#define XAODTRIGGER_VERSIONS_ENERGYSUMROIAUXINFO_V2_H

// System include(s):
extern "C" {
#   include <stdint.h>
}
#include <vector>
#include <string>

// EDM include(s):
#include "xAODCore/AuxInfoBase.h"

namespace xAOD {

   /// Auxiliary store for an EnergySum RoI object
   ///
   /// This auxiliary store describes a single EnergySum RoI object, as we
   /// only have one of those in the ATLAS events.
   ///
   /// @author Lukas Heinrich <Lukas.Heinrich@cern.ch>
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   /// $Revision: 631149 $
   /// $Date: 2014-11-26 13:26:18 +0100 (Wed, 26 Nov 2014) $
   ///
   class EnergySumRoIAuxInfo_v2 : public AuxInfoBase {

   public:
      /// Default constuctor
      EnergySumRoIAuxInfo_v2();

   private:
      uint32_t roiWord0;
      uint32_t roiWord1;
      uint32_t roiWord2;
      uint32_t roiWord3;
      uint32_t roiWord4;
      uint32_t roiWord5;

      std::vector< std::string > thrNames;

      float energyX;
      float energyY;
      float energyT;
      float energyXRestricted;
      float energyYRestricted;
      float energyTRestricted;

   }; // class EnergySumRoIAuxInfo_v2

} // namespace xAOD

// Declare the inheritance of the type:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::EnergySumRoIAuxInfo_v2, xAOD::AuxInfoBase );

#endif // XAODTRIGGER_VERSIONS_EnergySumRoIAuxInfo_v2_H
