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


#ifndef _PLOTRECT_H_
#define _PLOTRECT_H_
#include "QatPlotting/Plotable.h"
#include "QatPlotting/PlotRectProperties.h"
#include <QtCore/QRectF>
#include <memory>

class PlotRect: public Plotable {

public:

  // Define Properties:
  typedef PlotRectProperties Properties;

   // Constructors
  PlotRect(const QRectF & rect); 

  // Destructor
  virtual ~PlotRect();

  // Get the "natural rectangular border"
  virtual const QRectF  rectHint() const;

  // Describe to plotter, in terms of primitives:
  virtual void describeYourselfTo(AbsPlotter *plotter) const;

   // Set the properties
  void setProperties(const Properties &properties);

  // Revert to default properties:
  void resetProperties();

  // Get the properties (either default, or specific)
  const Properties properties () const;
  
  // Copy constructor:
  PlotRect(const PlotRect &);
  
  // Assignment operator:
  PlotRect & operator=(const PlotRect &); 
  
private:

  class Clockwork;
  std::unique_ptr<Clockwork> c;

};
#endif
