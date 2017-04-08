/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HistogramFiller_h
#define HistogramFiller_h

#include <functional>
#include <mutex>
#include <memory>
#include <vector>

#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
#include "TProfile2D.h"

#include "AthenaMonitoring/IMonitoredVariable.h"
#include "AthenaMonitoring/HistogramDef.h"
 
/**
 * @brief base class for fillers 
 */
class HistogramFiller {
public:
  HistogramFiller(TH1* hist, HistogramDef histDef) 
    : m_hist(hist), m_mutex(std::make_shared<std::mutex>()), m_histDef(new HistogramDef(histDef)) {}
  HistogramFiller(const HistogramFiller& hf) 
    : m_hist(hf.m_hist), m_mutex(hf.m_mutex), m_histDef(hf.m_histDef) {}
  HistogramFiller(HistogramFiller&&) = default;

  virtual ~HistogramFiller() {}
  virtual unsigned fill() = 0;
  virtual HistogramFiller* clone() = 0;

  void setMonitoredVariables(std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>> monitoredVariables) {
    m_monVariables = monitoredVariables;
  }

  std::vector<std::string> histogramVariablesNames() {
    return m_histDef->name;
  }
protected:
  virtual TH1* histogram() = 0;

  TH1* m_hist;
  std::shared_ptr<std::mutex> m_mutex;
  std::shared_ptr<HistogramDef> m_histDef;
  std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>> m_monVariables;
private:
  HistogramFiller& operator=(HistogramFiller const&) = delete;
};

/**
 * @brief filler for plain 1D hisograms 
 */
class HistogramFiller1D : public HistogramFiller {
public: 
  HistogramFiller1D(TH1* hist, HistogramDef histDef)
    : HistogramFiller(hist, histDef) {}
  virtual unsigned fill();
  HistogramFiller1D* clone() override { return new HistogramFiller1D(*this); };
protected:
  virtual TH1* histogram() { return m_hist; }
};

/**
 * @brief filler for 1D hisograms filled in cummulative mode
 */
class CumulativeHistogramFiller1D : public HistogramFiller1D {
public:
  CumulativeHistogramFiller1D(TH1* hist, HistogramDef histDef) 
    : HistogramFiller1D(hist, histDef) {}
  virtual unsigned fill();
  virtual CumulativeHistogramFiller1D* clone() override { return new CumulativeHistogramFiller1D(*this); };
};

class VecHistogramFiller1D : public HistogramFiller1D {
public:
  VecHistogramFiller1D(TH1* hist, HistogramDef histDef) 
    : HistogramFiller1D(hist, histDef) {}
  virtual unsigned fill();
  virtual VecHistogramFiller1D* clone() override { return new VecHistogramFiller1D(*this); };
};

class VecHistogramFiller1DWithOverflows : public HistogramFiller1D {
public:
  VecHistogramFiller1DWithOverflows(TH1* hist, HistogramDef histDef) 
    : HistogramFiller1D(hist, histDef) {}
  virtual unsigned fill();
  virtual VecHistogramFiller1DWithOverflows* clone() override { return new VecHistogramFiller1DWithOverflows(*this); };
};

/**
 * @brief filler for profile 1d histogram 
 */
class HistogramFillerProfile : public HistogramFiller {
public:
  HistogramFillerProfile(TProfile* hist, HistogramDef histDef)
    : HistogramFiller(hist, histDef) {};
  virtual unsigned fill();
  virtual HistogramFillerProfile* clone() override { return new HistogramFillerProfile(*this); };
protected:
  virtual TProfile* histogram() { return static_cast<TProfile*>(m_hist); }
};

/**
 * @brief filler for plain 2D hisograms 
 */
class HistogramFiller2D : public HistogramFiller {
public:
  HistogramFiller2D(TH2* hist, HistogramDef histDef)
    : HistogramFiller(hist, histDef) {};
  virtual unsigned fill();
  virtual HistogramFiller2D* clone() override { return new HistogramFiller2D(*this); };
protected:
  virtual TH2* histogram() { return static_cast<TH2*>(m_hist); }
};

/**
 * @brief filler for profile 2D histogram
 */
class HistogramFiller2DProfile : public HistogramFiller {
public:
  HistogramFiller2DProfile(TProfile2D* hist, HistogramDef histDef)
    : HistogramFiller(hist, histDef) {};
  virtual unsigned fill();
  virtual HistogramFiller2DProfile* clone() override { return new HistogramFiller2DProfile(*this); };
protected:
  virtual TProfile2D* histogram() { return static_cast<TProfile2D*>(m_hist); }
};  
  
#endif /* HistogramFiller */