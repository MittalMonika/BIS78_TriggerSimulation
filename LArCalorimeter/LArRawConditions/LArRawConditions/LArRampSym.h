/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARRAWCONDITIONS_LARRAMPSYM_H
#define LARRAWCONDITIONS_LARRAMPSYM_H

#include "LArElecCalib/ILArRamp.h"

class LArMCSym;
class LArRampMC;
class LArRampComplete;

class LArRampSym: public ILArRamp {
  
 public:

  LArRampSym() = delete;

  LArRampSym(const LArMCSym* mcsym,  const LArRampMC* rampComplete);
  
  virtual ~LArRampSym( );
  
  virtual  RampRef_t ADC2DAC(const HWIdentifier&  CellID, int gain) const override;
  
  virtual  RampRef_t ADC2DAC(const Identifier&  CellID, int gain) const override;
  
 private: 

  const LArMCSym* m_mcSym;
  const LArRampComplete* m_rampComplete;

};

#include "CLIDSvc/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
CONDCONT_BASE(LArRampSym,ILArRamp);
CLASS_DEF( LArRampSym,226012704,1)
CLASS_DEF( CondCont<LArRampSym> ,121227424 , 1 )

#endif 
