/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ICaloGeometry_h
#define ICaloGeometry_h

#include "Identifier/Identifier.h"
#include "ISF_FastCaloSimEvent/FastCaloSim_CaloCell_ID.h"
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "ISF_FastCaloSimParametrization/CaloGeoDetDescrElement.h"

class CaloDetDescrElement;
class ICaloGeometry {
public :
   virtual bool PostProcessGeometry() = 0;

   virtual void Validate() = 0;

   virtual const CaloDetDescrElement* getDDE(Identifier identify) = 0;
   virtual const CaloDetDescrElement* getDDE(int sampling,float eta,float phi,float* distance=0,int* steps=0) = 0;
//   virtual const CaloGeoDetDescrElement* getDDE(Identifier identify) = 0;
//   virtual const CaloGeoDetDescrElement* getDDE(int sampling,float eta,float phi,float* distance=0,int* steps=0) = 0;
   
   virtual double deta(int sample,double eta) const = 0;
   virtual void   minmaxeta(int sample,double eta,double& mineta,double& maxeta) const = 0;
   virtual double rzmid(int sample,double eta) const = 0;
   virtual double rzent(int sample,double eta) const = 0;
   virtual double rzext(int sample,double eta) const = 0;
   virtual double rmid(int sample,double eta) const = 0;
   virtual double rent(int sample,double eta) const = 0;
   virtual double rext(int sample,double eta) const = 0;
   virtual double zmid(int sample,double eta) const = 0;
   virtual double zent(int sample,double eta) const = 0;
   virtual double zext(int sample,double eta) const = 0;
   virtual double rpos(int sample,double eta,int subpos=CaloSubPos::SUBPOS_MID) const = 0;
   virtual double zpos(int sample,double eta,int subpos=CaloSubPos::SUBPOS_MID) const = 0;
   virtual double rzpos(int sample,double eta,int subpos=CaloSubPos::SUBPOS_MID) const = 0;
   virtual bool   isCaloBarrel(int sample) const = 0;
};

#endif


