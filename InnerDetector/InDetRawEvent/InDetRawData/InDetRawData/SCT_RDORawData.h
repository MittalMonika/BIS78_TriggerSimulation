/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SCT_RawData.h
//   Header file for class SCT_RawData
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Class to implement RawData for SCT
///////////////////////////////////////////////////////////////////
// Version 1.0 13/08/2002 Veronique Boisvert
///////////////////////////////////////////////////////////////////

#ifndef SITRACKEREVENT_SCT_RDORAWDATA_H
#define SITRACKEREVENT_SCT_RDORAWDATA_H

// Base class
#include "InDetRawData/InDetRawData.h"



class SCT_RDORawData :  public InDetRawData{


  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
public:

  // Constructor with parameters:
  // offline hash Id, last, word
  SCT_RDORawData(const Identifier rdoId, const unsigned int word);

  // Destructor:
  virtual ~SCT_RDORawData();

  ///////////////////////////////////////////////////////////////////
  // Virtual methods 
  ///////////////////////////////////////////////////////////////////

  // pure virtual
  virtual int getGroupSize() const = 0;

  // pure virtual
  virtual int getStrip() const = 0;
  ///////////////////////////////////////////////////////////////////
  // Static methods:
  ///////////////////////////////////////////////////////////////////


  // Create a new SCT_RawData and return a pointer to it:
  //  static SCT_RawData *newObject(const Identifier rdoId, const unsigned int word);
  ///////////////////////////////////////////////////////////////////
  // Private methods:
  ///////////////////////////////////////////////////////////////////
private:

  
  SCT_RDORawData();

  ///////////////////////////////////////////////////////////////////
  // Private data:
  ///////////////////////////////////////////////////////////////////
private:


};

///////////////////////////////////////////////////////////////////
// Inline methods:
///////////////////////////////////////////////////////////////////

#endif // SITRACKEREVENT_SCT_RAWDATA_H
