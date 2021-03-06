// Dear emacs, this is -*- c++ -*-
/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1INTERFACES_LVL1MUCTPIINPUTPHASE1_H
#define TRIGT1INTERFACES_LVL1MUCTPIINPUTPHASE1_H

// System include(s):
#include <iosfwd>
#include <vector>
#include <utility>
#include <memory>

// Local include(s):
#include "TrigT1Interfaces/Lvl1MuSectorLogicDataPhase1.h"
#include "TrigT1Interfaces/Lvl1MuBarrelSectorLogicDataPhase1.h"
#include "TrigT1Interfaces/Lvl1MuEndcapSectorLogicDataPhase1.h"
#include "TrigT1Interfaces/Lvl1MuForwardSectorLogicDataPhase1.h"

namespace LVL1MUONIF {

   /**
    *  @short Class representing (part of) the input data to the MuCTPI for Phase 1
    *
    *         This class can represent the full input data coming from the
    *         SL boards to the MuCTPI, or it can just represent a part of it.
    *         (Let's say only the data coming from the RPC simulation.)
    *
    *    @see LVL1MUONIF::Lvl1MuBarrelSectorLogicDataPhase1
    *    @see LVL1MUONIF::Lvl1MuEndcapSectorLogicDataPhase1
    *    @see LVL1MUONIF::Lvl1MuForwardSectorLogicDataPhase1
    *
    * $Revision: 782811 $
    * $Date: 2016-11-07 18:20:40 +0100 (Mon, 07 Nov 2016) $
    */
   class Lvl1MuCTPIInputPhase1 {

   public:
      Lvl1MuCTPIInputPhase1();
      Lvl1MuCTPIInputPhase1( const Lvl1MuCTPIInputPhase1& );
      virtual ~Lvl1MuCTPIInputPhase1();

      Lvl1MuCTPIInputPhase1& operator=( const Lvl1MuCTPIInputPhase1& right );

   protected:
      // data from each sector logic is identifed by
      //  System Address     :  Barrel=0, Endcap=1, Forward=2
      //  Sub-system Address :  A_side(Z>0) =1, C_side(Z<0) =0
      //  Sector Address     :  Barrel=0-31, Endcap=0-47, Forward=0-23
      enum MuonSystem { Barrel = 0, Endcap = 1, Forward = 2 };
      enum MuonSubSystem { A_side = 1, C_side = 0};
      enum { NumberOfMuonSystem = 3 };
      enum { NumberOfMuonSubSystem = 2 };
      enum { NumberOfBarrelSector = 32 };
      enum { NumberOfEndcapSector = 48 };
      enum { NumberOfForwardSector = 24 }; 

   public:
      static size_t idBarrelSystem() { return Barrel; }
      static size_t numberOfBarrelSector() { return NumberOfBarrelSector; }
      static size_t idEndcapSystem() { return Endcap; }
      static size_t numberOfEndcapSector() { return NumberOfEndcapSector; }
      static size_t idForwardSystem() { return Forward; }
      static size_t numberOfForwardSector() { return NumberOfForwardSector; }
      static size_t idSideA() { return A_side; }
      static size_t idSideC() { return C_side; }

      const Lvl1MuSectorLogicDataPhase1& getSectorLogicData( size_t systemAddress,
                                                       size_t subSystemAddress,
                                                       size_t sectorAddress,
						       int    bcid=0        ) const;

      void setSectorLogicData( const Lvl1MuSectorLogicDataPhase1& data,
                               size_t systemAddress,
                               size_t subSystemAddress,
                               size_t sectorAddress,
			       int    bcid=0        );

      void setSectorLogicData( const Lvl1MuSectorLogicDataPhase1::SectorLogicWord & sectorWord,
                               size_t systemAddress,
                               size_t subSystemAddress,
                               size_t sectorAddress,
			       int    bcid=0        );

      /**
       * Merge SectorLogic data of another object into this
       * data of a system (Barrel, Endcap or Forward) is not overwritten
       * if this object already has corresponding data
       */
      void merge( const Lvl1MuCTPIInputPhase1& another );

      void clear( size_t systemAddress );
      /// Clear Sector Logic data
      void clearAll();

      /// Return "true" if  data of specified system is empty
      bool isEmpty( size_t systemAddress,
		    int    bcid=0         ) const;

      /// Return "true" if  data of all systems for this bcid is empty
      bool isEmptyAll(int bcid=0) const;

      /// Return "true" if data of specified system is filled
      /// for bunches other than 'current' bunch 
      bool hasOutOfTimeCandidates(size_t systemAddress)  const
      {
	if ( systemAddress <  NumberOfMuonSystem) {
	  return isFilledOutOfTimeCandidates[systemAddress];
	} 
	return false;
      }

     friend std::ostream& operator<<( std::ostream&, const Lvl1MuCTPIInputPhase1& );

   private:

      size_t reserve( size_t systemAddress ,
		      int    bcid=0         );
     
      size_t getSystemIndex(size_t systemAddress,
	 	      size_t subSystemAddress,
		      size_t sectorAddress ) const
      {
	if (systemAddress==0) {
	  return NumberOfBarrelSector*subSystemAddress+sectorAddress;
	} else if (systemAddress==1) {
	  return NumberOfEndcapSector*subSystemAddress+sectorAddress;
	} else {
	  return NumberOfForwardSector*subSystemAddress+sectorAddress;
	}
      }

     size_t getBcidIndex(size_t systemAddress,
                         int    bcid=0) const
     {
       for( size_t ip=0; ip<m_data[systemAddress].size(); ip++){
         int bc=((m_data[systemAddress]).at(ip)).first;
         if (bc == bcid) return ip;
       }
       return -1;
     };

     bool isFilledOutOfTimeCandidates[NumberOfMuonSystem];

     
     typedef std::vector<std::shared_ptr <Lvl1MuSectorLogicDataPhase1> > Lvl1MuVect;
     typedef std::pair<int, Lvl1MuVect>  Lvl1MuVectWithBC; 
     std::vector<Lvl1MuVectWithBC> m_data[ NumberOfMuonSystem ];

   }; // class Lvl1MuCTPIInputPhase1

} // namespace LVL1MUONIF

#ifndef MUCTPI_STANDALONE
#ifndef CLIDSVC_CLASSDEF_H
#include "CLIDSvc/CLASS_DEF.h"
#endif
CLASS_DEF( LVL1MUONIF::Lvl1MuCTPIInputPhase1, 6112, 0 )
#endif

#endif // TRIGT1INTERFACES_LVL1MUCTPIINPUTPHASE1_H
