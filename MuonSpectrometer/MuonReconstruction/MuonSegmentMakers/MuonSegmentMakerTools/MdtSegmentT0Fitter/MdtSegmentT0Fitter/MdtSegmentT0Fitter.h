/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// Fitting for in situ calibration of segments
#ifndef MUON_MDTSEGMENTT0FITTER_H
#define MUON_MDTSEGMENTT0FITTER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

#include "TrkDriftCircleMath/DCSLFitter.h"
#include "MuonSegmentMakerInterfaces/IDCSLFitProvider.h"

#include "MdtCalibSvc/MdtCalibrationDbTool.h"

#include <atomic>
#include <vector>

class IIdToFixedIdTool;
class MdtCalibrationDbTool;
class TMinuit;

namespace TrkDriftCircleMath {

  class MdtSegmentT0Fitter : public AthAlgTool, public DCSLFitter, virtual public Muon::IDCSLFitProvider {
    public:

      MdtSegmentT0Fitter(const std::string&,const std::string&,const IInterface*);
      virtual ~MdtSegmentT0Fitter ();
      virtual StatusCode initialize() override;
      virtual StatusCode finalize  () override;
      
      virtual bool fit( const Line& line, const DCOnTrackVec& dcs, double t0Seed ) const override;
      virtual bool fit( const Line& line, const DCOnTrackVec& dcs, const HitSelection& selection, double t0Seed ) const override;

 
      virtual const DCSLFitter* getFitter() const override { return this; }

    private:
		  bool m_trace; // debug - traces operation
      bool m_dumpToFile; // debug - dumps some performance info
		  bool m_dumpNoFit; // debug - print hit info where fit doesn't run		

      bool m_useInternalRT; // whether to use an internal RT function or the one from Calibration Service
      ToolHandle<MdtCalibrationDbTool> m_calibrationDbTool;

      int m_minHits; // minimum number of selected hits for t0 fit. Otherwise use default

      bool m_constrainShifts; // whether to constrain t0 shifts to a 50 ns window
      double m_constrainT0Error; // t0 error that is used in the constraint
      bool   m_rejectWeakTopologies; // Reject topolgies that do not have at least one +- combination in one Multilayer
      bool m_scaleErrors; //!< rescale errors in fit

      bool m_propagateErrors; //!< propagate errors

      TMinuit* m_minuit;

      // counters
      mutable std::atomic_uint m_ntotalCalls;
      mutable std::atomic_uint m_npassedNHits;
      mutable std::atomic_uint m_npassedSelectionConsistency;
      mutable std::atomic_uint m_npassedNSelectedHits;
      mutable std::atomic_uint m_npassedMinHits;
      mutable std::atomic_uint m_npassedMinuitFit;
    };
    
  inline bool MdtSegmentT0Fitter::fit( const Line& line, const DCOnTrackVec& dcs, double t0Seed ) const { 
    HitSelection selection(dcs.size(),0);
    return fit( line, dcs, selection, t0Seed ); 
  }
}



#endif
