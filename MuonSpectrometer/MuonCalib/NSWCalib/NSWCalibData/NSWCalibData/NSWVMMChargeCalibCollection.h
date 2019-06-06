/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/


#ifndef NSWVMMCHARGECALIBCOLLECTION_H
#define NSWVMMCHARGECALIBCOLLECTION_H

// Base classes
#include "Identifier/Identifiable.h"
#include "Identifier/IdentifierHash.h"
#include "AthContainers/DataVector.h"


// Needed class
//#include "TrkPrepRawData/PrepRawData.h"

namespace Muon{

/**@brief Template to hold collections of MuonPrepRawData objects. 
The collections must each have an IdentifierHash, which are used to identify them*/
//template< class PrepDataT >
class NSWVMMChargeCalibCollection : public Identifiable,
                 public DataVector< VMMChargePar > {

  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
public:

  // Constructor with parameters:
  //   Hashed offline identifier of the DE
  NSWVMMChargeCalibCollection(const IdentifierHash idHash);

 /** Default Constructor (for persistency)*/
  NSWVMMChargeCalibCollection();
                     
  /** Destructor:*/
  virtual ~NSWVMMChargeCalibCollection();


  ///////////////////////////////////////////////////////////////////
  // Const methods:
  ///////////////////////////////////////////////////////////////////

  // typedef needed for IdentifiableContainer base class
  typedef Identifier ID;

  // identifier of this detector element:
  virtual Identifier identify() const;

  virtual IdentifierHash identifyHash() const;

  virtual void setIdentifier(Identifier id);
  // plottable
  virtual std::string type() const;

  //MsgStream& operator << ( MsgStream& sl, const NSWVMMChargeCalibCollection& coll);
  //std::ostream& operator << ( std::ostream& sl, const NSWVMMChargeCalibCollection& coll);

    
  // typedef needed for IdentifiableContainer base class
//  typedef PrepRawData DIGIT;

  ///////////////////////////////////////////////////////////////////
  // Private methods:
  ///////////////////////////////////////////////////////////////////
private:

  NSWVMMChargeCalibCollection(const NSWVMMChargeCalibCollection&);
  NSWVMMChargeCalibCollection &operator=(const NSWVMMChargeCalibCollection&);

  ///////////////////////////////////////////////////////////////////
  // Private data:
  ///////////////////////////////////////////////////////////////////
private:
  const IdentifierHash m_idHash; 
  Identifier m_id; // identifier of the DE
  

};
// member functions that use Collection T
//#include"MuonPrepRawData/MuonPrepDataCollection.icc"
///////////////////////////////////////////////////////////////////
// Inline methods:
///////////////////////////////////////////////////////////////////


CLASS_DEF(NSWVMMChargeCalibCollection,1136237878 , 1)


}

#endif 
