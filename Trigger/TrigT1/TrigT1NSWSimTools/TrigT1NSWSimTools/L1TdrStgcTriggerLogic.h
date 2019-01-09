// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef NSW_L1TDRSTGCTRIGGERLOGIC_H
#define NSW_L1TDRSTGCTRIGGERLOGIC_H

#include "TrigT1NSWSimTools/TriggerTypes.h"
#include "TrigT1NSWSimTools/SectorTriggerCandidate.h"
#include "TrigT1NSWSimTools/SingleWedgePadTrigger.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "AthenaKernel/MsgStreamMember.h"
#include <string>
#include <vector>
#include "TRandom.h"

//forward declarations
class MsgStream;

namespace NSWL1 {
    
/**
  @brief Initial version of the stgc pad trigger logic

  This class contains the trigger logic for the stgc pads as implemented
  for the NSW TDR (CERN-LHCC-2013-006, https://cds.cern.ch/record/1552862).
  In particular, the naming convention and the trigger logic are
  described in ATL-MUON-INT-2014-003 https://cds.cern.ch/record/1694727.

  This class requires the input data to be formatted as a vector<PadWithHits>,
  and it provides as output a vector<SectorTriggerCandidate>.

  Imported from
  svn+ssh://svn.cern.ch/reps/atlasusr/ataffard/UpgradeNSW/NSWAna/NSWNtuple/trunk ,
  this class is in TrigT1NSWSimTools mainly meant to provide a reference.
  Note that all the 'legacy' code from the TDR study is in the 'nsw' namespace.
  (newer 'official' code is in the 'NSWL1' namespace).

  @todo add unit test
 
  Based on the original implementation by Shikma, sTGCTriggerLogic.
 
  April 2013
*/
class L1TdrStgcTriggerLogic {

    public:
        
        L1TdrStgcTriggerLogic();//get svc from parent class
        virtual ~L1TdrStgcTriggerLogic();
        /**
        @brief main function to compute trigger candidates

        @arg pads all active pads in this event
        @arg padIndicesThisSector indices of pads that should be
        considered to compute the triggers for this sector (i.e. after
        filtering by side+sector and eventually applying the
        efficiency)

        */
        
        bool buildSectorTriggers(const std::vector<PadWithHits> &pads);
        /// access cached output of buildSectorTriggers()
        const std::vector< SectorTriggerCandidate >& candidates() const {return m_secTrigCand;}
        /// simulate efficiency by dropping random pads (their indices)
        std::vector< size_t > removeRandomPadIndices(const std::vector< size_t > &padIndices);
        /**
        @brief trigger patterns that will be stored in the lookup table

        These are the original patterns we got in 2013 from D.Lellouch.
        They are probably obsolete and they should be updated
        \todo update trigger patterns
        */
        static std::vector<std::string> sTGC_triggerPatterns();
        static std::vector<std::string> sTGC_triggerPatternsEtaUp();
        static std::vector<std::string> sTGC_triggerPatternsEtaDown();
        static std::vector<std::string> sTGC_triggerPatternsPhiUp();
        static std::vector<std::string> sTGC_triggerPatternsPhiDown();
        static std::vector<std::string> sTGC_triggerPatternsPhiDownUp();
        static std::vector<std::string> sTGC_triggerPatternsPhiUpDown();

        static bool hitPattern(const Pad &firstPad, const Pad &otherPad,
                            std::string &pattern);
        static bool hitPattern(const int &iEta0, const int &iPhi0,
                            const int &iEta1, const int &iPhi1,
                            std::string &pattern);
        static std::vector< SingleWedgePadTrigger > buildSingleWedgeTriggers(const std::vector<PadWithHits> &pads,
                                                        const std::vector< size_t > &padIndicesLayer0,
                                                        const std::vector< size_t > &padIndicesLayer1,
                                                        const std::vector< size_t > &padIndicesLayer2,
                                                        const std::vector< size_t > &padIndicesLayer3,
                                                        bool isLayer1, bool isLayer2,
                                                        bool isLayer3, bool isLayer4);
        static std::vector< SingleWedgePadTrigger > build34swt(const std::vector<PadWithHits> &pads,
                                                            const std::vector< size_t > &padIndicesLayer0,
                                                            const std::vector< size_t > &padIndicesLayer1,
                                                            const std::vector< size_t > &padIndicesLayer2,
                                                            const std::vector< size_t > &padIndicesLayer3);
        static std::vector< SingleWedgePadTrigger > build44swt(const std::vector<PadWithHits> &pads,
                                                            const std::vector< size_t > &padIndicesLayer0,
                                                            const std::vector< size_t > &padIndicesLayer1,
                                                            const std::vector< size_t > &padIndicesLayer2,
                                                            const std::vector< size_t > &padIndicesLayer3);

    protected:
        /// Log a message using the Athena controlled logging system
        MsgStream& msg(MSG::Level lvl) const { return m_msg.get() << lvl; }
   
        /// Check whether the logging system is active at the provided verbosity level
        bool msgLvl(MSG::Level lvl) { return m_msg.get().level() <= lvl; }
   
        /// Private message stream member
        mutable Athena::MsgStreamMember m_msg;
        
    private:
        TRandom m_rand;
        std::vector< SectorTriggerCandidate > m_secTrigCand;        
        bool m_writePickle; /// after computing the triggers, write the canditates to 'pickle' files (for event display)
        std::string m_picklePrefix; /// path where the pickle files will be written
        
    };

} // end namespace NSWL1
#endif // NSW_L1TDRSTGCTRIGGERLOGIC_H
