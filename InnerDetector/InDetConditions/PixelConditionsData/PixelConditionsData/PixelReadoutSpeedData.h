/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELREADOUTSPEEDDATA_H
#define PIXELREADOUTSPEEDDATA_H

//#define PIXEL_DEBUG
#include "AthenaKernel/CLASS_DEF.h"
#include <map>

class PixelReadoutSpeedData {
  public:
    PixelReadoutSpeedData();
    virtual ~PixelReadoutSpeedData();
    PixelReadoutSpeedData(const PixelReadoutSpeedData &other) = delete;
    PixelReadoutSpeedData& operator= (const PixelReadoutSpeedData &other) = delete;

    void setReadoutMap(std::map<uint32_t,bool> rodReadoutMap);
    std::map<uint32_t,bool> getReadoutMap() const;

    void clear();

  private:
    std::map<uint32_t,bool> m_rodReadoutMap;     ///< Readout speed for each ROD. false=40MBit, true=80MBit
};

inline void PixelReadoutSpeedData::setReadoutMap(std::map<uint32_t,bool> rodReadoutMap) { m_rodReadoutMap=rodReadoutMap; }

inline std::map<uint32_t,bool> PixelReadoutSpeedData::getReadoutMap() const { return m_rodReadoutMap; }

CLASS_DEF( PixelReadoutSpeedData , 107567278 , 1 )   // class definition with CLID

#include "AthenaKernel/CondCont.h"
CONDCONT_DEF( PixelReadoutSpeedData, 107567679 );

#endif