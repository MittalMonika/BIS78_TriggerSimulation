/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AthenaMonitoringKernel_HistogramDef_h
#define AthenaMonitoringKernel_HistogramDef_h

#include <string>
#include <vector>

namespace Monitored {
  /**
   * @brief the internal class used to keep parsed Filler properties
   */
  struct HistogramDef {
    std::vector<std::string> name; //!< names of monitored variables
    std::string alias; //!< unique alias for THistSvc
    std::string type; //!< class name
    std::string path; //!< booking path
    std::string title; //!< title of the histogram
    std::string opt; //!< options
    std::string tld{""}; //!< top level directory (below THistSvc stream)
    std::string weight; //!< name of weight variable
    std::string cutMask; //!< variable that defines whether event is accepted

    // RunMode enum created in case of future usage besides online and offline
    enum class RunMode {
      Online, //!< monitoring data online
      Offline, //!< monitoring data offline
      Invalid, //!< monitoring mode not specified
    };
    // RunPeriod enum indicates when to rebook offline monitoring histograms
    enum class RunPeriod {
      Run, //!< rebook histogram after each run
      LowStat, //!< rebook histogram after every 20 lumiblocks
      Lumiblock, //!< rebook histogram after every 1 lumiblock
      Invalid, //!< rebook period not specified
    };
    std::string convention; //!< path naming convention (e.g. OFFLINE:LowStat)
    RunMode runmode = RunMode::Invalid; //!< online or offline
    RunPeriod runperiod = RunPeriod::Invalid; //!< rebook period in offline monitoring

    bool Sumw2; //!< store sum of squares of weights
    int kLBNHistoryDepth; //!< length of lb history
    bool kAddBinsDynamically; //!< add new bins outside the existing range
    bool kRebinAxes; //!< increase the axis range without adding new bins
    bool kCanRebin; //!< allow all axes to be rebinned
    bool kVec; //!< add content to each bin from each element of a vector
    bool kVecUO; //!< add content to each bin from vector, including overflow/underflow
    bool kCumulative; //!< fill bin of monitored object's value, and every bin below it

    std::string xvar; //!< name of x variable
    int xbins; //!< number of y bins
    float xmin; //!< x axis minimum
    float xmax; //!< x axis maximum
    std::vector<std::string> xlabels; //!< labels for x axis
    std::vector<double> xarray; //!< array of x bin edges

    std::string yvar; //!< name of y variable
    int ybins; //!< number of y bins
    float ymin; //!< y axis minimum
    float ymax; //!< y axis maximum
    std::vector<std::string> ylabels; //!< labels for y axis
    std::vector<double> yarray; //!< array of y bin edges

    std::string zvar; //!< name of z variable
    int zbins; //!< number of z bins
    float zmin; //!< z axis minimum
    float zmax; //!< z axis maximum
    std::vector<std::string> zlabels; //!< labels for z axis

    std::string treeDef; //!< defines output TTree of monitored quantities
    std::string merge;

    bool ok{false}; //!< good declaration: parsing or copying successful

    /**
     * @brief Parses histogram defintion from json data.
     *
     * @param histogramDefinition A string contains histogram definition to parse
     * @return HistogramDef Object that contains the parsed histogram definition
     */
    static const HistogramDef parse(const std::string& histogramDefinition);
  };
} // namespace Monitored

#endif /* AthenaMonitoringKernel_HistogramDef_h */
