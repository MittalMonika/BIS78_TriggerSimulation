/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCalibNtuple/RpcTruthHitNtupleBranch.h"
#include "MuonCalibNtuple/NtupleBranchCreator.h"

#include "MuonCalibEventBase/MuonCalibRpcTruthHit.h"

#include "TTree.h"

#include <iostream>

namespace MuonCalib {

  RpcTruthHitNtupleBranch::RpcTruthHitNtupleBranch(std::string branchName) : m_branchName(branchName), m_branchesInit(false), m_index(0)
  {}

  bool RpcTruthHitNtupleBranch::fillBranch(const MuonCalibRpcTruthHit &hit) {
    // check if branches were initialized
    if( !m_branchesInit ){
      //std::cout << "RpcTruthHitNtupleBranch::fillBranch  ERROR <branches were not initialized>"
      //	<<  std::endl;
      return false;    
    }

    // check if index not out of range 
    if( m_index >= m_blockSize || m_index < 0 ){
//       std::cout << "RpcTruthHitNtupleBranch::fillBranch  ERROR <index out of range; hit not added to ntuple> "
// 		<<  m_index << std::endl;
      return false;
    }

    // copy values 
    m_id[m_index] = hit.identify().getIdInt();
    m_barCode[m_index] = hit.barCode();
    m_time[m_index] = hit.time();

    // increment hit index
    ++m_index;
  
    return true;
  }  //end RpcTruthHitNtupleBranch::fillBranch

  bool RpcTruthHitNtupleBranch::createBranch(TTree *tree) {
    // check if pointer is valid
    if( !tree ){
      //std::cout << "RpcTruthHitNtupleBranch::createBranch  ERROR <got invalid tree pointer> " 
      //	<< std::endl;
      return false;
    }

    // helper class to create branches in trees
    NtupleBranchCreator branchCreator(m_branchName);

    std::string index_name ="nRpcTruthHit";

    // create a branch for every data member
    branchCreator.createBranch( tree, index_name, &m_index, "/I");

    // all entries of same size, the number of hits in the event
    std::string array_size( std::string("[") + m_branchName + index_name + std::string("]") );

    // create the branches
    branchCreator.createBranch( tree, "id",      &m_id,      array_size + "/I" );
    branchCreator.createBranch( tree, "barCode", &m_barCode, array_size + "/I" );
    branchCreator.createBranch( tree, "time",    &m_time,    array_size + "/D" );

    m_branchesInit = true;
  
    // reset branch
    reset();

    return true;
  }  //end RpcTruthHitNtupleBranch::createBranch

}  //end namespace MuonCalib
