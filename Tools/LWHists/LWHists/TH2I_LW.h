/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Header file for class TH2I_LW                             //
//                                                            //
//  Description: LightWeight version of TH2I.                 //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: March 2009                               //
//                                                            //
////////////////////////////////////////////////////////////////

#ifndef TH2I_LW_H
#define TH2I_LW_H

#include "LWHists/LWHist2D.h"
class TH2I;

class TH2I_LW : public LWHist2D {
public:
  typedef int bin_type_t;

  //To allocate from pool - remember to delete with LWHist::safeDelete(..):
  static TH2I_LW * create( const char* name, const char* title,
			   unsigned nbinsx, const double& xmin, const double& xmax,
			   unsigned nbinsy, const double& ymin, const double& ymax );
  static TH2I_LW * create( const char* name, const char* title,
			   unsigned nbinsx, const double* xbins,
			   unsigned nbinsy, const double& ymin, const double& ymax );
  static TH2I_LW * create( const char* name, const char* title,
			   unsigned nbinsx, const double& xmin, const double& xmax,
			   unsigned nbinsy, const double* ybins );
  static TH2I_LW * create( const char* name, const char* title,
			   unsigned nbinsx, const double* xbins,
			   unsigned nbinsy, const double* ybins );
  static TH2I_LW * create( const char* name, const char* title,
			   unsigned nbinsx, const float* xbins,
			   unsigned nbinsy, const float* ybins );

  void Fill(const double& x, const double& y);
  void Fill(const double& x, const double& y, const double& w);
  unsigned GetNbinsX() const;
  unsigned GetNbinsY() const;
  double GetBinContent(unsigned binx, unsigned biny) const;
  double GetBinError(unsigned binx, unsigned biny) const;
  void SetBinContent(unsigned binx, unsigned biny, const double& );
  void SetBinError(unsigned binx, unsigned biny, const double& );
  unsigned GetEntries() const;
  void SetEntries(unsigned);
  void SetBins(unsigned nbinsx,double xmin,double xmax,
	       unsigned nbinsy,double ymin,double ymax);

  void GetBinContentAndError(unsigned binx, unsigned biny,double& content, double&error) const;
  void SetBinContentAndError(unsigned binx, unsigned biny,const double& content, const double& error);
  double getXMin() const;
  double getXMax() const;
  double getYMin() const;
  double getYMax() const;

  void Reset();

  void getSums( double& sumW, double& sumW2,
		double& sumWX,double& sumWX2,
		double& sumWY, double& sumWY2,
		double& sumWXY) const;
  void setSums( const double& sumW, const double&sumW2,
		const double& sumWX, const double& sumWX2,
		const double& sumWY, const double& sumWY2,
		const double& sumWXY );

  TH2I* getROOTHist();
  TH1* getROOTHistBase();

  double Integral() const;

  //For fast looping, skipping bins where (content,error)==(0,0):
  void resetActiveBinLoop();
  bool getNextActiveBin(unsigned& binx, unsigned& biny, double& content, double& error);

  void scaleContentsAndErrors( const double& fact );//C.f. comment in LWHist1D.h

private:
  friend class LWHistInt;
  friend class LWHistVal;
  void clear();
  double getBinCenterX(int bin) const;
  double getBinCenterY(int bin) const;
  TH1* getROOTHistBaseNoAlloc() const;
  void clearKeptROOTHist();//Does nothing if root-backend.
  const float * getVarBinsX() const;//null if fixed bin-widths
  const float * getVarBinsY() const;//null if fixed bin-widths
  virtual double actualGetBinCenterX(int bin) const;
  virtual double actualGetBinCenterY(int bin) const;
  virtual unsigned actualFindBinX(const double&) const;
  virtual unsigned actualFindBinY(const double&) const;
  TH2I_LW( const char* name, const char* title,
	   unsigned nbinsx, const double& xmin, const double& xmax,
	   unsigned nbinsy, const double& ymin, const double& ymax, bool rootbackend );
  TH2I_LW( const char* name, const char* title,
	   unsigned nbinsx, const double* xbins,
	   unsigned nbinsy, const double& ymin, const double& ymax, bool rootbackend );
  TH2I_LW( const char* name, const char* title,
	   unsigned nbinsx, const double& xmin, const double& xmax,
	   unsigned nbinsy, const double* ybins, bool rootbackend );
  TH2I_LW( const char* name, const char* title,
	   unsigned nbinsx, const double* xbins,
	   unsigned nbinsy, const double* ybins, bool rootbackend );
  TH2I_LW( const char* name, const char* title,
	   unsigned nbinsx, const float* xbins,
	   unsigned nbinsy, const float* ybins, bool rootbackend );
  virtual ~TH2I_LW();
  TH2I_LW( const TH2I_LW & );
  TH2I_LW & operator= ( const TH2I_LW & );
  void * m_flexHisto;
  TH2I * m_rootHisto;
  unsigned m_rootbackend_fastloopbin;
  bool m_ownsRootSumw2;

};

#endif
