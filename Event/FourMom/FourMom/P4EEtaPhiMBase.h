///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FOURMOM_P4EETAPHIMBASE_H
#define FOURMOM_P4EETAPHIMBASE_H
#include "EventKernel/I4Momentum.h"
#include "CLHEP/Vector/LorentzVector.h"
#include <ostream>

/** 
   @class P4EEtaPhiMBase

   P4EEtaPhiMBase is a base class for classes with 4-momentum behavior, 
for which E, eta, phi and M are natural parameters, which is typically 
the case for a calorimeter object.
   Any class deriving from it should implement e(), eta(), phi(), m().
    
   @author David Rousseau rousseau@lal.in2p3.fr
 */


class P4EEtaPhiMBase : public virtual I4Momentum
{
 public:
  /** virtual destructor needed by pool */
  virtual ~P4EEtaPhiMBase();

  virtual double px() const ;
  virtual double py() const ;
  virtual double pz() const;
  virtual double et() const;
  /** magnitude of 3-momentum. Special implementation from Frank Paige : if 
negative energy p is negative but eta and phi still the same.  */
  virtual double p() const;
  virtual double p2() const;
  virtual double m2() const;
  virtual double pt() const;
  virtual double iPt() const;
  virtual double rapidity() const;
  virtual double cosPhi() const;
  virtual double sinPhi() const;
  virtual double cotTh() const;
  virtual double cosTh() const;
  virtual double sinTh() const;
  virtual double tanTh() const;
  /** HepLorentzVector Special implementation from Frank Paige : if 
negative energy, points in 
opposite direction but eta and phi still the same  */

  virtual CLHEP::HepLorentzVector hlv() const;

  virtual const I4MomentumError* errors() const;

  /** set all 4-mom from another I4Momentum reference DUMMY IMPLEMENTATION  */  
  virtual void set4Mom (const I4Momentum & theI4Mom );
  /** set all 4-mom from another I4Momentum pointer  DUMMY IMPLEMENTATION */  
  virtual void set4Mom (const I4Momentum * const theI4Mom );
  /** set all 4-mom from a CLHEP HepLorentzVector DUMMY IMPLEMENTATION */  
  virtual void set4Mom (const CLHEP::HepLorentzVector & theHlv );  


  I4Momentum::Kind kind() const { return I4Momentum::P4EETAPHIM; };

  /// Print @c I4Momentum content
  virtual std::ostream& dump( std::ostream& out = std::cout ) const;
};

#endif
