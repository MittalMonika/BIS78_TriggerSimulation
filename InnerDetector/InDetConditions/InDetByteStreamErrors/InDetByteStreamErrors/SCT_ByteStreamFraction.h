/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SCT_ByteStreamFraction.h
 * header file for SCT byte stream fraction object
 * @author Susumu Oda - 2018-03-06
 **/

#ifndef SCT_ByteStreamFraction_h
#define SCT_ByteStreamFraction_h

#include <cstdint>
#include <set>

// Include Athena stuff
#include "AthenaKernel/CLASS_DEF.h"

class SCT_ByteStreamFraction {
public:

  // Constructor
  SCT_ByteStreamFraction();

  // Destructor
  virtual ~SCT_ByteStreamFraction() = default;

  // Insert a new rodId with a bool value
  void insert(const uint32_t rodId, const bool value);

  // Clear m_trueSets and m_falseSets
  void clear();

  // Get majority
  bool majority() const;

  // Get entries
  unsigned int entries() const;

  // Get true set
  std::set<uint32_t> getTrueSet() const;

  // Get false set
  std::set<uint32_t> getFalseSet() const;

  // Set true set
  void setTrueSet(const std::set<uint32_t>& trueSet);

  // Set false set
  void setFalseSet(const std::set<uint32_t>& falseSet);

private:
  // Store RODIDs with true and false separately
  std::set<uint32_t> m_trueSet;
  std::set<uint32_t> m_falseSet;

};

CLASS_DEF( SCT_ByteStreamFraction , 96378427 , 1 )

#endif // SCT_ByteStreamFraction_h