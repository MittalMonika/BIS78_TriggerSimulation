/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SCTCalibWriteTool.cxx
 *
 * @brief Implementation file for uploading to DB on CAF
 *
 * @author Jose E. Garcia
 **/
#include "SCT_CalibAlgs/SCTCalibWriteTool.h"

// IOVDbTest includes
#include "RegistrationServices/IIOVRegistrationSvc.h"

// Athena includes
#include "AthenaKernel/IAthenaOutputStreamTool.h"
#include "CoralBase/Attribute.h"

#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "Identifier/IdentifierHash.h"
#include "InDetIdentifier/SCT_ID.h"

// Event Info
#include "EventInfo/EventID.h"
#include "EventInfo/EventType.h"

//path resolver to find the file
#include "PathResolver/PathResolver.h"

// Gaudi includes
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/IToolSvc.h"

#include <fstream>
#include <iterator>
#include <sstream>
#include <iostream>
#include <istream>
#include <boost/lexical_cast.hpp>

using std::string;
/////////////////////////////////////////////////////////////////////////////
const string SCTCalibWriteTool::s_separator{"-"};
const string SCTCalibWriteTool::s_defectFolderName{"/SCT/Derived/Monitoring"};
const string SCTCalibWriteTool::s_deadStripFolderName{"/SCT/Derived/DeadStrips"};
const string SCTCalibWriteTool::s_deadChipFolderName{"/SCT/Derived/DeadChips"};
const string SCTCalibWriteTool::s_effFolderName{"/SCT/Derived/Efficiency"};
const string SCTCalibWriteTool::s_noFolderName{"/SCT/Derived/NoiseOccupancy"};
const string SCTCalibWriteTool::s_RawOccuFolderName{"/SCT/Derived/RawOccupancy"};
const string SCTCalibWriteTool::s_BSErrFolderName{"/SCT/Derived/BSErrorsRun2"};
const string SCTCalibWriteTool::s_LAFolderName{"/SCT/Derived/LorentzAngleRun2_v2"};

const bool becCapsFormat{true};
const bool becUnderscoreFormat{false};

SCTCalibWriteTool::SCTCalibWriteTool(const std::string& type, const std::string& name, const IInterface* parent) :
  AthAlgTool(type, name, parent)
{
}

///////////////////////////////////////////////////////////////////////////////////////////

