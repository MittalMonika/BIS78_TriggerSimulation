/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// DAF_WeightCalculator.cxx
//   Source file for class DAF_WeightCalculator
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Sebastian.Fleischmann@cern.ch
///////////////////////////////////////////////////////////////////


// Trk
#include "TrkDeterministicAnnealingFilter/DAF_WeightCalculator.h"
//#include "GaudiKernel/MsgStream.h"

#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkSurfaces/Surface.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"

// standard libs
//#include <math.h>
#include <cmath>
#include <numeric> //for accumulate

// constructor
Trk::DAF_WeightCalculator::DAF_WeightCalculator(
    const std::string& t,
    const std::string& n,
    const IInterface*  p )
        :
AthAlgTool(t,n,p) {
    declareInterface<IWeightCalculator>(this);
}

// destructor
Trk::DAF_WeightCalculator::~DAF_WeightCalculator() {}



// initialize
StatusCode Trk::DAF_WeightCalculator::initialize() {
    StatusCode s = AthAlgTool::initialize();
    if (s.isFailure()) return s;

    ATH_MSG_INFO( "initialize() successful in " << name() );
    return StatusCode::SUCCESS;
}

// finalize
StatusCode Trk::DAF_WeightCalculator::finalize() {

    ATH_MSG_INFO( "finalize() successful in " << name() );
    return StatusCode::SUCCESS;
}


//calculate the assignment probabilities (assignment weights) for a collection of measurements
// const std::vector< Trk::CompetingRIOsOnTrack::AssignmentProb >* Trk::DAF_WeightCalculator::calculateWeights (
//     const Trk::TrackParameters& trkPar,
//     const std::vector< const Trk::RIO_OnTrack* >* ROTs,
//     const double cutValue,
//     const bool doNormalization=true ) const {
//
//     ATH_MSG_DEBUG("calculate weight for a collection of measurements");
//     // -----------------------------
//     // produce local TrackParameters
//     std::vector< const Trk::RIO_OnTrack*>::const_iterator rotIter = ROTs->begin();
//     for (; rotIter!=ROTs->end(); ++rotIter) {
//     }
// }

//calculate the assignment probabilities (assignment weights) for a collection of measurements
const std::vector< Trk::CompetingRIOsOnTrack::AssignmentProb >* Trk::DAF_WeightCalculator::calculateWeights (
    const std::vector< const Trk::TrackParameters* >* trkPars,
    const std::vector< const Trk::RIO_OnTrack* >* ROTs,
    const AnnealingFactor beta,
    const double cutValue,
    const bool doNormalization ) const {

    ATH_MSG_DEBUG("--->  calculate weight for a collection of measurements");
    std::vector< Trk::CompetingRIOsOnTrack::AssignmentProb >* assgnProbVec = new std::vector< Trk::CompetingRIOsOnTrack::AssignmentProb >;

    //check if vectors have the same lenght
    if ( ROTs->size() != trkPars->size() ) {
        ATH_MSG_ERROR("vector of RIO_OnTrack and TrackParameters do not have the same size: assignmentProbabilities cannot be calculated");
        delete assgnProbVec;
        return nullptr;
    } else {
        // -----------------------------
        // loop over ROTs to calc non-normalized assignment probbailities
        ATH_MSG_VERBOSE("loop over ROTs");
        for (unsigned int i=0; i < ROTs->size(); i++) {
            Trk::CompetingRIOsOnTrack::AssignmentProb prob = calculateWeight( *(trkPars->operator[](i)), *(ROTs->operator[](i)), beta );
            ATH_MSG_VERBOSE("assignment probability for ROT "<< i << ": " << prob );
            assgnProbVec->push_back( prob );
        }
        if (doNormalization) {
            // normalize assignment probabilities
            ATH_MSG_VERBOSE("call normalize()");
            normalize( *assgnProbVec, ROTs, beta, cutValue);
        }
        return assgnProbVec;
    }
}

