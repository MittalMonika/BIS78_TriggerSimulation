
/**
   @namespace Monitored
   @brief athenaMT monitoring infrastructure
   
   The Monitored namespace collects the infrastructure to create histograms from
   quantitities within an athena component. While this infrastrcucture has been
   created specifically for the use in athenaMT it is of course also usable in
   single-threaded athena. To make use of this infrastructure the following
   steps are needed:
   
   1) Add a GenericMonitoringTool instance to your component
   \code
    private:
      ToolHandle<GenericMonitoringTool> m_monTool{this,"MonTool","","Monitoring tool"};
   \endcode
   Note that by default we are using an empty ToolHandle, i.e. there is no
   monitoring tool attached by default. This should instead be done in the
   python configuration of the component:
   \code
      from AthenaMonitoring.GenericMonitoringTool import GenericMonitoringTool
      myalg.MonTool = GenericMonitoringTool('MonTool')
   \endcode

   2) Retrieve the monitoring tool during `initialize()` **if the ToolHandle is not empty**:
   \code
      if (!m_monTool.empty()) CHECK(m_monTool.retrieve());
   \endcode
   The additional check is needed because retrieval of an emtpy ToolHandle results in a failure.

   3) Declare the monitored quantities to the monitoring framework.
   Several classes are available to export different types to the monitoring framwork: 
     - Monitored::MonitoredScalar
     - Monitored::MonitoredCollection
     - Monitored::MonitoredTimer

  The declaration in all cases is done via the `declare` method in the relevant namespace.
  For example to declare a simple scalar, use:
   \code
   Monitored::MonitoredScalar::declare(std::string name, const T& defaultVaule):
   \endcode

   @copydetails Monitored::MonitoredScalar::declare(std::string name, const T& defaultVaule)
   
   All above functions are within the Monitored namespace. Consider adding
   \code using namespace Monitored;\endcode
   to your function (but avoid doing this at global scope).

   4)
   @copydoc Monitored::MonitoredScope

   5) Configure the list of histograms in python
   \code
      from AthenaMonitoring.GenericMonitoringTool import GenericMonitoringTool, defineHistogram
      monTool = GenericMonitoringTool('MonTool')
      monTool.Histograms = [defineHistogram('eta', path='EXPERT', type='TH1F', title='Eta;;Entries',
                                             xbins=40, xmin=-2, xmax=2),
                            defineHistogram('phi', path='EXPERT', type='TH1F', title='Phi;;Entries',
                                             xbins=60, xmin=-3.2, xmax=3.2),
                            defineHistogram('eta,phi', path='EXPERT', type='TH2F', title='Eta vs Phi',
                                             xbins=20, xmin=-2, xmax=2, ybins=30, ymin=-3.2, ymax=3.2)]

     topSequence.myAlg.MonTool = monTool
   \endcode

   \remark Without this python configuration, i.e. the last line, no monitoring tool is instantiated
   and no monitoring histograms created thus reducing the overhead (both time and memory) to a minimum.
                                             
   Additional documentation:
   - The MonitoredAlg standalone example and its MonitoredOptions.py job
   options
   - <a href="https://gitlab.cern.ch/atlas/athena/blob/master/Control/AthenaMonitoring/test/GenericMonFilling_test.cxx">GenericMonFilling_test.cxx</a>
   unit test for an example of all features.
 */


