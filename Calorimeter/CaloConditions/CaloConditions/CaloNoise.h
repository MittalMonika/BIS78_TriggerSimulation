//Dear emacs, this is -*-c++-*-
/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCONDITIONS_CALONOISE_H
#define CALOCONDITIONS_CALONOISE_H

#include <boost/multi_array.hpp>
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloCondBlobObjs/CaloCondUtils.h" 
#include "CaloCondBlobObjs/CaloCondBlobFlt.h"
          
class CaloNoise {
  ///Conditions Data Object holding the calorimeter noise per cell and per gain

 public:
 
  CaloNoise() =delete;

  ~CaloNoise();

  /// Explicit constructor with number of cells and gains and ptr to CaloCell_ID obj 
  CaloNoise(const size_t nLArCells, const size_t nLArGains, const size_t nTileCells, const size_t nTileGains,
	    const CaloCell_Base_ID* caloCellId);

  /// Accessor by IdentifierHash and gain.
  float getNoise(const IdentifierHash h, const int gain) const {
    if (h<m_tileHashOffset) {
      return m_larNoise[gain][h];
    }
    else {
      const unsigned int dbGain = CaloCondUtils::getDbCaloGain(gain);
      return m_tileNoise[dbGain][h-m_tileHashOffset];
    }
  }
  
  /// Accessor by Identifier and gain.
  float getNoise(const Identifier id, const int gain) const {
    IdentifierHash h=m_caloCellId->calo_cell_hash(id);
    return getNoise(h,gain);
  }

  float getEffectiveSigma(const Identifier id, const int gain, const float energy) {
     IdentifierHash h=m_caloCellId->calo_cell_hash(id);
     if (h<m_tileHashOffset) {
       return m_larNoise[gain][h];
     }
     else
       return calcSig(h-m_tileHashOffset,gain,energy);
  }

  float calcSig(const IdentifierHash tilehash, const int gain, const float energy) const;

  /// Non-const accessor to underlying storage for filling:
  boost::multi_array<float, 2>& larStorage() {return m_larNoise;}
  boost::multi_array<float, 2>& tileStorage() {return m_tileNoise;}

  void setTileBlob(const CaloCondBlobFlt* flt, const float lumi);

 private:
  const CaloCell_Base_ID* m_caloCellId;

  //Flat structure, choosen based on profiling done by Scott in Nov 2013
  boost::multi_array<float, 2> m_larNoise;
  boost::multi_array<float, 2> m_tileNoise;
  unsigned m_tileHashOffset;


  //For double-gaussian noise:
  const CaloCondBlobFlt* m_tileBlob=nullptr; 
  float m_lumi=0;

};

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( CaloNoise , 16721262 , 1 )
#include "AthenaKernel/CondCont.h"
CONDCONT_MIXED_DEF(CaloNoise, 51500196);
#endif
