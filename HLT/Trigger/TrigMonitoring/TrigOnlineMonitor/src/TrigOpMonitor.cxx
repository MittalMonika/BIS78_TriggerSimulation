/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigOpMonitor.h"

#include "AthenaKernel/IOVRange.h"
#include "AthenaMonitoringKernel/OHLockedHist.h"
#include "ByteStreamData/ByteStreamMetadata.h"
#include "ByteStreamData/ByteStreamMetadataContainer.h"
#include "StoreGate/ReadCondHandle.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "eformat/DetectorMask.h"
#include "eformat/SourceIdentifier.h"

#include <algorithm>
#include <fstream>

#include <boost/algorithm/string.hpp>

// Helpers
namespace {
  /// Sanitize COOL folder names
  std::string folder2HistName(const std::string& folder)
  {
    std::string s(folder);
    if (s.front() == '/') s.erase(0, 1);
    std::replace(s.begin(), s.end(), '/', '_');
    return s;
  }

  /// Read release metadata file from InstallArea/$BINARY_TAG/ReleaseData
  StatusCode readReleaseData(const std::string& file, std::map<std::string, std::string>& result)
  {
    std::ifstream f;
    f.open(file.c_str(), std::ifstream::in);
    if (!f.is_open()) return StatusCode::FAILURE;

    std::string line;
    while (getline(f, line)) {
      // Format: name:value
      std::string::size_type loc = line.find(":");
      if (loc == std::string::npos || loc == 0) continue;
      result[line.substr(0, loc)] = line.substr(loc + 1);
    }
    f.close();
    return StatusCode::SUCCESS;
  }

} // namespace

TrigOpMonitor::TrigOpMonitor(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator),
  m_histPath("/EXPERT/HLTFramework/" + name + "/")
{}

StatusCode TrigOpMonitor::initialize()
{
  ATH_CHECK(m_histSvc.retrieve());
  ATH_CHECK(m_lumiDataKey.initialize(!m_lumiDataKey.empty()));

  return StatusCode::SUCCESS;
}

StatusCode TrigOpMonitor::start()
{
  m_previousLB = 0;
  ATH_CHECK(bookHists());

  fillReleaseDataHist();
  return StatusCode::SUCCESS;
}

StatusCode TrigOpMonitor::execute()
{
  static bool first_call{true};
  if (first_call) {
    first_call = false;
    /* One time fills. Could be done in start(), but we need to run
       after all other services have been setup. */
    fillMagFieldHist();
    fillIOVDbHist();
    // This one needs to run after prepareForRun, which is currently called after Alg's start()
    fillSubDetHist();
  }

  /* Per-LB fills */
  const EventContext& ctx = getContext();
  if (m_previousLB != ctx.eventID().lumi_block()) { // New LB
    m_previousLB = ctx.eventID().lumi_block();

    // Fill IOV diff histogram
    fillIOVDbChangeHist(ctx);

    // Fill lumi histogram
    fillLumiHist(ctx);
  }

  return StatusCode::SUCCESS;
}

StatusCode TrigOpMonitor::bookHists()
{
  m_magFieldHist = nullptr;
  m_iovChangeHist = nullptr;
  m_currentIOVs.clear();
  m_folderHist.clear();

  if (!m_IOVDbSvc) service("IOVDbSvc", m_IOVDbSvc, /*createIf=*/false).ignore();
  if (m_IOVDbSvc) {
    m_iovChangeHist = new TH2I("CoolFolderUpdate_LB", "COOL folder updates;Lumiblock;", 1, 0, 1, 1,
                               0, 1);
  }

  // create histogram for magnetic field information
  if (!m_magFieldSvc) service("AtlasFieldSvc", m_magFieldSvc, /*createIf=*/false).ignore();
  if (m_magFieldSvc) {
    m_magFieldHist = new TH2I("MagneticFieldSettings", "Magnetic field settings", 2, 0, 2, 2, 0, 2);

    m_magFieldHist->GetYaxis()->SetBinLabel(1, "off");
    m_magFieldHist->GetYaxis()->SetBinLabel(2, "on");
    m_magFieldHist->GetXaxis()->SetBinLabel(1, "Solenoid");
    m_magFieldHist->GetXaxis()->SetBinLabel(2, "Toroid");
  }

  m_releaseHist = new TH1I("GeneralOpInfo", "General operational info;;Applications", 1, 0, 1);

  m_subdetHist = new TH2I("Subdetectors", "State of subdetectors", 1, 0, 1, 3, 0, 3);
  m_subdetHist->SetCanExtend(TH1::kXaxis);
  m_subdetHist->GetYaxis()->SetBinLabel(1, "# ROB");
  m_subdetHist->GetYaxis()->SetBinLabel(2, "off");
  m_subdetHist->GetYaxis()->SetBinLabel(3, "on");

  m_lumiHist = new TProfile("Luminosity", "Luminosity;Lumiblock;Luminosity [10^{33} cm^{-2}s^{-1}]",
                            m_maxLB, 0, m_maxLB);

  m_muHist = new TProfile("Pileup", "Pileup;Lumiblock;Interactions per BX", m_maxLB, 0, m_maxLB);

  // Register histograms
  TH1* hist[] = {m_releaseHist, m_subdetHist, m_iovChangeHist, m_magFieldHist, m_lumiHist, m_muHist};
  for (TH1* h : hist) {
    if (h) ATH_CHECK(m_histSvc->regHist(m_histPath + h->GetName(), h));
  }

  return StatusCode::SUCCESS;
}

