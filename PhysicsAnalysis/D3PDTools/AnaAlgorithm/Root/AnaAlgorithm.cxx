//        Copyright Iowa State University 2017.
//                  Author: Nils Krumnack
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// Please feel free to contact me (nils.erik.krumnack@cern.ch) for bug
// reports, feature suggestions, praise and complaints.


//
// includes
//

#include <AnaAlgorithm/AnaAlgorithm.h>

#include <AsgTools/MessageCheck.h>
#include <RootCoreUtils/Assert.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>

#ifdef ROOTCORE
#include <AnaAlgorithm/IFilterWorker.h>
#include <AnaAlgorithm/IHistogramWorker.h>
#include <AnaAlgorithm/ITreeWorker.h>
#endif

//
// method implementations
//

namespace EL
{
  AnaAlgorithm ::
  AnaAlgorithm (const std::string& name, 
                ISvcLocator*
#ifndef ROOTCORE
                pSvcLocator
#endif
                )
#ifdef ROOTCORE
    : AsgMessaging (name)
    , m_name (name)
    , m_properties (new PropertyMgr)
#else
    : AthHistogramAlgorithm (name, pSvcLocator)
#endif
  {
#ifdef ROOTCORE
    msg().declarePropertyFor (*this);
    declareProperty ("RootStreamName", m_treeStreamName = "ANALYSIS",
                     "Name of the stream to put trees into");
#endif

    ANA_MSG_DEBUG ("AnaAlgorithm: " << name);
  }



  AnaAlgorithm ::
  ~AnaAlgorithm () noexcept
  {}



#ifdef ROOTCORE
  asg::SgTEvent *AnaAlgorithm ::
  evtStore () const
  {
    if (!m_evtStore)
      throw std::logic_error ("no evtStore set on algorithm " + name());
    return m_evtStore;
  }



  ::StatusCode AnaAlgorithm ::
  book (const TH1& hist)
  {
    histogramWorker()->addOutput (hist.Clone());
    return ::StatusCode::SUCCESS;
  }



  TH1 *AnaAlgorithm ::
  hist (const std::string& name) const
  {
    return histogramWorker()->getOutputHist (name);
  }



  TH2 *AnaAlgorithm ::
  hist2d (const std::string& name) const
  {
    TH2 *hist = dynamic_cast<TH2*>(histogramWorker()->getOutputHist (name));
    if (hist == nullptr)
      throw std::runtime_error ("histogram not a 2d-histogram: " + name);
    return hist;
  }



  TH3 *AnaAlgorithm ::
  hist3d (const std::string& name) const
  {
    TH3 *hist = dynamic_cast<TH3*>(histogramWorker()->getOutputHist (name));
    if (hist == nullptr)
      throw std::runtime_error ("histogram not a 3d-histogram: " + name);
    return hist;
  }



  IHistogramWorker *AnaAlgorithm ::
  histogramWorker () const
  {
    if (!m_histogramWorker)
      throw std::logic_error ("no histogram worker set on algorithm " + name());
    return m_histogramWorker;
  }



  ::StatusCode AnaAlgorithm ::
  book (const TTree& tree)
  {
     ANA_CHECK_SET_TYPE( ::StatusCode );
     ANA_CHECK( treeWorker()->addTree( tree, m_treeStreamName ) );
     return ::StatusCode::SUCCESS;
  }



  TTree *AnaAlgorithm ::
  tree (const std::string& name) const
  {
     return treeWorker()->getOutputTree( name, m_treeStreamName );
  }



  ITreeWorker *AnaAlgorithm ::
  treeWorker () const
  {
     if( ! m_treeWorker ) {
        throw std::logic_error( "no tree worker set on algorithm " + name() );
     }
     return m_treeWorker;
  }



  bool AnaAlgorithm ::
  filterPassed() const
  {
    return filterWorker()->filterPassed();
  }



  void AnaAlgorithm ::
  setFilterPassed (bool val_filterPassed)
  {
    filterWorker()->setFilterPassed (val_filterPassed);
  }



  IFilterWorker *AnaAlgorithm ::
  filterWorker () const
  {
    if (!m_filterWorker)
      throw std::logic_error ("no filter worker set on algorithm " + name());
    return m_filterWorker;
  }



  Worker *AnaAlgorithm ::
  wk () const
  {
    if (!m_wk)
      throw std::logic_error ("no worker set on algorithm " + name());
    return m_wk;
  }
#endif



  ::StatusCode AnaAlgorithm ::
  initialize ()
  {
    return StatusCode::SUCCESS;
  }



  ::StatusCode AnaAlgorithm ::
  execute ()
  {
    return StatusCode::SUCCESS;
  }



  ::StatusCode AnaAlgorithm ::
  finalize ()
  {
    return StatusCode::SUCCESS;
  }



  void AnaAlgorithm ::
  print () const
  {}



#ifdef ROOTCORE
  ::StatusCode AnaAlgorithm ::
  sysInitialize ()
  {
    return initialize ();
  }



  ::StatusCode AnaAlgorithm ::
  sysExecute ()
  {
    return execute ();
  }



  ::StatusCode AnaAlgorithm ::
  sysFinalize ()
  {
    return finalize ();
  }



  void AnaAlgorithm ::
  sysPrint ()
  {
    print ();
  }



  void AnaAlgorithm ::
  setEvtStore (asg::SgTEvent *val_evtStore)
  {
    if (m_evtStore)
      throw std::logic_error ("set evtStore twice on algorithm " + name());
    m_evtStore = val_evtStore;
  }



  void AnaAlgorithm ::
  setHistogramWorker (IHistogramWorker *val_histogramWorker)
  {
    if (m_histogramWorker)
      throw std::logic_error ("set histogram worker twice on algorithm " + name());
    m_histogramWorker = val_histogramWorker;
  }



  void AnaAlgorithm ::
  setTreeWorker (ITreeWorker *val_treeWorker)
  {
     if( m_treeWorker ) {
        throw std::logic_error( "set tree worker twice on algorithm " +
                                name() );
     }
     m_treeWorker = val_treeWorker;
  }



  void AnaAlgorithm ::
  setFilterWorker (IFilterWorker *val_filterWorker)
  {
    if (m_filterWorker)
      throw std::logic_error ("set filter worker twice on algorithm " + name());
    m_filterWorker = val_filterWorker;
  }



  void AnaAlgorithm ::
  setWk (Worker *val_wk)
  {
    if (m_wk)
      throw std::logic_error ("set wk twice on algorithm " + name());
    m_wk = val_wk;
  }



  void AnaAlgorithm ::
  addCleanup (const std::shared_ptr<void>& cleanup)
  {
    m_cleanup.push_back (cleanup);
  }



  const std::string& AnaAlgorithm ::
  name () const
  {
    return m_name;
  }
#endif
}
