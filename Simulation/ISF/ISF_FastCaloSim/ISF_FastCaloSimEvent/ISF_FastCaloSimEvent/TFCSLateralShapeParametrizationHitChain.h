/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TFCSLateralShapeParametrizationHitChain_h
#define TFCSLateralShapeParametrizationHitChain_h

#include "ISF_FastCaloSimEvent/TFCSLateralShapeParametrization.h"
#include "ISF_FastCaloSimEvent/TFCSLateralShapeParametrizationHitBase.h"
#include <vector>

class TFCSLateralShapeParametrizationHitChain:public TFCSLateralShapeParametrization {
public:
  TFCSLateralShapeParametrizationHitChain(const char* name=nullptr, const char* title=nullptr);
  TFCSLateralShapeParametrizationHitChain(TFCSLateralShapeParametrizationHitBase* hitsim);

  virtual FCSReturnCode simulate(TFCSSimulationState& simulstate,const TFCSTruthState* truth, const TFCSExtrapolationState* extrapol) const override;

  virtual void set_geometry(ICaloGeometry* geo) override;

  typedef std::vector< TFCSLateralShapeParametrizationHitBase* > Chain_t;
  virtual unsigned int size() const override {return m_chain.size();};
  virtual const TFCSParametrizationBase* operator[](unsigned int ind) const override {return m_chain[ind];};
  virtual TFCSParametrizationBase* operator[](unsigned int ind) override {return m_chain[ind];};
  const Chain_t& chain() const {return m_chain;};
  Chain_t& chain() {return m_chain;};
  void push_back( const Chain_t::value_type& value ) {m_chain.push_back(value);};
  //TODO: add generic functionality to determine the number of hits or center position only once
  // and not for every iteration of the hit chain

  /// set which instance should determine the number of hits
  virtual void set_number_of_hits_simul(TFCSLateralShapeParametrizationHitBase* sim) {m_number_of_hits_simul=sim;};
  
  /// Call get_number_of_hits() only once, as it could contain a random number
  virtual int get_number_of_hits(TFCSSimulationState& simulstate,const TFCSTruthState* truth, const TFCSExtrapolationState* extrapol) const;
  
  ///Get hit energy from layer energy and number of hits
  virtual float get_E_hit(TFCSSimulationState& simulstate,const TFCSTruthState* truth, const TFCSExtrapolationState* extrapol) const;

  ///Give the effective size sigma^2 of the fluctuations that should be generated.
  virtual float get_sigma2_fluctuation(TFCSSimulationState& simulstate,const TFCSTruthState* truth, const TFCSExtrapolationState* extrapol) const;
  
  ///Get minimum and maximum value of weight for hit energy reweighting
  virtual float getMinWeight() const;
  virtual float getMaxWeight() const;

  static constexpr float s_max_sigma2_fluctuation=1000;//! Do not persistify!

  void Print(Option_t *option = "") const override;

#if defined(__FastCaloSimStandAlone__)
  /// Update outputlevel
  virtual void setLevel(int level,bool recursive=false) override {
    TFCSLateralShapeParametrization::setLevel(level,recursive);
    if(recursive) if(m_number_of_hits_simul) m_number_of_hits_simul->setLevel(level,recursive);
  }
#endif


protected:
  Chain_t m_chain;
  
private:
  TFCSLateralShapeParametrizationHitBase* m_number_of_hits_simul;
  ClassDefOverride(TFCSLateralShapeParametrizationHitChain,1)  //TFCSLateralShapeParametrizationHitChain
};

#endif