void TrigOpMonitor::fillMagFieldHist()
{
  if (!m_magFieldSvc || !m_magFieldHist) return;

  m_magFieldHist->Fill("Solenoid", m_magFieldSvc->solenoidOn() ? "on" : "off", 1.0);
  m_magFieldHist->Fill("Toroid", m_magFieldSvc->toroidOn() ? "on" : "off", 1.0);
}

void TrigOpMonitor::fillIOVDbHist()
{
  if (!m_IOVDbSvc) return;

  // create and fill histogram for IOVDb entries
  std::vector<std::string> keyList(m_IOVDbSvc->getKeyList());
  std::string folderName;
  std::string tag;
  IOVRange range;
  bool retrieved = false;
  unsigned long long bytesRead = 0;
  float readTime = 0;

  // set up histograms
  TH2I* IOVDbRunHist = new TH2I("IOVDbRunRange", "IOVDb Run Range", 1, 0, 1, 1, 0, 1);
  TH2I* IOVDbTimeHist = new TH2I("IOVDbTimeRange", "IOVDb Time Range", 1, 0, 1, 1, 0, 1);
  TProfile* IOVDbBytesReadHist = new TProfile("IOVDbBytesRead", "Folder data read", 1, 0, 1);
  IOVDbBytesReadHist->SetYTitle("Data read [bytes]");
  TProfile* IOVDbReadTimeHist = new TProfile("IOVDbReadTime", "Folder update time", 1, 0, 1);
  IOVDbReadTimeHist->SetYTitle("Update time [ms]");

  // fill histograms
  for (const std::string& key : keyList) {

    if (m_IOVDbSvc->getKeyInfo(key, folderName, tag, range, retrieved, bytesRead, readTime) &&
        retrieved) {

      m_currentIOVs[key] = range;

      IOVTime start(range.start());
      IOVTime stop(range.stop());

      // fill start condition (run number)
      if (start.isRunEvent()) {
        IOVDbRunHist->Fill(std::to_string(start.run()).c_str(), folderName.c_str(), 1.0);
      }

      // fill start condition (timestamp)
      if (start.isTimestamp()) {
        IOVDbTimeHist->Fill(std::to_string(start.timestamp()).c_str(), folderName.c_str(), 1.0);
      }

      // fill stop condition (run number)
      if (stop.isRunEvent()) {
        if (stop.run() == IOVTime::MAXRUN) {
          IOVDbRunHist->Fill("infinity", folderName.c_str(), 1.0);
        }
        else {
          IOVDbRunHist->Fill(std::to_string(stop.run()).c_str(), folderName.c_str(), 1.0);
        }
      }

      // fill stop condition (timestamp)
      if (stop.isTimestamp()) {
        if (stop.timestamp() == IOVTime::MAXTIMESTAMP) {
          IOVDbTimeHist->Fill("infinity", folderName.c_str(), 1.0);
        }
        else {
          IOVDbTimeHist->Fill(std::to_string(stop.timestamp()).c_str(), folderName.c_str(), 1.0);
        }
      }

      // fill read bytes and time read
      IOVDbBytesReadHist->Fill(folderName.c_str(), bytesRead, 1.0);
      IOVDbReadTimeHist->Fill(folderName.c_str(), readTime * 1000, 1.0);
    }
  }

  // final configuration of histograms
  IOVDbRunHist->LabelsDeflate("X");
  IOVDbRunHist->LabelsDeflate("Y");
  IOVDbRunHist->GetYaxis()->LabelsOption("a");

  IOVDbTimeHist->LabelsDeflate("X");
  IOVDbTimeHist->LabelsDeflate("Y");
  IOVDbTimeHist->GetYaxis()->LabelsOption("a");

  IOVDbBytesReadHist->LabelsDeflate("X");
  IOVDbReadTimeHist->LabelsDeflate("X");

  // Register histograms
  TH1* hist[] = {IOVDbRunHist, IOVDbTimeHist, IOVDbBytesReadHist, IOVDbReadTimeHist};
  for (TH1* h : hist) {
    m_histSvc->regHist(m_histPath + h->GetName(), h).ignore();
  }
}

