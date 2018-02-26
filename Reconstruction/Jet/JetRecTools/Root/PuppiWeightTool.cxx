#include "JetRecTools/PuppiWeightTool.h"

#include "fastjet/PseudoJet.hh"
#include <vector>

#include "xAODCore/ShallowCopy.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODBase/IParticleHelpers.h"
#include "xAODCore/ShallowAuxContainer.h"

using namespace std;

//------------------------------------------------------------------------------

PuppiWeightTool::PuppiWeightTool(const std::string& name) : JetConstituentModifierBase(name) {

  declareProperty("R0", m_R0 = 0.3);
  declareProperty("Rmin", m_Rmin = 0.001);
  declareProperty("Beta", m_beta = 1);
  declareProperty("CentralPTCutOffset", m_centralPTCutOffset = 200);
  declareProperty("CentralPTCutSlope", m_centralPTCutSlope = 14);
  declareProperty("ForwardPTCutOffset", m_forwardPTCutOffset = 200);
  declareProperty("ForwardPTCutSlope", m_forwardPTCutSlope = 14);
  declareProperty("EtaBoundary",m_etaBoundary = 2.5);

  declareProperty("ApplyWeight",m_applyWeight=true);
  declareProperty("IncludeCentralNeutralsInAlpha",m_includeCentralNeutralsInAlpha=false);

  declareProperty("VertexContainerKey", m_vertexContainer_key);
  
  m_puppi = new Puppi(m_R0, m_Rmin, m_beta, m_centralPTCutOffset, m_centralPTCutSlope, m_forwardPTCutOffset, m_forwardPTCutSlope, m_etaBoundary);
}

//------------------------------------------------------------------------------

StatusCode PuppiWeightTool::initialize() {
  ATH_MSG_INFO("Initializing tool " << name() << "...");
  
  ATH_CHECK(m_vertexContainer_key.initialize());

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------

StatusCode PuppiWeightTool::process_impl(xAOD::IParticleContainer* cont) const {
  // Type-checking happens in the JetConstituentModifierBase class
  // so it is safe just to static_cast
  xAOD::PFOContainer* pfoCont = dynamic_cast<xAOD::PFOContainer*> (cont);
  if(pfoCont) return process_impl(pfoCont);
  else{
    ATH_MSG_ERROR("Unable to dynamic cast IParticleContainer to PFOContainer");
    return StatusCode::FAILURE;
  }
}

//------------------------------------------------------------------------------

StatusCode PuppiWeightTool::process_impl(xAOD::PFOContainer* cont) const{
  const static SG::AuxElement::Accessor<char> PVMatchedAcc("matchedToPV");
  const static SG::AuxElement::Accessor<double> alphaAcc("PUPPI_alpha");
  const static SG::AuxElement::Accessor<double> weightAcc("PUPPI_weight");

  std::vector<fastjet::PseudoJet> chargedHSVector;
  std::vector<fastjet::PseudoJet> chargedPUVector;
  std::vector<fastjet::PseudoJet> neutralVector;
  std::vector<fastjet::PseudoJet> forwardVector;

  // Fill input particle vectors for puppi
  for ( xAOD::PFO* ppfo : *cont ) {
    if (fabs(ppfo->charge()) > FLT_MIN && !PVMatchedAcc.isAvailable(*ppfo)){
      ATH_MSG_ERROR("Not known if PFO is matched to primary vertex.  Run ChargedHadronSubtractionTool before PuppiWeightTool");
      return StatusCode::FAILURE;
    }

    if (ppfo->pt()<=FLT_MIN) continue;

    fastjet::PseudoJet pj(ppfo->p4());
    //pj.set_user_info(new PuppiUserInfo({someOtherChi2,yetAnotherChi2}));  //example of how additional information could be exploited - needs to be calculated somewhere above

    float charge = ppfo->charge();
    bool isCharged = (fabs(charge) > FLT_MIN);

    if(fabs(ppfo->eta()) > m_etaBoundary) forwardVector.push_back(pj);
    else{     
      if(isCharged){
        bool matchedToPrimaryVertex=PVMatchedAcc(*ppfo);
	if(matchedToPrimaryVertex) chargedHSVector.push_back(pj);
	else chargedPUVector.push_back(pj);
      }
      else neutralVector.push_back(pj);
    }
  }

  //Count the number of primary vertices
<<<<<<< HEAD
  const xAOD::VertexContainer* pvtxs = nullptr;
  ATH_CHECK( evtStore()->retrieve(pvtxs,m_vertexContainer_key) );
  if(pvtxs->empty()){
=======
  const xAOD::VertexContainer* pvtxs = 0;
  ATH_CHECK(evtStore()->retrieve(pvtxs, "PrimaryVertices"));
  if ( pvtxs == 0 || pvtxs->size()==0 ) {
>>>>>>> Revert "switched from evtStore calls to DataHandles"
    ATH_MSG_WARNING(" This event has no primary vertices " );
    return StatusCode::FAILURE;
  }

  int nPV=0;
  for( auto vtx_itr : *pvtxs ){
    if((int)vtx_itr->nTrackParticles() < 2 ) continue;
    ++nPV;
  }

  m_puppi->setParticles(chargedHSVector, chargedPUVector, neutralVector, forwardVector, nPV);

  for ( xAOD::PFO* ppfo : *cont ) {
    float charge = ppfo->charge();
    bool isCharged = (fabs(charge) > FLT_MIN);
    bool isForward = (fabs(ppfo->eta()) > m_etaBoundary);

    fastjet::PseudoJet pj(ppfo->p4());

    double weight = m_puppi->getWeight(pj);
    double alpha = m_puppi->getAlpha(pj);

    if ((!isCharged || isForward) && m_applyWeight) ppfo->setP4(weight*ppfo->p4());
    alphaAcc(*ppfo) = alpha;
    weightAcc(*ppfo) = weight;
  }

  ATH_MSG_DEBUG("Median: "<<m_puppi->getMedian());
  ATH_MSG_DEBUG("RMS: "<<m_puppi->getRMS());

  return StatusCode::SUCCESS;
}
