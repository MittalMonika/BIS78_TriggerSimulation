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


//----------------------------------------------------------------------------------//
//                                                                                  //
// Plottable adaptor for Hist2D's                                                   //
//                                                                                  //
//----------------------------------------------------------------------------------//
#ifndef _PLOTHIST2D_H_
#define _PLOTHIST2D_H_
#include "QatPlotting/Plotable.h"
#include "QatPlotting/PlotHist2DProperties.h"
#include <QtCore/QRectF>
#include <vector>
class Hist2D;
class PlotHist2D: public Plotable {

public:
  
  
  // Define properties:
  typedef PlotHist2DProperties Properties;

  // Constructor
  PlotHist2D(const Hist2D &);

  // Copy constructor:
  PlotHist2D(const PlotHist2D &);

  // Assignment operator:
  PlotHist2D & operator=(const PlotHist2D &); 

  // Destructor
  virtual ~PlotHist2D();

  // Get the "natural rectangular border"
  virtual const QRectF & rectHint() const;

  // Describe to plotter, in terms of primitives:
  virtual void describeYourselfTo(AbsPlotter *plotter) const;

  // Get the histogram:
  const Hist2D *histogram() const;

  // Set the properties
  void setProperties(const Properties &properties);

  // Revert to default properties:
  void resetProperties();

  // Get the properties (either default, or specific)
  const Properties &properties () const;

private:

  class Clockwork;
  Clockwork *c;
  
};
#endif
