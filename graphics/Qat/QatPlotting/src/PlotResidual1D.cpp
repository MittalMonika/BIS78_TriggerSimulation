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


#include "QatDataAnalysis/Hist1D.h"
#include "QatPlotting/PlotResidual1D.h"
#include "QatPlotting/AbsPlotter.h"
#include "QatPlotting/LinToLog.h"
#include <QtGui/QGraphicsRectItem>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsRectItem>
#include <QtGui/QGraphicsEllipseItem>
#include <QtGui/QGraphicsPolygonItem>
#include <QtGui/QGraphicsRectItem>
#include <QtGui/QGraphicsItemGroup>
#include <QtGui/QPainterPath>
#include <QtGui/QGraphicsPathItem>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cfloat>
// Constructor

class PlotResidual1D::Clockwork {
public:
  //Clockwork doesn't own the histogram pointer
  Clockwork() : histogram(nullptr), function(nullptr), nRectangle(),
    myProperties(nullptr), defaultProperties() {}

  ~Clockwork() {
    delete myProperties; 
    delete function; 
  }
  
  //copy
  Clockwork(const Clockwork & other){
    histogram = other.histogram; // same histogram is shared
    function = other.function->clone();
    nRectangle = other.nRectangle;
    myProperties = (other.myProperties)?new Properties(*(other.myProperties)):nullptr;
    defaultProperties=other.defaultProperties;
  }
  
  //assign
  Clockwork & operator=(const Clockwork & other){
    if (&other != this){
      histogram = other.histogram;
      delete function;
      function = other.function->clone();
      nRectangle = other.nRectangle;
      delete myProperties;
      myProperties = (other.myProperties)?new Properties(*(other.myProperties)):nullptr;
      defaultProperties=other.defaultProperties;
    }
    return *this;
  }
    
  // This is state:
  const Hist1D                          *histogram;         // The histogram
  const Genfun::AbsFunction             *function;          // The function
  QRectF                                 nRectangle;        // The "natural" bounding rectangle
  Properties                            *myProperties;      // My properties.  May 
  Properties                             defaultProperties;
};


PlotResidual1D::PlotResidual1D(const Hist1D & histogram, Genfun::GENFUNCTION function):
  Plotable(),m_c(new Clockwork())
{
  m_c->histogram=&histogram;
  m_c->function = function.clone();
  double yMin=FLT_MAX;
  double yMax=FLT_MIN;
  for (unsigned int i=0;i<histogram.nBins();i++) {
    double x      = histogram.binCenter(i);
    double yplus  = histogram.bin(i) -(*m_c->function)(x)+ histogram.binError(i);
    double yminus = histogram.bin(i) -(*m_c->function)(x)- histogram.binError(i);
    yMin = std::min(yMin,yminus);
    yMax = std::max(yMax,yplus);
  }   

  double dist = yMax-yMin;
  double marge = dist/10;
  yMax += marge;
  if (yMin>0) {
    yMin=0;
  }
  else if (yMin<0) {
    yMin-=marge;
  }

  QPointF lowerRight(histogram.min(),yMin);
  QPointF upperLeft (histogram.max(),yMax);
  m_c->nRectangle.setLeft  (histogram.min());
  m_c->nRectangle.setRight(histogram.max());
  m_c->nRectangle.setTop(yMin);
  m_c->nRectangle.setBottom(yMax);
}

// Copy constructor:
PlotResidual1D::PlotResidual1D(const PlotResidual1D & source):
  Plotable(),m_c(new Clockwork(*(source.m_c)))
{
 
}

// Assignment operator:
PlotResidual1D & PlotResidual1D::operator=(const PlotResidual1D & source)
{
  if (&source!=this) {
    Plotable::operator=(source);
    m_c.reset(new Clockwork(*(source.m_c)));
  }
  return *this;
} 


// Destructor
PlotResidual1D::~PlotResidual1D(){
  //delete m_c;
}


const QRectF  PlotResidual1D::rectHint() const {
  return m_c->nRectangle;
}


