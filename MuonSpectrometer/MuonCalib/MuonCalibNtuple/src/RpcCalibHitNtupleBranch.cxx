/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCalibNtuple/RpcCalibHitNtupleBranch.h"
#include "MuonCalibNtuple/NtupleBranchCreator.h"

#include "MuonCalibEventBase/RpcCalibHitBase.h"

#include "TTree.h"

#include <iostream>

namespace MuonCalib {

  RpcCalibHitNtupleBranch::RpcCalibHitNtupleBranch(std::string branchName) : m_branchName(branchName), branchesInit(false), index(0)
  {}

  bool RpcCalibHitNtupleBranch::fillBranch(const RpcCalibHitBase &hit, const int segmentIndex) {
    // check if branches where initialized
    if( !branchesInit ){
      //std::cout << "RpcCalibHitNtupleBranch::fillBranch  ERROR <branches where not initialized>"
      //	<<  std::endl;
      return false;    
    }

    // check if index not out of range 
    if( index >= m_blockSize || index < 0 ){
//       std::cout << "RpcCalibHitNtupleBranch::fillBranch  ERROR <index out of range, hit not added to ntuple> "
// 		<<  index << std::endl;
      return false;
    }

    // copy values 
    segIndex[index]   = segmentIndex;
    id[index]         = (hit.identify()).getIdInt();
    nStrips[index]    = hit.nStrips();
    stripWidth[index] = hit.stripWidth();
    stripLength[index]= hit.stripLength();
    time[index]       = hit.time();
    error[index]      = hit.error();
    posX[index]       = hit.localPosition().x();
    posY[index]       = hit.localPosition().y();
    posZ[index]       = hit.localPosition().z();
    gPosX[index]      = hit.globalPosition().x();
    gPosY[index]      = hit.globalPosition().y();
    gPosZ[index]      = hit.globalPosition().z();
    distanceToRO[index] = hit.distanceToRO();

    // increment hit index
    ++index;
  
    return true;
  }  //end RpcCalibHitNtupleBranch::fillBranch

  bool RpcCalibHitNtupleBranch::createBranch( TTree *tree ) {
    // check if pointer is valid
    if( !tree ){
      // std::cout << "RpcCalibHitNtupleBranch::createBranch  ERROR <got invalid tree pointer> " 
      //	<< std::endl;
      return false;
    }

    // helper class to create branches in trees
    NtupleBranchCreator branchCreator(m_branchName);

    std::string index_name ="nRpcHits";

    // create a branch for every data member
    branchCreator.createBranch( tree, index_name, &index, "/I");

    // all entries of same size, the number of hits in the event
    std::string array_size( std::string("[") + m_branchName + index_name + std::string("]") );

    // create the branches
    branchCreator.createBranch( tree, "segIndex",    &segIndex,    array_size + "/I" );
    branchCreator.createBranch( tree, "id",          &id,          array_size + "/I" );
    branchCreator.createBranch( tree, "nStrips",     &nStrips,     array_size + "/I" );
    branchCreator.createBranch( tree, "stripWidth",  &stripWidth,  array_size + "/F" );
    branchCreator.createBranch( tree, "stripLength", &stripLength, array_size + "/F" );
    branchCreator.createBranch( tree, "error",       &error,       array_size + "/F" );
    branchCreator.createBranch( tree, "time",        &time,        array_size + "/F" );
    branchCreator.createBranch( tree, "posX",        &posX,        array_size + "/F" );
    branchCreator.createBranch( tree, "posY",        &posY,        array_size + "/F" );
    branchCreator.createBranch( tree, "posZ",        &posZ,        array_size + "/F" );
    branchCreator.createBranch( tree, "gPosX",       &gPosX,       array_size + "/F" );
    branchCreator.createBranch( tree, "gPosY",       &gPosY,       array_size + "/F" );
    branchCreator.createBranch( tree, "gPosZ",       &gPosZ,       array_size + "/F" );
    branchCreator.createBranch( tree, "distanceToRO",&distanceToRO,array_size + "/F" );

    branchesInit = true;
  
    // reset branch
    reset();

    return true;
  }  //end RpcCalibHitNtupleBranch::createBranch

}  //end namespace MuonCalib
