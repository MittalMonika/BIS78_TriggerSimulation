//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILECONDITIONS_TILEBADCHANNELS_H
#define TILECONDITIONS_TILEBADCHANNELS_H

// Tile includes
#include "TileCalibBlobObjs/TileBchStatus.h"

// Athena includes
#include "Identifier/HWIdentifier.h"

/**
 * @class TileBadChannels
 * @brief Condition object to keep Tile channel and ADC status
 */

#include <map>

class TileBadChannels {

  public:

    TileBadChannels();
    virtual ~TileBadChannels();

   /**
    * @brief Add status for given Tile ADC and corresponding channel
    * @param channel_id Tile hardware (online) channel identifier
    * @param adc_id Tile hardware (online) ADC identifier
    * @param adcStatus Tile ADC status
    */
    void addAdcStatus(const HWIdentifier channel_id, const HWIdentifier adc_id, TileBchStatus adcStatus);


   /**
    * @brief Return Tile ADC status
    * @param adc_id Tile hardware (online) ADC identifier
    * @return Tile ADC status
    */
    const TileBchStatus& getAdcStatus(const HWIdentifier adc_id) const;

   /**
    * @brief Return Tile channel status
    * @param channel_id Tile hardware (online) channel identifier
    * @return Tile channel status
    */
    const TileBchStatus& getChannelStatus(const HWIdentifier channel_id) const;

   /**
    * @brief Store trips probabilities for all Tile drawers
    * @param tripsProbs Trips probabilites for all Tile drawers
    */
    void setTripsProbabilities(std::vector<std::vector<float>>& tripsProbs);

   /**
    * @brief Return trips probabilities for all Tile drawers
    * @return Trips probabilites for all Tile drawers
    */
    const std::vector<std::vector<float>>& getTripsProbabilities(void) const {return m_tripsProbs;};

  private:

    TileBchStatus m_defaultStatus;

    std::map<const HWIdentifier, TileBchStatus> m_adcStatus;
    std::map<const HWIdentifier, TileBchStatus> m_channelStatus;

    std::vector<std::vector<float>> m_tripsProbs;
};


#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"

CLASS_DEF( TileBadChannels, 186938202, 0)
CONDCONT_DEF ( TileBadChannels, 136686108);

// inlines
inline
void TileBadChannels::setTripsProbabilities(std::vector<std::vector<float>>& tripsProbs) {
  m_tripsProbs = std::move(tripsProbs);
}

#endif // TILECONDITIONS_TILEBADCHANNELS_H
