//---------------------------------------------------------------------------//
//                                                                           //
// This file is part of the Pittsburgh Visualization System (PVS)            //
//                                                                           //
// Copyright (C) 2004 Joe Boudreau, University of Pittsburgh                 //
//                                                                           //
//  This program is free software; you can redistribute it and/or modify     //
//  it under the terms of the GNU General Public License as published by     //
//  the Free Software Foundation; either version 2 of the License, or        //
//  (at your option) any later version.                                      //
//                                                                           //
//  This program is distributed in the hope that it will be useful,          //
//  but WITHOUT ANY WARRANTY; without even the implied warranty of           //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            //
//  GNU General Public License for more details.                             //
//                                                                           //
//  You should have received a copy of the GNU General Public License        //
//  along with this program; if not, write to the Free Software              //
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307     //
// --------------------------------------------------------------------------//


//---------------------------------------------------------------------------//
//                                                                           //
// LogRangeDivider.  This class takes a range of values and divides it       //
// up in a way that is appropriate for log scales.                           //
//                                                                           //
//                                                                           //
//                                                                           //
//---------------------------------------------------------------------------//
#ifndef QATPLOTWIDGETS_LOGRANGEDIVIDER_H
#define QATPLOTWIDGETS_LOGRANGEDIVIDER_H
#include "QatPlotWidgets/AbsRangeDivider.h"
#include <vector>
class LogRangeDivider:public AbsRangeDivider {
  
public:

  // Constructor
  LogRangeDivider();

  // Destructor
  virtual ~LogRangeDivider();

  // Three methods to set the range.  The calling the first method once
  // is more efficient than calling the last methods twice!
  virtual void setRange(double min, double max);
  virtual void setMin(double min);
  virtual void setMax(double max);

  // Get the number of subdivisions:
  virtual int getNumSubdivisions() const;

  // Get the location of each subdivision:
  virtual const RangeDivision & getSubdivision(int i) const;

 // Get the validity of each subdivision:
  virtual bool isValid(int i) const;

private:

  double                     m_min;
  double                     m_max;
  std::vector<RangeDivision> m_subdivision;
  void                       recompute();
  
};
#endif
