/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////
/// Name    : TruthClusterizationFactory.cxx
/// Package : SiClusterizationTool 
/// Author  : Roland Jansky & Felix Cormier
/// Created : April 2016
///
/// DESCRIPTION: Emulates NN evaluation from truth (for ITK studies)
///////////////////////////////////////////////////////////////////////////////////////////////////////



#include "SiClusterizationTool/TruthClusterizationFactory.h"

//for position estimate and clustering
#include "InDetIdentifier/PixelID.h"
#include "InDetPrepRawData/PixelCluster.h"

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/Incident.h"
#include "InDetSimData/InDetSimData.h"
#include "HepMC/GenParticle.h"

#include "TrkEventPrimitives/ParamDefs.h"

#include "CLHEP/Random/RandFlat.h"

#include <TMath.h>

namespace InDet {

  TruthClusterizationFactory::TruthClusterizationFactory(const std::string& name,
                                                         const std::string& n, const IInterface* p):
    AthAlgTool(name, n, p)
  {
    declareInterface<TruthClusterizationFactory>(this);
  } 
  
  StatusCode TruthClusterizationFactory::initialize() {
  

    ATH_MSG_INFO("initialize() successful in " << name());

    // random svc
    CHECK(m_rndmSvc.retrieve());
  
    // get the random stream
    ATH_MSG_DEBUG ( "Getting random number engine : <" << m_rndmEngineName << ">" );
    m_rndmEngine = m_rndmSvc->GetEngine(m_rndmEngineName);
    if (!m_rndmEngine) {
      ATH_MSG_ERROR("Could not find RndmEngine : " << m_rndmEngineName);
      return StatusCode::FAILURE;
    }
    else {
      ATH_MSG_DEBUG("Found RndmEngine : " << m_rndmEngineName);
    }
 
    ATH_CHECK( m_simDataCollectionName.initialize() );


    return StatusCode::SUCCESS;
  }
  


  std::vector<double> TruthClusterizationFactory::estimateNumberOfParticles(const InDet::PixelCluster& pCluster) const
  {
    std::vector<double> probabilities(3,0.);
    auto rdos = pCluster.rdoList();
    bool crazycluster(true);
    unsigned int nPartContributing = 0;
    //Initialize vector for a list of UNIQUE barcodes for the cluster
    std::vector<int> barcodes;
    SG::ReadHandle<InDetSimDataCollection> pixSdoColl(m_simDataCollectionName);
    //Loop over all elements (pixels/strips) in the cluster
    if(pixSdoColl.isValid()){
      for (auto rdoIter :  rdos){
        auto simDataIter = pixSdoColl->find(rdoIter);
        if (simDataIter != pixSdoColl->end()){
          crazycluster = false;
          // get the SimData and count the individual contributions
          auto simData = (simDataIter->second);
          //auto simDataDeposits = simData.getdeposits();
          for( auto deposit : simData.getdeposits() ){
            //If deposit exists
            if (deposit.first){
              //Now iterate over all barcodes
              std::vector<int>::iterator barcodeIterator;
              //Look for the barcode of the specific particle depositing energy in the barcodes vector
              barcodeIterator  = find(barcodes.begin(), barcodes.end(), deposit.first->barcode());
              //If this barcode is not found
              if (!(barcodeIterator != barcodes.end())){
                //Add the barcode to the barcodes vector
                barcodes.push_back(deposit.first->barcode());
              }
            }
            else ATH_MSG_WARNING("No deposits found");
          }
          //nPartContributing = simDataDeposits.size() > nPartContributing ? simDataDeposits.size() : nPartContributing;
        }
      }
    }
    //Barcodes vector is then a list of the total number of UNIQUE
    //barcodes in the cluster, each corresponding to a truth particle
    nPartContributing = barcodes.size();
    ATH_MSG_VERBOSE("n Part Contributing: " << nPartContributing);
    ATH_MSG_VERBOSE("Smearing TruthClusterizationFactory probability output for TIDE studies");
    //If only 1 truth particles found
    if (nPartContributing==1) {
      //NN will always return 100% chance of there being only 1 particle
      probabilities[0] = 1.0;
    }
    //If two unique truth particles found in cluster
    else if (nPartContributing==2) {
      //90% chance NN returns high probability of there being 2 particles
      if (CLHEP::RandFlat::shoot( m_rndmEngine, 0, 1 ) < 0.9) probabilities[1] = 1.0;
      //Other 10% NN returns high probability of there being 1 particle
      else probabilities[0] = 1.0;
    }
    //If greater than 2 unique truth particles in cluster
    else if (nPartContributing>2) {
      //90% chance NN returns high probability of there being >2 particles
      if (CLHEP::RandFlat::shoot( m_rndmEngine, 0, 1 ) < 0.9) probabilities[2] = 1.0;
      //Other 10% NN returns high probability of there being 1 particle
      else probabilities[0] = 1.0;
    }
    
    //if truth collection not found
    if(crazycluster) {
      std::vector<double> noprobabilities;
      return noprobabilities;
    }
    //Else return probabilities calculated above
    
    return probabilities;
 
  }

  std::vector<Amg::Vector2D> TruthClusterizationFactory::estimatePositions(const InDet::PixelCluster& ) const
  { 
    ATH_MSG_ERROR("TruthClusterizationFactory::estimatePositions called for ITk ambiguity setup, should never happen! Digital clustering should be run for positions & errors.");
    return std::vector<Amg::Vector2D>(2,Amg::Vector2D (2,0.));
  }
  
}//end InDet namespace
