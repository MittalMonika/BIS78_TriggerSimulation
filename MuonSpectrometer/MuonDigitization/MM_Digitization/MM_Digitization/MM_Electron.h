/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MM_DIGITIZATION_MM_ELECTRON_H
#define MM_DIGITIZATION_MM_ELECTRON_H
//
// MM_Electron.cxx
//     Primary electron objects that are diffused, propagated and avalanched
//

#include <memory>

#include "TF1.h"
#include "TRandom3.h"
#include "TVector2.h"
#include "TMath.h"

class MM_Electron {

 public:

  MM_Electron();
  MM_Electron(float x, float y);
  MM_Electron(const MM_Electron& MM_Electron);

  void diffuseElectron(float LongitudinalSigma, float TransverseSigma, TRandom3* rndm);
  void setOffsetPosition(float x, float y);
  void propagateElectron(float lorentzAngle, float driftVel);
  void setTime(float Time);
  void setCharge(float Charge);

  TVector2 getOffsetPosition() const;
  float getCharge() const;
  float getTime() const;
  float getX() const;
  float getY() const;
  float getInitialX() const;
  float getInitialY() const;

 private:

  TVector2 m_initialPosition;
  TVector2 m_offsetPosition;

  float m_time;
  float m_charge;


};

#endif
