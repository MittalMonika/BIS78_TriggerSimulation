/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DecisionHandling_HLTIdentifier_h
#define DecisionHandling_HLTIdentifier_h

#include <string>
#include <vector>
#include <set>
#include <map>



#include "GaudiKernel/MsgStream.h"
/**
 * @brief An trigger identifier class, used to provide mapping fromt the human readable IDs to efficienct unsigned ints
 **/
namespace HLT {
class Identifier {
public:
  /**
   * @brief disables reporting human readable names == no debugging, fast execution
   **/
  static void reportStringIDs( bool report ) { s_reportStringIDs = report; }
  static bool reportStringIDs() { return s_reportStringIDs; }
  /**
   * @brief constructs identifier from human redable name
   **/  
  Identifier( const std::string& stringID );

  /**
   * @brief Construct wiht numeric ID
   **/
 Identifier( unsigned id ) : m_id( id ) {}

  /**
   * @brief reports human redable name if it is enabled or, empty string
   **/  
  std::string name() const;
  
  /**
   * @brief numeric ID
   **/    
  unsigned numeric() const { return m_id; }
  /**
   *  @brief comparisons, for containers of identifiers
   **/      
  bool operator == ( const Identifier& rhs )  const { return numeric() == rhs.numeric(); }
  bool operator < ( const Identifier& rhs )  const { return numeric() < rhs.numeric(); }
private:
  unsigned m_id;
  static bool s_reportStringIDs;
  
};
 typedef std::vector<HLT::Identifier> IDVec;
 typedef std::set<HLT::Identifier> IDSet;
 typedef std::map<HLT::Identifier,  IDVec> IDtoIDVecMap;
}

MsgStream& operator<< ( MsgStream& m, const HLT::Identifier& id );



#endif // HLTIdentifier
