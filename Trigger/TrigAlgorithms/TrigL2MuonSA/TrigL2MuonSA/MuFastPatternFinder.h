/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  TRIGL2MUONSA_MUFASTPATTERNFINDER_H
#define  TRIGL2MUONSA_MUFASTPATTERNFINDER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GeoPrimitives/GeoPrimitives.h"

#include "MdtCalibSvc/MdtCalibrationTool.h"

#include "TrigL2MuonSA/MuonRoad.h"
#include "TrigL2MuonSA/MdtData.h"
#include "TrigL2MuonSA/TrackData.h"

#include "MuonIdHelpers/MuonIdHelperTool.h"


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

namespace TrigL2MuonSA {

struct MdtLayerHits
{
  unsigned int ntot;
  unsigned int ntot_all;
  unsigned int ndigi;
  unsigned int ndigi_all;
  double ResSum;
  std::vector<unsigned int> indexes;
};

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

class MuFastPatternFinder: public AthAlgTool
{
   public:
      
      static const InterfaceID& interfaceID();

   public:

      MuFastPatternFinder(const std::string& type, 
			   const std::string& name,
			   const IInterface*  parent);
    
      ~MuFastPatternFinder();
    
      virtual StatusCode initialize();
      virtual StatusCode finalize  ();
    
   private:

      double calc_residual(double aw,double bw,double x,double y);
      void  doMdtCalibration(TrigL2MuonSA::MdtHitData& mdtHit, double track_phi, double phi0, bool isEndcap);

   public:

      StatusCode findPatterns(const TrigL2MuonSA::MuonRoad& muonRoad,
			      TrigL2MuonSA::MdtHits&        mdtHits,
			      std::vector<TrigL2MuonSA::TrackPattern>& v_trackPatterns);

      void setGeometry(bool use_new_geometry);

   private:
      // MDT calibration service
      ToolHandle<MdtCalibrationTool> m_mdtCalibrationTool;

      // Id helper
      ToolHandle<Muon::MuonIdHelperTool> m_muonIdHelperTool{this, "idHelper", 
         "Muon::MuonIdHelperTool/MuonIdHelperTool", "Handle to the MuonIdHelperTool"};

};

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

} // namespace TrigL2MuonSA

#endif  // MUFASTPATTERNFINDER_H