void TrigOpMonitor::fillIOVDbChangeHist(const EventContext& ctx)
{
  if (m_IOVDbSvc == nullptr) return;

  IOVRange iov;
  std::string folder, tag;
  bool retrieved;
  unsigned long long bytesRead;
  float readTime;

  std::vector<std::string> keys(m_IOVDbSvc->getKeyList());

  // Loop over all keys known to IOVDbSvc
  for (const std::string& k : keys) {
    if (not m_IOVDbSvc->getKeyInfo(k, folder, tag, iov, retrieved, bytesRead, readTime)) continue;
    if (not retrieved) continue;

    const auto curIOV = m_currentIOVs.find(k);
    if (curIOV == m_currentIOVs.end()) {
      m_currentIOVs[k] = iov;
      continue;
    }

    // Print IOV changes and fill histogram
    if (iov != curIOV->second) {
      ATH_MSG_INFO("IOV of " << k << " changed from " << curIOV->second << " to " << iov
                             << " on event: " << ctx.eventID());

      if (m_iovChangeHist) {
        // Perform a locked fill and remove any empty bins to allow correct gathering
        oh_scoped_lock_histogram lock;
        m_iovChangeHist->Fill(std::to_string(ctx.eventID().lumi_block()).c_str(), folder.c_str(),
                              1);
        m_iovChangeHist->LabelsDeflate("X");
        m_iovChangeHist->LabelsDeflate("Y");
      }

      // Create detailed histograms per changed folder
      if (m_detailedHists) {
        auto fh = m_folderHist.find(folder);
        if (fh == m_folderHist.end()) {
          std::string name(folder2HistName(folder));

          fh = m_folderHist.insert({folder, FolderHist()}).first;
          fh->second.h_time = new TH1F((name + "_ReadTime").c_str(),
                                       ("Update time for " + folder + ";Time [ms];Entries").c_str(),
                                       100, 0, 200);

          fh->second.h_bytes = new TH1F(
              (name + "_BytesRead").c_str(),
              ("Bytes read for " + folder + ";Data [bytes];Entries").c_str(), 100, 0, 1000);

          for (TH1* h : {fh->second.h_time, fh->second.h_bytes}) {
            m_histSvc->regHist(h->GetName(), h);
          }
        }

        // Need to plot the difference because IOVDbSvc reports total time and bytes for entire job
        fh->second.h_time->Fill(readTime * 1000 - m_folderHist[folder].total_time);
        fh->second.total_time += readTime * 1000;

        fh->second.h_bytes->Fill(bytesRead - m_folderHist[folder].total_bytes);
        fh->second.total_bytes += bytesRead;
      }

      m_currentIOVs[k] = iov;
    }
  }
}

void TrigOpMonitor::fillLumiHist(const EventContext& ctx)
{
  if (!m_lumiDataKey.empty()) {
    SG::ReadCondHandle<LuminosityCondData> lumiData(m_lumiDataKey, ctx);
    m_lumiHist->Fill(ctx.eventID().lumi_block(), lumiData->lbAverageLuminosity());
    m_muHist->Fill(ctx.eventID().lumi_block(), lumiData->lbAverageInteractionsPerCrossing());
  }
}

void TrigOpMonitor::fillReleaseDataHist()
{
  const char* ld_lib_path = getenv("LD_LIBRARY_PATH");
  if (ld_lib_path == nullptr) {
    ATH_MSG_WARNING("LD_LIBRARY_PATH is not defined. Will not fill release histogram.");
    return;
  }

  // Find entries in LD_LIBRARY_PATH matching our project(s)
  std::vector<std::string> paths, file_list;
  boost::split(paths, ld_lib_path, boost::is_any_of(":"));
  for (const std::string& p : paths) {
    for (const std::string& proj : m_projects) {
      if (p.find("/" + proj + "/") != std::string::npos) {
        file_list.push_back(p + "/" + m_releaseData);
      }
    }
  }

  if (file_list.empty()) {
    ATH_MSG_WARNING("Could not find release metadata file " << m_releaseData
                                                            << " in LD_LIBRARY_PATH");
    m_releaseHist->Fill("Release ?", 1);
    return;
  }

  for (const auto& f : file_list) {
    // Read metadata file
    std::map<std::string, std::string> result;
    if (readReleaseData(f, result).isFailure()) {
      ATH_MSG_WARNING("Could not read release metadata from " << f);
      m_releaseHist->Fill("Release ?", 1);
      return;
    }

    // Check format
    if (result.find("project name") == result.end() || result.find("release") == result.end()) {
      ATH_MSG_WARNING("Invalid release metadata format in " << f);
      m_releaseHist->Fill("Release ?", 1);
      return;
    }

    m_releaseHist->Fill((result["project name"] + " " + result["release"]).c_str(), 1);
  }
}

