/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FASTCALOSIMEVENT_TFCSParametrizationBase_h
#define ISF_FASTCALOSIMEVENT_TFCSParametrizationBase_h

#include <TNamed.h>
#include <set>

class ICaloGeometry;
class TFCSSimulationState;
class TFCSTruthState;
class TFCSExtrapolationState;

// Define Athena like message macro's such that they work stand alone and inside athena
#if defined(__FastCaloSimStandAlone__)
  #include <iostream>
  #include <iomanip>
  typedef std::ostream MsgStream;
  #define endmsg std::endl
  
  namespace MSG {
    enum Level {
      NIL = 0,
      VERBOSE,
      DEBUG,
      INFO,
      WARNING,
      ERROR,
      FATAL,
      ALWAYS,
      NUM_LEVELS
    }; // enum Level
    const char* LevelNames[NUM_LEVELS]={"NIL","VERBOSE","DEBUG","INFO","WARNING","ERROR","FATAL","ALWAYS"};
  }  // end namespace MSG  
  // Needs a check despite the name, as stand alone mode is not using MsgStream, but just cout internally
  #define ATH_MSG_LVL_NOCHK(lvl, x)               \
    do {                                          \
      if(this->msgLvl(lvl)) this->msg(lvl)<<setw(45)<<std::left<<this->GetName()<<" "<<MSG::LevelNames[lvl]<<" "<< x << endmsg; \
    } while (0)

  #define ATH_MSG_LVL(lvl, x)                     \
    do {                                          \
      if (this->msgLvl(lvl)) ATH_MSG_LVL_NOCHK(lvl, x);                \
    } while (0)

  #define ATH_MSG_VERBOSE(x) ATH_MSG_LVL(MSG::VERBOSE, x)
  #define ATH_MSG_DEBUG(x)   ATH_MSG_LVL(MSG::DEBUG, x)
  // note that we are using the _NOCHK variant here
  #define ATH_MSG_INFO(x)    ATH_MSG_LVL_NOCHK(MSG::INFO, x)
  #define ATH_MSG_WARNING(x) ATH_MSG_LVL_NOCHK(MSG::WARNING, x)
  #define ATH_MSG_ERROR(x)   ATH_MSG_LVL_NOCHK(MSG::ERROR,  x)
  #define ATH_MSG_FATAL(x)   ATH_MSG_LVL_NOCHK(MSG::FATAL,  x)

  // can be used like so: ATH_MSG(INFO) << "hello" << endmsg;
  #define ATH_MSG(lvl) \
    if (this->msgLvl(MSG::lvl)) this->msg(MSG::lvl)<<setw(45)<<std::left<<this->GetName()<<" "<<MSG::LevelNames[MSG::lvl]<<" " 

#else
  #include "AthenaKernel/MsgStreamMember.h"
  #include "AthenaBaseComps/AthMsgStreamMacros.h"
#endif

class TFCSParametrizationBase:public TNamed {
public:
  TFCSParametrizationBase(const char* name=nullptr, const char* title=nullptr);

  virtual bool is_match_pdgid(int /*id*/) const {return false;};
  virtual bool is_match_Ekin(float /*Ekin*/) const {return false;};
  virtual bool is_match_eta(float /*eta*/) const {return false;};

  virtual bool is_match_Ekin_bin(int /*Ekin_bin*/) const {return false;};
  virtual bool is_match_calosample(int /*calosample*/) const {return false;};

  virtual const std::set< int > &pdgid() const {return m_no_pdgid;};
  virtual double Ekin_nominal() const {return 0;};
  virtual double Ekin_min() const {return 0;};
  virtual double Ekin_max() const {return 0;};
  virtual double eta_nominal() const {return 100;};
  virtual double eta_min() const {return 100;};
  virtual double eta_max() const {return 100;};

  virtual void set_geometry(ICaloGeometry*) {};

  // Do some simulation. Result should be returned in simulstate
  // Simulate all energies in calo layers for energy parametrizations
  // Simulate one HIT for later shape parametrizations (TO BE DISCUSSED!)
  virtual void simulate(TFCSSimulationState& simulstate,const TFCSTruthState* truth, const TFCSExtrapolationState* extrapol);

  void Print(Option_t *option = "") const;
  
#if defined(__FastCaloSimStandAlone__)
public:
  /// Update outputlevel
  void setLevel(int level) {
    level = (level >= MSG::NUM_LEVELS) ?
      MSG::ALWAYS : (level<MSG::NIL) ? MSG::NIL : level;
    m_level = MSG::Level(level);
  }
  /// Retrieve output level
  MSG::Level level() const {return m_level;}

  /// Log a message using cout; a check of MSG::Level lvl is not possible!
  MsgStream& msg() const {return *m_msg;}
  MsgStream& msg( const MSG::Level lvl ) const {return *m_msg;}  
  /// Check whether the logging system is active at the provided verbosity level
  bool msgLvl( const MSG::Level lvl ) const {return m_level<=lvl;}
private:
  MSG::Level m_level;//! Do not persistify!
  
  MsgStream* m_msg;//! Do not persistify!
#else
public:
  /// Update outputlevel
  void setLevel(int level) {s_msg->get().setLevel(level);}
  /// Retrieve output level
  MSG::Level level() const {return s_msg->get().level();}
  /// Log a message using the Athena controlled logging system
  MsgStream& msg() const { return s_msg->get(); }
  MsgStream& msg( MSG::Level lvl ) const { return *s_msg << lvl; }
  /// Check whether the logging system is active at the provided verbosity level
  bool msgLvl( MSG::Level lvl ) const { return s_msg->get().level() <= lvl; }
  
private:
  /// Static private message stream member. We don't want this to take memory for every instance of this object created
  static Athena::MsgStreamMember* s_msg;//! Do not persistify!
#endif  
  
private:
  static std::set< int > m_no_pdgid;

  ClassDef(TFCSParametrizationBase,1)  //TFCSParametrizationBase
};

#if defined(__ROOTCLING__) && defined(__FastCaloSimStandAlone__)
#pragma link C++ class TFCSParametrizationBase+;
#endif

#endif