StatusCode
SCTCalibWriteTool::queryInterface(const InterfaceID& riid, void** ppvIF)
{
  if (SCTCalibWriteTool::interfaceID().versionMatch(riid) ) {
    *ppvIF = static_cast<SCTCalibWriteTool*>(this);
  } else {
    return AthAlgTool::queryInterface(riid, ppvIF);
  }
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////

StatusCode
SCTCalibWriteTool::initialize() {
  if (detStore()->retrieve(m_pHelper,"SCT_ID").isFailure()) {
    ATH_MSG_ERROR("SCT mgr failed to retrieve");
    return StatusCode::FAILURE;
  }

  ATH_CHECK(m_eventInfoKey.initialize());

  // ------------------------------------------------------------
  // The following is required for writing out something to COOL

  // CondAttrListCollection to store table temporarily
  m_attrListColl = new CondAttrListCollection{true};
  m_attrListColl_deadStrip = new CondAttrListCollection{true};
  m_attrListColl_deadChip = new CondAttrListCollection{true};
  m_attrListColl_eff = new CondAttrListCollection{true};
  m_attrListColl_no = new CondAttrListCollection{true};
  m_attrListColl_RawOccu = new CondAttrListCollection{true};
  m_attrListColl_BSErr = new CondAttrListCollection{true};
  m_attrListColl_LA = new CondAttrListCollection{true};

  // Get the IOVRegistrationSvc when needed
  if (m_regIOV) {
    if (service("IOVRegistrationSvc", m_regSvc).isFailure()) {
      ATH_MSG_ERROR("Unable to find IOVRegistrationSvc ");
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////

StatusCode
SCTCalibWriteTool::finalize() {
  delete m_attrListColl;
  delete m_attrListColl_deadStrip;
  delete m_attrListColl_deadChip;
  delete m_attrListColl_eff;
  delete m_attrListColl_no;
  delete m_attrListColl_RawOccu;
  delete m_attrListColl_BSErr;
  delete m_attrListColl_LA;
  return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////

unsigned int
SCTCalibWriteTool::computeIstrip4moncond(const Identifier& elementId) const {
  unsigned int iiside{static_cast<unsigned int>(m_pHelper->side(elementId))};
  unsigned int iistrip{static_cast<unsigned int>(m_pHelper->strip(elementId))};
  return 768*iiside + iistrip;
}

//////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
// Local stuff
//////////////////////////////////////////////////////////////////////////////////////////

string
SCTCalibWriteTool::addDefect(const string& defectlist, const int defectBeginChannel, const int defectEndChannel) const {
  // check size of defect list,
  // if it is empty then use createDefectString to make first entry.
  if (defectlist.empty()) return createDefectString(defectBeginChannel, defectEndChannel);

  // adding another Defect in DefectList
  std::ostringstream defect;
  defect << defectlist << " " << defectBeginChannel;
  if (defectBeginChannel==defectEndChannel) {
    defect << " ";
  } else {
    defect << "-" << defectEndChannel << " ";
  }
  return defect.str();
}

///////////////////////////////////////////////////////////////////////////////////////

std::string
SCTCalibWriteTool::createDefectString(const int defectBeginChannel, const int defectEndChannel) const {
  std::ostringstream defect;
  defect << " " << defectBeginChannel;
  if (defectBeginChannel!=defectEndChannel) {
    defect << "-" << defectEndChannel;
  }
  defect << " ";
  return defect.str();
}


///////////////////////////////////////////////////////////////////////////////////////

std::string
SCTCalibWriteTool::addNumber(const string numStr, const unsigned long long number) const {
  std::ostringstream num_string;
  // if it is empty then use createDefectString to make first entry.
  if (numStr.empty()) {
    num_string << number;
  } else { // adding another number to numStr
    num_string << numStr << " " << number;
  }
  return num_string.str();
}

/////////////////////////////////////////////////////////////////////////////////////////////

StatusCode
SCTCalibWriteTool::createCondObjects(const Identifier& wafer_id, const SCT_ID* sctId, const int samplesize, const std::string& defectType, const float threshold, const std::string& defectList) const {
  if (!m_writeCondObjs) {
    return StatusCode::SUCCESS;
  }
  coral::AttributeListSpecification* attrSpec{createBasicDbSpec(becCapsFormat)};
  attrSpec->extend("DefectType", "string");
  attrSpec->extend("Threshold", "float");
  attrSpec->extend("DefectList", "string");

  if (!attrSpec->size()) {
    ATH_MSG_ERROR(" Attribute list specification is empty");
    return StatusCode::FAILURE;
  }

  coral::AttributeList attrList0{*attrSpec};
  setBasicValues(attrList0, wafer_id, samplesize,sctId,becCapsFormat);
  attrList0["DefectType"].setValue(static_cast<std::string>(defectType));
  attrList0["Threshold"].setValue(static_cast<float>(threshold));
  attrList0["DefectList"].setValue(static_cast<std::string>(defectList));
  std::ostringstream attrStr2;
  attrList0.toOutputStream(attrStr2);
  m_attrListColl->add(wafer_id.get_identifier32().get_compact(), attrList0);
  return StatusCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

StatusCode
SCTCalibWriteTool::createListStrip(const Identifier& wafer_id,
                                   const SCT_ID* sctId,
                                   const int samplesize,
                                   const std::string& defectType,
                                   const float threshold,
                                   const std::string& defectList) const
{
  if (!m_writeCondObjs) {
    return StatusCode::SUCCESS;
  }
  coral::AttributeListSpecification* attrSpec{createBasicDbSpec(becCapsFormat)};
  attrSpec->extend("DefectType", "string");
  attrSpec->extend("Threshold", "float");
  attrSpec->extend("DefectList", "string");

  if (!attrSpec->size()) {
    ATH_MSG_ERROR(" Attribute list specification is empty");
    return StatusCode::FAILURE;
  }

  coral::AttributeList attrList0{*attrSpec};
  setBasicValues(attrList0, wafer_id, samplesize, sctId, becCapsFormat);
  attrList0["DefectType"].setValue(static_cast<std::string>(defectType));
  attrList0["Threshold"].setValue(static_cast<float>(threshold));
  attrList0["DefectList"].setValue(static_cast<std::string>(defectList));

  std::ostringstream attrStr2;
  attrList0.toOutputStream(attrStr2);
  m_attrListColl_deadStrip->add(wafer_id.get_identifier32().get_compact(), attrList0);
  ATH_MSG_INFO("createListStrip: return StatusCode::SUCCESS");
  return StatusCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

StatusCode
SCTCalibWriteTool::createListChip(const Identifier& wafer_id, const SCT_ID* sctId, const int samplesize, const std::string& defectType, const float threshold, const std::string& defectList) const {
  if (!m_writeCondObjs) {
    return StatusCode::SUCCESS;
  }
  coral::AttributeListSpecification* attrSpec{createBasicDbSpec(becCapsFormat)};
  attrSpec->extend("DefectType", "string");
  attrSpec->extend("Threshold", "float");
  attrSpec->extend("DefectList", "string");
  if (!attrSpec->size()) {
    ATH_MSG_ERROR(" Attribute list specification is empty");
    return StatusCode::FAILURE;
  }

  // Add three attr lists
  coral::AttributeList attrList0{*attrSpec};
  setBasicValues(attrList0, wafer_id, samplesize, sctId, becCapsFormat);
  attrList0["DefectType"].setValue(static_cast<std::string>(defectType));
  attrList0["Threshold"].setValue(static_cast<float>(threshold));
  attrList0["DefectList"].setValue(static_cast<std::string>(defectList));

  std::ostringstream attrStr2;
  attrList0.toOutputStream(attrStr2);
  m_attrListColl_deadChip->add(wafer_id.get_identifier32().get_compact(), attrList0);

  return StatusCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

StatusCode
SCTCalibWriteTool::createListEff(const Identifier& wafer_id,const SCT_ID* sctId, const int samplesize, const float eff) const {
  if (!m_writeCondObjs) {
    return StatusCode::SUCCESS;
  }

  coral::AttributeListSpecification* attrSpec{createBasicDbSpec(becUnderscoreFormat)};
  attrSpec->extend("Efficiency", "float");
  if (!attrSpec->size()) {
    ATH_MSG_ERROR(" Attribute list specification is empty");
    return StatusCode::FAILURE;
  }

  coral::AttributeList attrList0{*attrSpec};
  setBasicValues(attrList0, wafer_id, samplesize,sctId,becUnderscoreFormat);
  attrList0["Efficiency"].setValue(static_cast<float>(eff));

  std::ostringstream attrStr2;
  attrList0.toOutputStream(attrStr2);
  m_attrListColl_eff->add(wafer_id.get_identifier32().get_compact(), attrList0);

  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////

StatusCode
SCTCalibWriteTool::createListNO(const Identifier& wafer_id, const SCT_ID* sctId, const int samplesize, const float noise_occ) const {
  if (!m_writeCondObjs) {
    return StatusCode::SUCCESS;
  }
  coral::AttributeListSpecification* attrSpec{createBasicDbSpec(becUnderscoreFormat)};
  attrSpec->extend("NoiseOccupancy", "float");
  if (!attrSpec->size()) {
    ATH_MSG_ERROR(" Attribute list specification is empty");
    return StatusCode::FAILURE;
  }
  // Add three attr lists
  coral::AttributeList attrList0{*attrSpec};
  setBasicValues(attrList0, wafer_id, samplesize, sctId, becUnderscoreFormat);
  attrList0["NoiseOccupancy"].setValue(static_cast<float>(noise_occ));
  std::ostringstream attrStr2;
  attrList0.toOutputStream(attrStr2);
  m_attrListColl_no->add(wafer_id.get_identifier32().get_compact(), attrList0);
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////

StatusCode
SCTCalibWriteTool::createListRawOccu(const Identifier& wafer_id, const SCT_ID* sctId, const int samplesize, const float raw_occu) const {
  if (!m_writeCondObjs) {
    return StatusCode::SUCCESS;
  }
  coral::AttributeListSpecification* attrSpec{createBasicDbSpec(becUnderscoreFormat)};
  attrSpec->extend("RawOccupancy", "float");
  if (!attrSpec->size()) {
    ATH_MSG_ERROR(" Attribute list specification is empty");
    return StatusCode::FAILURE;
  }
  // Add three attr lists
  coral::AttributeList attrList0{*attrSpec};
  setBasicValues(attrList0, wafer_id, samplesize, sctId,becUnderscoreFormat);
  attrList0["RawOccupancy"].setValue(static_cast<float>(raw_occu));
  std::ostringstream attrStr2;
  attrList0.toOutputStream(attrStr2);
  m_attrListColl_RawOccu->add(wafer_id.get_identifier32().get_compact(), attrList0);
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////

StatusCode
SCTCalibWriteTool::createListBSErr(const Identifier& wafer_id,const SCT_ID* sctId,const int samplesize, const std::string& errorList, const std::string& probList) const {
  if (!m_writeCondObjs) {
    return StatusCode::SUCCESS;
  }

  int eta{sctId->eta_module(wafer_id)};
  int phi{sctId->phi_module(wafer_id)};
  int barrel_ec{sctId->barrel_ec(wafer_id)};
  int layer{sctId->layer_disk(wafer_id)};

  coral::AttributeListSpecification* attrSpec{new coral::AttributeListSpecification{}};
  attrSpec->extend("SampleSize", "int");
  attrSpec->extend("barrel_endcap", "int");
  attrSpec->extend("Layer", "int");
  attrSpec->extend("Eta", "int");
  attrSpec->extend("Phi", "int");
  attrSpec->extend("BSErrors", "string");
  attrSpec->extend("BadFraction", "string");

  if (!attrSpec->size()) {
    ATH_MSG_ERROR(" Attribute list specification is empty");
    return StatusCode::FAILURE;
  }

  // Add three attr lists
  coral::AttributeList attrList0{*attrSpec};
  attrList0["SampleSize"].setValue(static_cast<int>(samplesize));
  attrList0["barrel_endcap"].setValue(static_cast<int>(barrel_ec));
  attrList0["Layer"].setValue(static_cast<int>(layer));
  attrList0["Phi"].setValue(static_cast<int>(phi));
  attrList0["Eta"].setValue(static_cast<int>(eta));
  attrList0["BSErrors"].setValue(static_cast<std::string>(errorList));
  attrList0["BadFraction"].setValue(static_cast<std::string>(probList));

  std::ostringstream attrStr2;
  attrList0.toOutputStream(attrStr2);
  m_attrListColl_BSErr->add(wafer_id.get_identifier32().get_compact(), attrList0);
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////

StatusCode
SCTCalibWriteTool::createListLA(const Identifier& wafer_id,const SCT_ID* sctId,const int samplesize,int module, const float lorentz, const float err_lorentz, const float chisq, const float fitParam_a, const float err_a, const float fitParam_b, const float err_b, const float fitParam_sigma, const float err_sigma, const float MCW, const float err_MCW) const {
  if (!m_writeCondObjs) return StatusCode::SUCCESS;
  int barrel_ec{sctId->barrel_ec(wafer_id)};
  int layer{sctId->layer_disk(wafer_id)};
  int side{sctId->side(wafer_id)};

  coral::AttributeListSpecification* attrSpec{new coral::AttributeListSpecification{}};
  attrSpec->extend("SampleSize", "int");
  attrSpec->extend("barrel_endcap", "int");
  attrSpec->extend("Layer", "int");
  attrSpec->extend("Side", "int");
  attrSpec->extend("moduleType", "int");
  attrSpec->extend("lorentzAngle", "float");
  attrSpec->extend("err_lorentzAngle", "float");
  attrSpec->extend("chisq", "float");
  attrSpec->extend("fitParam_a", "float");
  attrSpec->extend("err_a", "float");
  attrSpec->extend("fitParam_b", "float");
  attrSpec->extend("err_b", "float");
  attrSpec->extend("fitParam_sigma", "float");
  attrSpec->extend("err_sigma", "float");
  attrSpec->extend("minClusterWidth", "float");
  attrSpec->extend("err_minClusterWidth", "float");

  if (!attrSpec->size()) {
    ATH_MSG_ERROR(" Attribute list specification is empty");
    return StatusCode::FAILURE;
  }

  // Add three attr lists
  coral::AttributeList attrList0{*attrSpec};
  attrList0["SampleSize"].setValue(static_cast<int>(samplesize));
  attrList0["barrel_endcap"].setValue(static_cast<int>(barrel_ec));
  attrList0["Layer"].setValue(static_cast<int>(layer));
  attrList0["Side"].setValue(static_cast<int>(side));
  attrList0["moduleType"].setValue(static_cast<int>(module));
  attrList0["lorentzAngle"].setValue(static_cast<float>(lorentz));
  attrList0["err_lorentzAngle"].setValue(static_cast<float>(err_lorentz));
  attrList0["chisq"].setValue(static_cast<float>(chisq));
  attrList0["fitParam_a"].setValue(static_cast<float>(fitParam_a));
  attrList0["err_a"].setValue(static_cast<float>(err_a));
  attrList0["fitParam_b"].setValue(static_cast<float>(fitParam_b));
  attrList0["err_b"].setValue(static_cast<float>(err_b));
  attrList0["fitParam_sigma"].setValue(static_cast<float>(fitParam_sigma));
  attrList0["err_sigma"].setValue(static_cast<float>(err_sigma));
  attrList0["minClusterWidth"].setValue(static_cast<float>(MCW));
  attrList0["err_minClusterWidth"].setValue(static_cast<float>(err_MCW));

  std::ostringstream attrStr2;
  attrList0.toOutputStream(attrStr2);
  m_attrListColl_LA->add(wafer_id.get_identifier32().get_compact(), attrList0);

  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////

const CondAttrListCollection*
SCTCalibWriteTool::getAttrListCollectionByFolder(const string& foldername) const {
  std::lock_guard<std::mutex> lock{m_mutex};
  // trying to find the pointer in the hashmap
  // if it exists, return it, otherwise put it in.
  const CondAttrListCollection* attrListCollection{nullptr};
  if (m_attrListCollectionMap.count(foldername) == 0) {
    if (detStore()->retrieve(attrListCollection, foldername).isFailure()) {
      ATH_MSG_ERROR("Could not retrieve " << foldername);
      return nullptr;
    }
    m_attrListCollectionMap.insert(make_pair(foldername, attrListCollection));
  } else {
    attrListCollection = m_attrListCollectionMap[foldername];
  }
  return attrListCollection;
}

///////////////////////////////////////////////////////////////////////////////////////

int
SCTCalibWriteTool::stringToInt(const std::string& s) const {
  return atoi(s.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////

StatusCode
SCTCalibWriteTool::wrapUpNoisyChannel() {
  if (recordAndStream(m_attrListColl, s_defectFolderName, m_defectRecorded).isFailure()) return StatusCode::FAILURE;
  if (registerCondObjectsWithErrMsg(s_defectFolderName, m_tagID4NoisyStrips).isFailure()) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////

StatusCode
SCTCalibWriteTool::wrapUpDeadStrips() {
  if (recordAndStream(m_attrListColl_deadStrip, s_deadStripFolderName, m_deadStripRecorded).isFailure()) return  StatusCode::FAILURE;
  if (registerCondObjectsWithErrMsg(s_deadStripFolderName, m_tagID4DeadStrips).isFailure()) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////
StatusCode

SCTCalibWriteTool::wrapUpDeadChips() {
  if (recordAndStream(m_attrListColl_deadChip, s_deadChipFolderName, m_deadChipRecorded).isFailure())  return StatusCode::FAILURE;
  if (registerCondObjectsWithErrMsg(s_deadChipFolderName, m_tagID4DeadChips).isFailure()) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////

StatusCode 
SCTCalibWriteTool::wrapUpEfficiency() {
  if (recordAndStream(m_attrListColl_eff, s_effFolderName, m_effRecorded).isFailure()) return StatusCode::FAILURE;
  if (registerCondObjectsWithErrMsg(s_effFolderName, m_tagID4Efficiency).isFailure()) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////

StatusCode 
SCTCalibWriteTool::wrapUpNoiseOccupancy() {
  if (recordAndStream(m_attrListColl_no, s_noFolderName, m_noRecorded).isFailure()) return StatusCode::FAILURE;
  if (registerCondObjectsWithErrMsg(s_noFolderName, m_tagID4NoiseOccupancy).isFailure()) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////

StatusCode 
SCTCalibWriteTool::wrapUpRawOccupancy() {
  if (recordAndStream(m_attrListColl_RawOccu, s_RawOccuFolderName, m_RawOccuRecorded).isFailure()) return StatusCode::FAILURE;
  if (registerCondObjectsWithErrMsg(s_RawOccuFolderName, m_tagID4RawOccupancy).isFailure()) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////

StatusCode 
SCTCalibWriteTool::wrapUpBSErrors() {
  if (recordAndStream(m_attrListColl_BSErr, s_BSErrFolderName, m_BSErrRecorded).isFailure()) return StatusCode::FAILURE;
  if (registerCondObjectsWithErrMsg(s_BSErrFolderName, m_tagID4BSErrors).isFailure()) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////

StatusCode 
SCTCalibWriteTool::wrapUpLorentzAngle() {
  if (recordAndStream(m_attrListColl_LA, s_LAFolderName, m_LARecorded).isFailure()) return StatusCode::FAILURE;
  if (registerCondObjectsWithErrMsg(s_LAFolderName, m_tagID4LorentzAngle).isFailure()) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////

StatusCode 
SCTCalibWriteTool::streamOutCondObjects(const std::string& foldername) {
  if (m_streamer->connectOutput(m_streamName).isFailure()) {
    ATH_MSG_ERROR("Could not connect stream to output");
    return( StatusCode::FAILURE);
  }
  IAthenaOutputStreamTool::TypeKeyPairs typeKeys{1};
  if (m_readWriteCool) {
    IAthenaOutputStreamTool::TypeKeyPair
      attrCollPair{"CondAttrListCollection", foldername};
    typeKeys[0] = attrCollPair;
  }

  if (m_streamer->streamObjects(typeKeys).isFailure()) {
    ATH_MSG_ERROR("Could not stream out AttributeLists");
    return StatusCode::FAILURE;
  }

  if (m_streamer->commitOutput().isFailure()) {
    ATH_MSG_ERROR("Could not commit output stream");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////

StatusCode
SCTCalibWriteTool::streamOutCondObjectsWithErrMsg(const std::string& foldername) {
  if (streamOutCondObjects(foldername).isFailure()) {
    ATH_MSG_ERROR("Could create conditions object  " << foldername);
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////

StatusCode
SCTCalibWriteTool::registerCondObjects(const std::string& foldername,const std::string& tagname) const {
  // Register the IOV DB with the conditions data written out
  if (m_readWriteCool) {
    // Can only write out AttrList's if this is NOT write and reg in two steps
    if (!m_twoStepWriteReg) {
      // Using COOL, write out attrlist and collection of attrlists
      // attrlist collection
      StatusCode sc;
      unsigned int beginRun;
      unsigned int endRun;
      if (!m_manualiov) {

        SG::ReadHandle<EventInfo> evt{m_eventInfoKey};
        if (not evt.isValid()) {
          ATH_MSG_ERROR("Unable to get the EventInfo");
          return StatusCode::FAILURE;
        }

        beginRun = evt->event_ID()->run_number();
        endRun = beginRun;

      } else {
        beginRun = m_beginRun;
        if ( m_endRun != -1 ) endRun = m_endRun;
        else                  endRun = IOVTime::MAXRUN;
      }

      unsigned int beginLB{IOVTime::MINEVENT};
      unsigned int endLB{IOVTime::MAXEVENT};

      if (not tagname.empty()) {
        sc = m_regSvc->registerIOV("CondAttrListCollection", foldername, tagname, beginRun, endRun, beginLB, endLB);
      } else {
        sc = m_regSvc->registerIOV("CondAttrListCollection", foldername, "", beginRun, endRun, beginLB, endLB);
      }
      if (sc.isFailure()) {
        ATH_MSG_ERROR("Could not register in IOV DB for CondAttrListCollection");
        return StatusCode::FAILURE;
      }
    }
  }
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////

StatusCode
SCTCalibWriteTool::registerCondObjectsWithErrMsg(const std::string& foldername,const std::string& tagname) const {
  if (m_regIOV) {
    if (registerCondObjects(foldername,tagname).isFailure()) {
      ATH_MSG_ERROR("Could not register " << foldername);
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////

StatusCode
SCTCalibWriteTool::recordAndStream(const CondAttrListCollection* pCollection,const std::string& foldername, bool& flag) {
  if (m_writeCondObjs) {
    if (detStore()->record(pCollection, foldername).isFailure()) {
      ATH_MSG_ERROR("Could not record "<<foldername);
      return StatusCode::FAILURE;
    }
    flag=true;
    if (streamOutCondObjectsWithErrMsg(s_defectFolderName).isFailure()) return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////

coral::AttributeListSpecification*
SCTCalibWriteTool::createBasicDbSpec(const bool capsFormat) const {
  coral::AttributeListSpecification* attrSpec{new coral::AttributeListSpecification{}};
  const std::string becName{capsFormat?"BarrelEndcap":"barrel_endcap"};
  attrSpec->extend("SampleSize", "int");
  attrSpec->extend(becName, "int");
  attrSpec->extend("Layer", "int");
  attrSpec->extend("Eta", "int");
  attrSpec->extend("Phi", "int");
  return attrSpec;
}

///////////////////////////////////////////////////////////////////////////////////////

void
SCTCalibWriteTool::setBasicValues(coral::AttributeList& attrList, const Identifier& wafer_id, const int samplesize, const SCT_ID* sctId, const bool capsFormat) const {
  int eta{sctId->eta_module(wafer_id)};
  int phi{sctId->phi_module(wafer_id)};
  int barrel_ec{sctId->barrel_ec(wafer_id)};
  int layer{sctId->layer_disk(wafer_id)};
  //
  const std::string becName{capsFormat?"BarrelEndcap":"barrel_endcap"};
  attrList["SampleSize"].setValue(static_cast<int>(samplesize));
  attrList[becName].setValue(static_cast<int>(barrel_ec));
  attrList["Layer"].setValue(static_cast<int>(layer));
  attrList["Eta"].setValue(static_cast<int>(eta));
  attrList["Phi"].setValue(static_cast<int>(phi));

  return;
}

///////////////////////////////////////////////////////////////////////////////////////