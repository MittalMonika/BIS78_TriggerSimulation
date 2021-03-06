/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// KalmanOutlierRecovery_InDet.h
//   Header file for Kalman Outlier Recovery in the Inner Detector
//////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRK_KALMANOUTLIERRECOVERY_INDET_H
#define TRK_KALMANOUTLIERRECOVERY_INDET_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkFitterInterfaces/IKalmanOutlierLogic.h"
#include "TrkFitterUtils/ProtoTrackStateOnSurface.h"

namespace Trk {
	
    class ProtoTrajectoryUtility; // helper to analyse current trajectory
    class IResidualPullCalculator;// gets pulls
    class FitQuality;             // track class FitQuality
    typedef std::vector<Trk::ProtoTrackStateOnSurface> Trajectory;

	/** @brief AlgTool to perform an optional outlier detection
        and possible removal by refitting on the reduced set of hits.

        @author M. Elsing, W. Liebig
	 */
	class KalmanOutlierRecovery_InDet : virtual public IKalmanOutlierLogic, public AthAlgTool { 
public:	
		//! standard AlgTool constructor
		KalmanOutlierRecovery_InDet(const std::string&,const std::string&,const IInterface*);
		virtual ~KalmanOutlierRecovery_InDet();
			
		//! standard Athena initialisation
		StatusCode initialize();
        //! standard Athena termination
		StatusCode finalize();
		
        /** give it some tools - temporary */
        virtual StatusCode configureWithTools(IExtrapolator*, const IUpdator*,
                                              const IMeasurementRecalibrator* );

        /** @brief applies logics to find outliers, if any it flags them
            on the trajectory and
            returns if outliers have been found and a refit is needed. */
        virtual bool flagNewOutliers(Trajectory&, const FitQuality&,
                                           int&, const int) const; 

		/** @brief determines if a track is to be rejected because of a
            bad fit, such that the outlier logic should be run. */
        virtual bool reject(const FitQuality&) const;
		
private:
        //! cut above which chi2/ndf to reject track - turned into probability
		double m_Trajectory_Chi2PerNdfCut;
        //! cut above which chi2/ndt to flag a single state as outlier
		double m_State_Chi2PerNdfCut;
        //! position tolerance in sigmas when a track should be defined as being outside detector bounds
                //double m_surfaceProximityCut;
        //! trajectory chi2/ndf cut turned into a probability
        double m_Trajectory_Chi2ProbCut;

        IExtrapolator*                    m_extrapolator;
        const IUpdator*                   m_updator;
        const IMeasurementRecalibrator*   m_recalibrator;
        const ProtoTrajectoryUtility*     m_utility;

	};
	
} // end of namespace

#endif // TRK_KALMANOUTLIERRECOVERY_INDET_H