void TrigOpMonitor::fillSubDetHist()
{
  // Retrieve the enabled ROBs/SubDets list from DataFlowConfig which is a special object
  // used online to hold DF properties passed from TDAQ to HLT as run parameters
  SmartIF<IJobOptionsSvc> jobOptionsSvc = service<IJobOptionsSvc>("JobOptionsSvc", /*createIf=*/ false);
  if (!jobOptionsSvc.isValid()) {
    ATH_MSG_WARNING("Could not retrieve JobOptionsSvc, will not fill SubDetectors histogram");
    return;
  }
  const Gaudi::Details::PropertyBase* prop = jobOptionsSvc->getClientProperty("DataFlowConfig", "DF_Enabled_ROB_IDs");
  Gaudi::Property<std::vector<uint32_t>> enabledROBsProp("EnabledROBs",{});
  std::set<uint32_t> enabledROBs;
  if (prop && enabledROBsProp.assign(*prop)) {
    enabledROBs.insert(enabledROBsProp.value().begin(), enabledROBsProp.value().end());
    ATH_MSG_DEBUG("Retrieved a list of " << enabledROBs.size()
                  << " ROBs from DataFlowConfig.DF_Enabled_ROB_IDs");
  }
  else {
    ATH_MSG_DEBUG("Could not retrieve DataFlowConfig.DF_Enabled_ROB_IDs from JobOptionsSvc. This is fine if running "
                  << "offline, but should not happen online");
  }

  // Retrieve detector mask from detector store
  SmartIF<StoreGateSvc> inputMetaDataStore = service<StoreGateSvc>("InputMetaDataStore", /*createIf=*/ false);
  if (!inputMetaDataStore.isValid()) {
    ATH_MSG_WARNING("Could not retrieve InputMetaDataStore, will not fill SubDetectors histogram");
    return;
  }
  const ByteStreamMetadataContainer* metadatacont{nullptr};
  if (inputMetaDataStore->retrieve(metadatacont, "ByteStreamMetadata").isFailure()) {
    ATH_MSG_WARNING("Could not retrieve ByteStreamMetadata, will not fill SubDetectors histogram");
    return;
  }

  const ByteStreamMetadata* metadata = *(metadatacont->begin());
  uint64_t detMaskLeast = metadata->getDetectorMask();
  uint64_t detMaskMost = metadata->getDetectorMask2();

  // Decode subdetector masks
  std::vector<eformat::SubDetector> subDetOn;
  std::vector<eformat::SubDetector> subDetOff;
  std::vector<eformat::SubDetector> subDetAll;
  eformat::helper::DetectorMask(detMaskLeast, detMaskMost).sub_detectors(subDetOn);
  eformat::helper::DetectorMask(~detMaskLeast, ~detMaskMost).sub_detectors(subDetOff);
  eformat::helper::DetectorMask(~std::bitset<128>()).sub_detectors(subDetAll);

  // Add bins with labels for every subdetector name
  for (const eformat::SubDetector sd : subDetAll) {
    m_subdetHist->GetXaxis()->FindBin(eformat::helper::SubDetectorDictionary.string(sd).data());
  }
  m_subdetHist->LabelsDeflate("X");

  // Fill histogram with enabled subdetectors
  for (const eformat::SubDetector sd : subDetOn) {
    m_subdetHist->Fill(eformat::helper::SubDetectorDictionary.string(sd).data(), "on", 1.0);
  }

  // Fill histogram with disabled subdetectors
  for (const eformat::SubDetector sd : subDetOff) {
    m_subdetHist->Fill(eformat::helper::SubDetectorDictionary.string(sd).data(), "off", 1.0);
  }

  // Fill histogram with ROB counts
  for (const uint32_t robid : enabledROBs) {
    const std::string sdname = eformat::helper::SourceIdentifier(robid).human_detector();
    m_subdetHist->Fill(sdname.data(), "# ROB", 1.0);
  }
}