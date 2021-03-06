/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/*****************************************************************************
 * Project: RooFit                                                           *
 *                                                                           *
  * This code was autogenerated by RooClassFactory                            * 
 *****************************************************************************/

#ifndef GENGAUSS3D
#define GENGAUSS3D

#include "RooAbsPdf.h"
#include "RooRealProxy.h"
#include "RooCategoryProxy.h"
#include "RooAbsReal.h"
#include "RooAbsCategory.h"
#include "RooMultiVarGaussian.h"


class GenGauss3D : public RooMultiVarGaussian {
public:
  GenGauss3D() {} ; 
  GenGauss3D(const char *name, const char *title,
	      RooArgList& _xvec,
	      RooArgList& _muvex,
	      RooAbsReal& _vxx,
	      RooAbsReal& _vxy,
	      RooAbsReal& _vxz,
	      RooAbsReal& _vyy,
	      RooAbsReal& _vyz,
	      RooAbsReal& _vzz,
	      TMatrixDSym& dummyMatrix);
  GenGauss3D(const GenGauss3D& other, const char* name=0) ;
  virtual TObject* clone(const char* newname) const { return new GenGauss3D(*this,newname); }
  inline virtual ~GenGauss3D() { }

  Int_t getAnalyticalIntegral(RooArgSet& allVars, RooArgSet& analVars, const char* rangeName=0) const ;
  Double_t analyticalIntegral(Int_t code, const char* rangeName=0) const ;

  void generateEvent(Int_t code);

  void updateCov();

protected:

  RooRealProxy xvec ;
  RooRealProxy muvec ;
  RooRealProxy vxx ;
  RooRealProxy vxy ;
  RooRealProxy vxz ;
  RooRealProxy vyy ;
  RooRealProxy vyz ;
  RooRealProxy vzz ;
  
  Double_t evaluate() const ;

private:

  ClassDef(GenGauss3D,1) // Your description goes here...

  //long nUpdates;
  //long nChanged;
  //double oldVxx;
  //double oldVyy;
  //double oldVxy;
  //double oldVzz;
};
 
#endif