//calculate the assignment probability (assignment weight) for a single measurement
//(no normalization can be done in this case)
Trk::CompetingRIOsOnTrack::AssignmentProb Trk::DAF_WeightCalculator::calculateWeight (
    const Trk::TrackParameters& trkPar,
    const Trk::RIO_OnTrack& ROT,
    const AnnealingFactor beta) const {
    static constexpr double M_PI_MULT_2 = M_PI*2.0;
    ATH_MSG_DEBUG("---> calculate weight for a single measurement");

    //check whether TrackParameters are given on the right surface
    if ( (ROT.associatedSurface()) !=  (trkPar.associatedSurface()) ) {
        ATH_MSG_ERROR("RIO_OnTrack and TrackParameters do not have the same associatedSurface: assignmentProbability cannot be calculated");
        return 0;
    } else {
        ATH_MSG_VERBOSE("start calculation:");
        // code can be written in one line; avoiding this 'cause of readability

        // -----------------------------------------
        // calculate covariance (of the measurement) dependend factor
        // dimension of the measurement vector
        const int ROTdim = ROT.localParameters().dimension();
        ATH_MSG_VERBOSE("dimension of the measurements: " << ROTdim );
        //sroe: see comment at line 199
        double ROTdimFactor =   std::pow(M_PI_MULT_2, (ROTdim*0.5)) ;     //  (2 \pi)^(n/2)
        // 1 / ( (2 \pi)^(n/2) * \sqrt(\beta \det(V_i)) )
        double factor =  1./( ROTdimFactor * std::sqrt( beta * ROT.localCovariance().inverse().eval().determinant() ) );
        ATH_MSG_VERBOSE("scale factor of prob: " << factor );
        // -----------------------------------------
        // calculate exponential in the multivariate Gaussian:
        // State to measurement dimensional reduction Matrix ( n x m )
        Amg::MatrixX H(ROT.localParameters().expansionMatrix());
        ATH_MSG_VERBOSE("size of reduction matrix: " << H.cols() << "x" << H.rows() );
        ATH_MSG_VERBOSE("dimension of TrackParameters: " << trkPar.parameters().rows() );
        // residual of the measurement r = (m_i - Hx)
	      Amg::VectorX r = ROT.localParameters() - H * trkPar.parameters();
        // (m_i - Hx)^T * V^{-1} * (m_i - Hx) = (m_i - Hx)^T * G * (m_i - Hx)
        Amg::MatrixX weight  = ROT.localCovariance().inverse();
        double exponential =  r.dot(weight*r)/(2.*beta);
        if (msgLvl(MSG::VERBOSE)){
            trkPar.dump(msg(MSG::VERBOSE));
            msg(MSG::VERBOSE)<<"local parameters of ROT: " << ROT.localParameters()<<endmsg;
            msg(MSG::VERBOSE)<<"locX of parameters: ROT: " << ROT.localParameters()[Trk::locX] << " track: " << trkPar.parameters()[Trk::locX]<< " H*track: " << (H * trkPar.parameters())[Trk::locX] <<endmsg;
            msg(MSG::VERBOSE)<<"norm of residual: " << r.norm() <<endmsg;
            msg(MSG::VERBOSE)<<"ROT weight(locX): " << weight(Trk::locX,Trk::locX) <<endmsg;
            msg(MSG::VERBOSE)<<"exponent of prob: " << exponential <<endmsg;
        }
        return ( factor * std::exp(-exponential) );
    } // end if (equal surfaces)
}

