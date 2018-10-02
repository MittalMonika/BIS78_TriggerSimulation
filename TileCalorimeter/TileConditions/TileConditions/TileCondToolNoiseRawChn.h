/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILECONDITIONS_TILECONDTOOLNOISERAWCHN_H
#define TILECONDITIONS_TILECONDTOOLNOISERAWCHN_H

// Tile includes
#include "TileConditions/ITileCondToolNoise.h"
#include "TileConditions/TileCalibData.h"
#include "TileConditions/TileEMScale.h"
#include "TileIdentifier/TileRawChannelUnit.h"

// Athena includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadCondHandleKey.h"


class TileCondToolNoiseRawChn: public AthAlgTool
                             , virtual public ITileCondToolNoise {
  public:

    static const InterfaceID& interfaceID();

    TileCondToolNoiseRawChn(const std::string& type, const std::string& name, const IInterface* parent);

    virtual ~TileCondToolNoiseRawChn();

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    float getElectronicNoise(unsigned int drawerIdx, unsigned int channel, unsigned int adc,
                             TileRawChannelUnit::UNIT unit = TileRawChannelUnit::ADCcounts) const;

    float getPileUpNoise(unsigned int drawerIdx, unsigned int channel, unsigned int adc) const;

    float getNoise(unsigned int drawerIdx, unsigned int channel, unsigned int adc,
                   TileRawChannelUnit::UNIT unit = TileRawChannelUnit::ADCcounts) const {
      return getElectronicNoise(drawerIdx, channel, adc, unit);
    }

  private:

    SG::ReadCondHandleKey<TileCalibDataFlt> m_calibRawChannelNoiseKey{this,
        "TileRawChannelNoise", "TileRawChannelNoise", "Input Tile raw channel noise constants"};

    SG::ReadCondHandleKey<TileEMScale> m_emScaleKey{this,
        "TileEMScale", "TileEMScale", "Input Tile EMS conditions"};

};

#endif
