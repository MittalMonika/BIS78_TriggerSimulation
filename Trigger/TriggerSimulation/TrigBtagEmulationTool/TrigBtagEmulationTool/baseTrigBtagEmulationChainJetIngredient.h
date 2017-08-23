#ifndef baseTrigBtagEmulationChainJetIngredient_H
#define baseTrigBtagEmulationChainJetIngredient_H

#include "TrigBtagEmulationTool/feature.h"

namespace Trig {

  class baseTrigBtagEmulationChainJetIngredient {
  public:
    baseTrigBtagEmulationChainJetIngredient();
    baseTrigBtagEmulationChainJetIngredient(std::string,float,float,float,unsigned int);
    baseTrigBtagEmulationChainJetIngredient(const baseTrigBtagEmulationChainJetIngredient&);
    ~baseTrigBtagEmulationChainJetIngredient();

    void addFeature(std::string,feature*);
    bool evaluate();
    virtual bool evaluateJet(struct TrigBtagEmulationJet&);
    virtual void Print() = 0;
    virtual bool isPassed(); 
    virtual bool hasFeature(std::string); 
    virtual void clear(); 

    virtual bool needsJet(std::string);
    bool addJet(std::string,std::vector< struct TrigBtagEmulationJet >&);

  public:
    std::string _triggerName;
    float _min_pt;
    float _min_eta;
    float _max_eta;
    unsigned int _min_mult;
    unsigned int _count;

    std::vector< std::pair<std::string,feature*> > type_THRESHOLD_features;
    std::vector< std::pair<std::string,feature*> > type_SELECTION_features;

  public:
    std::map< std::string,std::string > _neededJetCollection;
    std::map< std::string,std::vector< struct TrigBtagEmulationJet >* > _jetCollection;
  };

  // *** L1

  class TrigBtagEmulationChainJetIngredient_L1 : public baseTrigBtagEmulationChainJetIngredient {
  public:
    TrigBtagEmulationChainJetIngredient_L1();
    TrigBtagEmulationChainJetIngredient_L1(std::string,float,float,float,unsigned int);
    TrigBtagEmulationChainJetIngredient_L1(const TrigBtagEmulationChainJetIngredient_L1&);
    ~TrigBtagEmulationChainJetIngredient_L1();

    virtual void Print();
  };  

  // *** L1 JJ

  class TrigBtagEmulationChainJetIngredient_L1_JJ : public TrigBtagEmulationChainJetIngredient_L1 {
  public:
    TrigBtagEmulationChainJetIngredient_L1_JJ();
    TrigBtagEmulationChainJetIngredient_L1_JJ(std::string,float,float,float,unsigned int);
    TrigBtagEmulationChainJetIngredient_L1_JJ(const TrigBtagEmulationChainJetIngredient_L1_JJ&);
    ~TrigBtagEmulationChainJetIngredient_L1_JJ();

    void Print();
  };

  // *** HLT

  class TrigBtagEmulationChainJetIngredient_HLT : public baseTrigBtagEmulationChainJetIngredient {
  public:
    TrigBtagEmulationChainJetIngredient_HLT();
    TrigBtagEmulationChainJetIngredient_HLT(std::string,float,float,float,unsigned int);
    TrigBtagEmulationChainJetIngredient_HLT(const TrigBtagEmulationChainJetIngredient_HLT&);
    ~TrigBtagEmulationChainJetIngredient_HLT();

    virtual void Print();
  };

  // *** GSC

  class TrigBtagEmulationChainJetIngredient_GSC : public TrigBtagEmulationChainJetIngredient_HLT {
  public:
    TrigBtagEmulationChainJetIngredient_GSC();
    TrigBtagEmulationChainJetIngredient_GSC(std::string,float,float,float,float,unsigned int);
    TrigBtagEmulationChainJetIngredient_GSC(const TrigBtagEmulationChainJetIngredient_GSC&);
    ~TrigBtagEmulationChainJetIngredient_GSC();

    bool evaluateJet(struct TrigBtagEmulationJet&);
    bool isPassed();
    bool hasFeature(std::string);
    void Print();

    bool needsJet(std::string);
    void clear();

  private:
    unsigned int matchingJets(struct TrigBtagEmulationJet&);

  public:
    float _min_gsc;
    unsigned int _min_mult_gsc;
    unsigned int _count_gsc;
  };
  
}//namespace

#endif