// Get the graphic description::
void PlotResidual1D::describeYourselfTo(AbsPlotter *plotter) const {
  QPen pen =properties().pen;
  int symbolSize=properties().symbolSize;
  PlotResidual1D::Properties::SymbolStyle symbolStyle=properties().symbolStyle;
  
  LinToLog *toLogX= plotter->isLogX() ? new LinToLog (plotter->rect()->left(),plotter->rect()->right()) : NULL;
  LinToLog *toLogY= plotter->isLogY() ? new LinToLog (plotter->rect()->top(),plotter->rect()->bottom()) : NULL;

  QMatrix m=plotter->matrix(),mInverse=m.inverted();

  for (unsigned int i=0;i<m_c->histogram->nBins();i++) {
    double x = plotter->isLogX() ? (*toLogX) (m_c->histogram->binCenter(i)) : m_c->histogram->binCenter(i);
    double y = plotter->isLogY() ? (*toLogY) (m_c->histogram->bin(i)-(*m_c->function)(x)) : m_c->histogram->bin(i)-(*m_c->function)(x);
    double  ydyp = plotter->isLogY() ? (*toLogY)(m_c->histogram->bin(i)-(*m_c->function)(x)+m_c->histogram->binError(i)) : m_c->histogram->bin(i)-(*m_c->function)(x)+m_c->histogram->binError(i);
    double  ydym = plotter->isLogY() ? (*toLogY)(m_c->histogram->bin(i)-(*m_c->function)(x)-m_c->histogram->binError(i)) : m_c->histogram->bin(i)-(*m_c->function)(x)-m_c->histogram->binError(i);
    
    QPointF loc(x, y );
    QSizeF  siz(symbolSize,symbolSize);
    QPointF ctr(siz.width()*0.5, siz.height()*0.5);
    QPointF had(siz.width()*0.5, 0);
    QPointF vad(0,siz.height()*0.5);
    
    QPointF plus(x,ydyp);
    QPointF mnus(x,ydym);
    
    if (plotter->rect()->contains(loc)) {
      QAbstractGraphicsShapeItem *shape=NULL;
      if (symbolStyle==PlotResidual1D::Properties::CIRCLE) {
  shape = new QGraphicsEllipseItem(QRectF(m.map(loc)-ctr,siz));
      }
      else if (symbolStyle==PlotResidual1D::Properties::SQUARE) {
  shape = new QGraphicsRectItem(QRectF(m.map(loc)-ctr,siz));
      }
      else if (symbolStyle==PlotResidual1D::Properties::TRIANGLE_U) {
				QVector<QPointF> points;
				points.push_back(m.map(loc)-ctr+(QPointF(0,symbolSize)));
				points.push_back(m.map(loc)-ctr+(QPointF(symbolSize,symbolSize)));
				points.push_back(m.map(loc)-ctr+(QPointF(symbolSize*0.5,0)));
				points.push_back(m.map(loc)-ctr+(QPointF(0,symbolSize)));
				shape = new QGraphicsPolygonItem(QPolygonF(points));
      }
      else if (symbolStyle==PlotResidual1D::Properties::TRIANGLE_L) {
				QVector<QPointF> points;
				points.push_back(m.map(loc)-ctr+(QPointF(0,            0)));
				points.push_back(m.map(loc)-ctr+(QPointF(symbolSize,   0)));
				points.push_back(m.map(loc)-ctr+(QPointF(symbolSize*0.5, symbolSize)));
				points.push_back(m.map(loc)-ctr+(QPointF(0,            0)));
				shape = new QGraphicsPolygonItem(QPolygonF(points));
      }
      else {
       throw std::runtime_error("In PlotResidual1D:  unknown plot symbol");
      }
      
      shape->setPen(pen);
      shape->setMatrix(mInverse);
      plotter->scene()->addItem(shape);
      plotter->group()->addToGroup(shape);
      
      {
  QLineF  pLine(m.map(loc)-vad, m.map(plus));
  if (plotter->rect()->contains(plus)) {
    QGraphicsLineItem *line=new QGraphicsLineItem(pLine);
    line->setPen(pen);
    line->setMatrix(mInverse);
    plotter->scene()->addItem(line);
    plotter->group()->addToGroup(line);
    
    QGraphicsLineItem *topLine=new QGraphicsLineItem(QLineF(m.map(plus)+had,m.map(plus)-had));
    topLine->setPen(pen);
    topLine->setMatrix(mInverse);
    plotter->scene()->addItem(topLine);
    plotter->group()->addToGroup(topLine);
    
  }
  else {
    QPointF intersection;
    QLineF bottomLine(plotter->rect()->bottomLeft(),plotter->rect()->bottomRight());
    QLineF::IntersectType type=bottomLine.intersect(QLineF(loc,plus),&intersection);
    if (type==QLineF::BoundedIntersection) {
      QPointF plus=intersection;
      QLineF  pLine(m.map(loc)-vad, m.map(plus));
      QGraphicsLineItem *line=new QGraphicsLineItem(pLine);
      line->setPen(pen);
      line->setMatrix(mInverse);
      plotter->scene()->addItem(line);
      plotter->group()->addToGroup(line);
    }
  }
      }
      
      {
  QLineF  mLine(m.map(loc)+vad, m.map(mnus));
  if (plotter->rect()->contains(mnus)) {
    QGraphicsLineItem *line=new QGraphicsLineItem(mLine);
    line->setPen(pen);
    line->setMatrix(mInverse);
    plotter->scene()->addItem(line);
    plotter->group()->addToGroup(line);
    
    QGraphicsLineItem *bottomLine=new QGraphicsLineItem(QLineF(m.map(mnus)+had,m.map(mnus)-had));
    bottomLine->setPen(pen);
    bottomLine->setMatrix(mInverse);
    plotter->scene()->addItem(bottomLine);
    plotter->group()->addToGroup(bottomLine);
    
  }
  else {
    QPointF intersection;
    QLineF topLine(plotter->rect()->topLeft(),plotter->rect()->topRight());
    QLineF::IntersectType type=topLine.intersect(QLineF(loc,mnus),&intersection);
    if (type==QLineF::BoundedIntersection) {
      QPointF mnus=intersection;
      QLineF  mLine(m.map(loc)+vad, m.map(mnus));
      QGraphicsLineItem *line=new QGraphicsLineItem(mLine);
      line->setPen(pen);
      line->setMatrix(mInverse);
      plotter->scene()->addItem(line);
      plotter->group()->addToGroup(line);
    }
  }
      }
      
      
    }
    else if (plotter->rect()->contains(mnus)) {
      QPointF intersection;
      QLineF bottomLine(plotter->rect()->bottomLeft(),plotter->rect()->bottomRight());
      QLineF::IntersectType type=bottomLine.intersect(QLineF(loc,mnus),&intersection);
      if (type==QLineF::BoundedIntersection) {
        QPointF loc=intersection;
        QLineF  mLine(m.map(loc), m.map(mnus));
        QGraphicsLineItem *line=new QGraphicsLineItem(mLine);
        line->setPen(pen);
        line->setMatrix(mInverse);
        plotter->scene()->addItem(line);
        plotter->group()->addToGroup(line);
  
        QGraphicsLineItem *bottomLine=new QGraphicsLineItem(QLineF(m.map(mnus)+had,m.map(mnus)-had));
        bottomLine->setPen(pen);
        bottomLine->setMatrix(mInverse);
        plotter->scene()->addItem(bottomLine);
        plotter->group()->addToGroup(bottomLine);
      }
    }
    else if (plotter->rect()->contains(plus)) {
      QPointF intersection;
      QLineF topLine(plotter->rect()->topLeft(),plotter->rect()->topRight());
      QLineF::IntersectType type=topLine.intersect(QLineF(loc,plus),&intersection);
      if (type==QLineF::BoundedIntersection) {
        QPointF loc=intersection;
        QLineF  pLine(m.map(loc), m.map(plus));
        QGraphicsLineItem *line=new QGraphicsLineItem(pLine);
        line->setPen(pen);
        line->setMatrix(mInverse);
        plotter->scene()->addItem(line);
        plotter->group()->addToGroup(line);
  
        QGraphicsLineItem *topLine=new QGraphicsLineItem(QLineF(m.map(plus)+had,m.map(plus)-had));
        topLine->setPen(pen);
        topLine->setMatrix(mInverse);
        plotter->scene()->addItem(topLine);
        plotter->group()->addToGroup(topLine);
      }
    }
  }
  delete toLogX;
  delete toLogY;
}

std::string PlotResidual1D::title() const {
  return m_c->histogram->name();
}

// Get the histogram:
const Hist1D *PlotResidual1D::histogram() const {
  return m_c->histogram;
}

// Get the function:
const Genfun::AbsFunction *PlotResidual1D::function() const {
  return m_c->function->clone(); //cloned because the raw pointer is owned by this class
}

const PlotResidual1D::Properties  PlotResidual1D::properties() const { 
  return m_c->myProperties ? *m_c->myProperties : m_c->defaultProperties;
}

void PlotResidual1D::setProperties(const Properties &  properties) { 
  delete m_c->myProperties;
  m_c->myProperties = new Properties(properties);
}

void PlotResidual1D::resetProperties() {
  delete m_c->myProperties;
  m_c->myProperties=nullptr;
}