//normalize given assignment probabilities (assignment weights) using a given cutValue and annealing factor
const std::vector< Trk::CompetingRIOsOnTrack::AssignmentProb >* Trk::DAF_WeightCalculator::normalize (
    const std::vector< Trk::CompetingRIOsOnTrack::AssignmentProb >* assgnProbs,
    const std::vector< const Trk::RIO_OnTrack* >* ROTs,
    const AnnealingFactor beta,
    const double cutValue ) const {

    // copy given assgnProbs to new vector
    ATH_MSG_DEBUG("copy vector<AssignmentProb> to a new one");
    std::vector< Trk::CompetingRIOsOnTrack::AssignmentProb >* newAssgnProbs  = new std::vector< Trk::CompetingRIOsOnTrack::AssignmentProb >;
    // TODO: check whether this is right, or if a loop is needed:
    *newAssgnProbs = *assgnProbs;
    ATH_MSG_DEBUG("call other normalize()");
    Trk::DAF_WeightCalculator::normalize( *newAssgnProbs, ROTs, beta, cutValue );
    return newAssgnProbs;
}



//normalize given assignment probabilities (assignment weights) using a given cutValue and annealing factor
void Trk::DAF_WeightCalculator::normalize (
    std::vector< Trk::CompetingRIOsOnTrack::AssignmentProb >& assgnProbs,
    const std::vector< const Trk::RIO_OnTrack* >* ROTs,
    const AnnealingFactor beta,
    const double cutValue ) const {

    if (assgnProbs.size() != ROTs->size()) {
        ATH_MSG_ERROR("sizes of AssignmentProb vector and RIO_OnTrack vector do not match: no normalization done");
        return;
    } else {
        ATH_MSG_DEBUG("starting normalization:");
        // ----------------------------
        // calculate sum of assgnProbs:
        
        //std::vector< Trk::CompetingRIOsOnTrack::AssignmentProb >::const_iterator probIter = assgnProbs.begin();
        //use accumulate; 0.0 is a double literal by default (this is important)
        const double assgnProbSum = std::accumulate(assgnProbs.begin(), assgnProbs.end(),0.0);  
        /**
        for (; probIter!=assgnProbs.end(); ++probIter) {
            assgnProbSum += (*probIter);
        }
        **/
        if (assgnProbSum > 0.) {
            // -----------------------------------------
            // calculate sum of ROT specific cut values:
            // dimension of the measurement vectors (assume that all ROTs have the same dimension!!!)
            //int ROTdim = (*(ROTs->begin()))->localParameters().dimension();
            const int ROTdim = ROTs->front()->localParameters().dimension();
           /**
            * sroe 30/06/2016
            * Coverity 105661 Result is not floating-point; the original code returned an
            * integer exponent of 2*pi, but the original reference does not indicate this
            * is correct, and in fact a floating point exponent was intended.
            * ref.:http://cds.cern.ch/record/1014533/files/thesis-2007-011.pdf, p. 26
            **/
            const double ROTdimFactor =  std::pow(2.0*M_PI, (ROTdim*0.5)) ;     //  (2 \pi)^(n/2)
            const double cutFactor = std::exp(-cutValue/(beta*2.));
            ATH_MSG_VERBOSE("   cut factor: " << cutFactor );
            double cutProbSum = 0;
            //std::vector< const Trk::RIO_OnTrack*>::const_iterator rotIter = ROTs->begin();
            for (const auto & thisROT: *ROTs) {
                // exp(-1/2 * c/\beta) / ( (2 \pi)^(n/2) * \sqrt(\det(V_i)) )
                cutProbSum +=  cutFactor/( ROTdimFactor * std::sqrt( beta * thisROT->localCovariance().determinant() ) );
            }
            ATH_MSG_VERBOSE("   sum of non-normalized assgn-probs: " << assgnProbSum );
            ATH_MSG_VERBOSE("   sum of values for probabilty cuts: " << cutProbSum );
            //----------------------------------------
            // calculate new assignment probabilities:
            const double factor = 1./(assgnProbSum + cutProbSum);
            /**     }
            for (unsigned int i=0; i < assgnProbs.size(); i++) {
                assgnProbs[i] *= factor;
            }
            **/
            for (double &p : assgnProbs){ p *= factor; }
        } // end if (assgnProbSum > 0.)
    } // end if(vector sizes match)
}


