/*
 *   Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1CaloFexSim/JetAlg.h"
#include "TrigT1CaloFexSim/JGTower.h"
#include "TrigT1CaloFexSim/Objects.h"
// attempting to get ATH_MSG to work, but no joy
// #include "AthenaBaseComps/AthAlgorithm.h"
// #include "AthAnalysisBaseComps/AthAnalysisAlgorithm.h"

//function to build up the location of all seeds
StatusCode JetAlg::SeedGrid(const xAOD::JGTowerContainer*towers, JetAlg::Seed*seeds, bool &m_dumpSeedsEtaPhi){

  std::vector<float> seed_candi_eta;

  std::vector<std::pair<int,int>> seeds_candi;
  unsigned t_size = towers->size();
  float t_maxi=-999;

  for(unsigned i=0; i<t_size;i++){
     const xAOD::JGTower*tower = towers->at(i);
     float t_eta = tower->eta();
     if(t_eta>t_maxi) t_maxi=t_eta;
  }

  //arrange seeds in the order from eta=-4.9 to eta=4.9
  for(float t_eta=t_maxi-0.01; t_eta<t_maxi;){

     float tmp=999;

     for(unsigned i=0;i<t_size;i++){
        const xAOD::JGTower*tower = towers->at(i);
        std::vector<int> SC_indices = tower->SCIndex();
        if(SC_indices.size()==0) continue;

        float candi_eta = tower->eta()-tower->deta()/2;
        if(tower->eta()<0) candi_eta = tower->eta()+tower->deta()/2;
        if(fabs(tower->eta())>3.2) candi_eta = tower->eta();

        if( candi_eta<tmp && seed_candi_eta.size()==0) { tmp=candi_eta; t_eta = candi_eta;}
        else if( candi_eta > t_eta && candi_eta<tmp )  {  tmp = candi_eta; }

     }
     t_eta = tmp;
     seed_candi_eta.push_back(t_eta);
  }

  //arrange seeds in the order from phi=-pi to phi=pi
  for(unsigned i=0; i<seed_candi_eta.size(); i++){

     (seeds->eta).push_back(seed_candi_eta.at(i));
     std::vector<float> tmp_phi;
     for(float t_phi=-TMath::Pi(); ;){

        float dphi=0;

        for(unsigned t=0;t<t_size;){
           const xAOD::JGTower*tower = towers->at(t);
           t++;
           std::vector<int> SC_indices = tower->SCIndex();
           if(SC_indices.size()==0) continue;
           float eta = tower->eta()-tower->deta()/2;
           if(tower->eta()<0) eta = tower->eta()+tower->deta()/2;
           if(fabs(tower->eta())>3.2)  eta = tower->eta();
           if(eta!=seed_candi_eta.at(i)) continue;
           dphi = tower->dphi();
           break;
        }
       tmp_phi.push_back(t_phi);
       t_phi+=dphi;

       if(t_phi>TMath::Pi()) break;
     }
     (seeds->phi).push_back(tmp_phi);
  }

  if(m_dumpSeedsEtaPhi) {
    std::cout << "seed eta phi" << std::endl;
    std::cout << "i_eta" << "\t" << "i_phi" << "\t" << "eta" << "\t" << "phi" << std::endl;
    for(unsigned i=0; i<seeds->eta.size(); i++){
      for(unsigned ii=0; ii<seeds->phi.at(i).size(); ii++){
        float eta=seeds->eta.at(i);
        float phi=seeds->phi.at(i).at(ii);
        std::cout << i << "\t" << ii << "\t" << eta << "\t" << phi << std::endl;
      }
    }
    m_dumpSeedsEtaPhi = false; // only want to do this once
  }

  return StatusCode::SUCCESS;
}

//To find the seeds as local maxima
StatusCode JetAlg::SeedFinding(const xAOD::JGTowerContainer*towers, JetAlg::Seed*seeds,  float seed_size,float range, std::vector<float> noise, bool m_debug){
  // get the energy of each seeds which is defined as 2x2 towers in barrel and endcap, and single tower in fcal
  // static MsgStream staticMsg("MyStaticMsgStream"0);
  // static MsgStream staticMsg();
  // staticMsg << "My message" << endmsg
  // msg() << MSG::INFO << "Retrieved tool " << endmsg;
  // ATH_MSG_DEBUG("How can I get ATH_MSG_DEBUG to compile???");
  if(m_debug)
    std::cout << "JetAlg::SeedFinding: getting seeds with " << seed_size << " and " << range << std::endl;
  for(unsigned i=0; i<seeds->eta.size(); i++){
    std::vector<float> tmp_et;
    tmp_et.clear();
    for(unsigned ii=0; ii<seeds->phi.at(i).size(); ii++){
      float et=0;
      float thr=0;
      float eta=seeds->eta.at(i);
      float phi=seeds->phi.at(i).at(ii);
      for(unsigned t=0; t<towers->size(); t++){
        const xAOD::JGTower*tower = towers->at(t);
        if(!inBox(tower->eta(),eta,seed_size/2,tower->phi(),phi,seed_size/2)) continue;
        if(noise.size() < t) {
          std::cout << "JetAlg::SeedFinding FATAL: the noise vector is smaller (at " << noise.size() << " entries) than the tower number " << t << " that you are attempting to use" << std::endl;
          return StatusCode::FAILURE;
        }
        if( tower->et() > 5*noise.at(t) ) {
          if(m_debug)
            std::cout << "found a tower with high et = " << tower->et() << " at " << tower->eta() << ", " << tower->phi() << std::endl;
          et+= tower->et();
        }
        thr += noise.at(t);
      }
      if(et<thr*6) et = 0;
      tmp_et.push_back(et);
      if (et > 0 && m_debug)
        std::cout << eta << ", " << phi << " - pushed back tmp_et " << i << " with et = " << et << " after thr = " << thr << std::endl;
    }
    seeds->et.push_back(tmp_et);
  }
  
  // determine whether the seed is with locally maximal energy
  for(unsigned iseed_eta=0; iseed_eta<seeds->eta.size(); iseed_eta++){
    std::vector<bool> tmp_max;
    for(unsigned iseed_phi=0; iseed_phi<seeds->phi.at(iseed_eta).size(); iseed_phi++){
      float et = seeds->et.at(iseed_eta).at(iseed_phi);
      // only seeds with Et>5GeV is available
      if(et<5000) {
        tmp_max.push_back(0);
        continue;
      }
      // eta_n: et higher than all seeds with smaller eta
      // eta_p: et higher than all seeds with larger eta
      // eta_0: et higher than the other seeds along the same eta ring

      bool eta_n=1, eta_p=1, eta_0=1;

      for(unsigned i=iseed_eta+1; ;i++){
        if(i>=seeds->eta.size()) break;
        if(fabs(seeds->eta.at(i)-seeds->eta.at(iseed_eta))>range) break;
        for(unsigned ii=0; ii<seeds->phi.at(i).size(); ii++){

          float dphi = deltaPhi(seeds->phi.at(iseed_eta).at(iseed_phi),seeds->phi.at(i).at(ii));
          if(dphi>range) continue;
          if(seeds->et.at(iseed_eta).at(iseed_phi)<=seeds->et.at(i).at(ii)){
            eta_p = false;
            break;
          }
        }
      }

      for(int i=iseed_eta-1; ;i--){
        if(i<0) break;
        if(fabs(seeds->eta.at(iseed_eta)-seeds->eta.at(i))>range) break;
        for(unsigned ii=0; ii<seeds->phi.at(i).size(); ii++){
          float dphi = deltaPhi(seeds->phi.at(iseed_eta).at(iseed_phi),seeds->phi.at(i).at(ii));
          if(dphi>range) continue;
          if(seeds->et.at(iseed_eta).at(iseed_phi)<=seeds->et.at(i).at(ii)){
            eta_n = false;
            break;
          }
        }
      }

      for(unsigned ii=0; ii<seeds->phi.at(iseed_eta).size(); ii++){
        if(ii==iseed_phi) continue;
        float dphi = deltaPhi(seeds->phi.at(iseed_eta).at(iseed_phi),seeds->phi.at(iseed_eta).at(ii));
        if(dphi>range) continue;
        if(seeds->et.at(iseed_eta).at(iseed_phi)<=seeds->et.at(iseed_eta).at(ii)){
          eta_0 = false;
          break;
        }
      }
      tmp_max.push_back(eta_n&&eta_p&&eta_0);
    }
    seeds->local_max.push_back(tmp_max);
  }
  return StatusCode::SUCCESS;
}

StatusCode JetAlg::BuildFatJet(const xAOD::JGTowerContainer towers, std::vector<JetAlg::L1Jet>& js, float jet_r, std::vector<float> noise){

  std::vector<TowerObject::Block> blocks;

  TowerObject::TowerGrid grid = TowerObject::TowerGrid(towers);

  for(const xAOD::JGTower* seed: towers){
    int seedIndex = std::find(towers.begin(), towers.end(), seed) - towers.begin();

    std::vector<int> neighbors = grid.neighbors(*seed, 3, 3);
    float seed_Et = seed->et();

    double block_area(0.0);
    double block_pt(seed_Et);
    double neighbor_pt = 0;

    for(const int& neighborIndex: neighbors){
      const xAOD::JGTower* neighbor = towers.at(neighborIndex);
      block_area += neighbor->deta()*neighbor->dphi();
      neighbor_pt = neighbor->et();

      block_pt += neighbor_pt;
    }

    TowerObject::Block block(block_pt, seed->eta(), seed->phi(), 0.0);
    block.seedIndex(seedIndex);
    block.numEta(3);
    block.numPhi(3);
    block.area(block_area);
    block.numConstituents(neighbors.size());

    blocks.push_back(block);
  }
  std::sort(blocks.rbegin(), blocks.rend());

  float pt_cone_cut = 25*Gaudi::Units::GeV;

  for(unsigned b = 0; b < blocks.size(); b++){
    const xAOD::JGTower* seed = towers.at(blocks[b].seedIndex());
    float block_phi = blocks[b].Phi();
    float block_eta = blocks[b].Eta();
    float pt_cone = blocks[b].Pt();
    float seed_Et = seed->et();

    float j_Et = 0;

    if(pt_cone > pt_cone_cut){
      for(unsigned int t = 0; t < towers.size(); t++){
	const xAOD::JGTower* tower = towers.at(t);
	if(fabs(tower->et()) < noise.at(t)) continue;
	if(!inBox(block_eta, tower->eta(), jet_r, block_phi, tower->phi(), jet_r)) continue;
	j_Et += tower->et();
      }
    }
    if(j_Et < 10*Gaudi::Units::GeV) continue;
    JetAlg::L1Jet j = JetAlg::L1Jet(block_eta, block_phi, j_Et);
    js.push_back(j);
  }
  return StatusCode::SUCCESS;
}

StatusCode JetAlg::BuildJet(const xAOD::JGTowerContainer*towers,JetAlg::Seed*seeds, std::vector<JetAlg::L1Jet>& js, float jet_r, std::vector<float> noise, bool m_debug){
  for(unsigned eta_ind=0; eta_ind<seeds->eta.size(); eta_ind++){
     for(unsigned phi_ind=0; phi_ind<seeds->phi.at(eta_ind).size(); phi_ind++){
        if(!seeds->local_max.at(eta_ind).at(phi_ind)) continue;
        float eta = seeds->eta.at(eta_ind);
        float phi = seeds->phi.at(eta_ind).at(phi_ind);
        if(m_debug)
          std::cout << "BuildJet: found a local max seed at (eta,phi)=("<<eta<<","<<phi<<")" << std::endl;
        float j_et = 0;
        for(unsigned t=0; t<towers->size(); t++){
           const xAOD::JGTower* tower = towers->at(t);
           if(fabs(tower->et())<noise.at(t)) continue;
           if(!inBox(eta,tower->eta(),jet_r, phi, tower->phi(),jet_r)) continue;
           j_et += tower->et();
        }
        if(j_et<10000) continue;
        JetAlg::L1Jet j = JetAlg::L1Jet(eta,phi,j_et);
        js.push_back(j);
     }
  }
  return StatusCode::SUCCESS;
}


StatusCode JetAlg::BuildRoundJet(const xAOD::JGTowerContainer*towers, JetAlg::Seed*seeds, std::vector<JetAlg::L1Jet>& js, float jet_r, std::vector<float> noise, bool m_debug){
  for(unsigned eta_ind=0; eta_ind<seeds->eta.size(); eta_ind++){
     for(unsigned phi_ind=0; phi_ind<seeds->phi.at(eta_ind).size(); phi_ind++){
        if(!seeds->local_max.at(eta_ind).at(phi_ind)) continue;
        float eta = seeds->eta.at(eta_ind);
        float phi = seeds->phi.at(eta_ind).at(phi_ind);
        float j_et = 0;
        if(m_debug)
          std::cout << "BuildRoundJet: found a local max seed at (eta,phi)=("<<eta<<","<<phi<<")" << std::endl;
        for(unsigned t=0; t<towers->size(); t++){
           const xAOD::JGTower* tower = towers->at(t);
           if(fabs(tower->et())<noise.at(t)) continue;
           if(!withinRadius(eta, tower->eta(), phi, tower->phi(), jet_r)) continue;
           j_et += tower->et();
           if(m_debug)
             std::cout << "   adding tower at (eta,phi)=("<<tower->eta()<<","<<tower->phi()<<")" << std::endl;
        }
        if(m_debug)
          std::cout << "   final jet has et = " << j_et << std::endl;
        if(j_et<10000) continue;
        JetAlg::L1Jet j = JetAlg::L1Jet(eta,phi,j_et);
        js.push_back(j);
     }
  }
  return StatusCode::SUCCESS;
}

