/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef _ArrayFunction_h_
#define _ArrayFunction_h_
//-------------------------------------------------------//
// This one dimensional function takes its values from   //
// an array..which it copies in.                         //
//-------------------------------------------------------//


#include "GeoGenericFunctions/AbsFunction.h"
#include <vector>
namespace GeoGenfun {
class ArrayFunction : public AbsFunction  {
  
  FUNCTION_OBJECT_DEF(ArrayFunction)
    
    public:
  
  // Constructor
  ArrayFunction(const double *begin, const double *end);
  
  // Destructor
  virtual ~ArrayFunction();
  
  // Copy constructor
  ArrayFunction(const ArrayFunction &right);
  
  // Retreive function value
  virtual double operator ()(double argument) const;
  virtual double operator ()(const Argument & a) const {return operator() (a[0]);}
  
 private:
  
  // It is illegal to assign a ArrayFunction
  const ArrayFunction & operator=(const ArrayFunction &right);

  std::vector<double> m_values;
};
}
#endif
