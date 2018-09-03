/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetIdentifier/SCT_ID.h"
#include "InDetOverlay/SCTOverlay.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

namespace Overlay
{
  // Specialize copyCollection() for the SCT
  template<> void copyCollection(const InDetRawDataCollection<SCT_RDORawData> *input_coll, InDetRawDataCollection<SCT_RDORawData> *copy_coll)
  {  
    copy_coll->setIdentifier(input_coll->identify());
    InDetRawDataCollection<SCT_RDORawData>::const_iterator firstData = input_coll->begin();
    InDetRawDataCollection<SCT_RDORawData>::const_iterator lastData = input_coll->end();	
    for ( ; firstData != lastData; ++firstData)
    {	
      const Identifier ident = (*firstData)->identify();
      const unsigned int word = (*firstData)->getWord();
      const SCT3_RawData* oldData = dynamic_cast<const SCT3_RawData*>(*firstData);	
      std::vector<int> errorHit=oldData->getErrorCondensedHit();
      SCT3_RawData *newData=new SCT3_RawData(ident, word, &errorHit);
      copy_coll->push_back(newData);
    }
  }

  // Specialize mergeCollectionsNew() for the SCT
  template<> void mergeCollectionsNew(InDetRawDataCollection<SCT_RDORawData> *bkgCollection,
                                      InDetRawDataCollection<SCT_RDORawData> *signalCollection,
                                      InDetRawDataCollection<SCT_RDORawData> *outputCollection,
                                      IDC_OverlayBase *tmp)
  {
    // We want to use the SCT_ID helper provided by SCTOverlay, thus the constraint
    SCTOverlay *parent = dynamic_cast<SCTOverlay*>(tmp);
    if (!parent) {
      std::ostringstream os;
      os << "mergeCollectionsNew<SCT_RDORawData>() called by a wrong parent algorithm?  Must be SCTOverlay.";
      throw std::runtime_error(os.str());
    }

    // ----------------------------------------------------------------
    // debug
    static bool first_time = true;
    if (first_time) {
      first_time = false;
      parent->msg(MSG::INFO) << "SCTOverlay::mergeCollectionsNew(): "
                             << " SCT specific code is called for "
                             << typeid(*bkgCollection).name()
                             << endmsg;
    }

    // ----------------------------------------------------------------
    // Useful reading:
    //
    // Real data are handled like this:
    //
    // https://svnweb.cern.ch/trac/atlasoff/browser/InnerDetector/InDetEventCnv/SCT_RawDataByteStreamCnv/trunk/src/SCT_RodDecoder.cxx?rev=332542
    //
    // Note that SCT3 is hardcoded (no chance to get SCT1 from real data).
    // The SCT3 format is also the default in digitization:
    //
    // https://svnweb.cern.ch/trac/atlasoff/browser/InnerDetector/InDetDigitization/SCT_Digitization/trunk/src/SCT_Digitization.cxx?rev=355953

    // So we'll just go with SCT3.
    //
    // The client code that consumes SCT RDOs:
    //
    // http://alxr.usatlas.bnl.gov/lxr/source/atlas/InnerDetector/InDetRecTools/SiClusterizationTool/src/SCT_ClusteringTool.cxx


    // ----------------------------------------------------------------
    if (bkgCollection->identify() != signalCollection->identify()) {
      std::ostringstream os;
      os << "mergeCollectionsNew<SCT_RDORawData>(): collection Id mismatch";
      parent->msg(MSG::FATAL) << os.str() << endmsg;
      throw std::runtime_error(os.str());
    }

    const Identifier idColl = parent->get_sct_id()->wafer_id(signalCollection->identifyHash());

    // Empty the input collections and move RDOs to local vectors.
    InDetRawDataCollection<SCT_RDORawData> bkg(bkgCollection->identifyHash());
    bkg.setIdentifier(idColl);
    bkgCollection->swap(bkg);

    InDetRawDataCollection<SCT_RDORawData> sig(signalCollection->identifyHash());
    sig.setIdentifier(idColl);
    signalCollection->swap(sig);

    // Strip hit timing information for Next, Current, Previous and Any BCs
    // Prepare one more strip to create the last one. The additional strip has no hits.
    std::bitset<SCTOverlay::NumberOfStrips+1> stripInfo[SCTOverlay::NumberOfBitSets];
    // Process background and signal in the wafer
    for (unsigned source = SCTOverlay::BkgSource; source < SCTOverlay::NumberOfSources; source++) {
      InDetRawDataCollection<SCT_RDORawData>::const_iterator rdo;
      InDetRawDataCollection<SCT_RDORawData>::const_iterator rdoEnd;
      if (source == SCTOverlay::BkgSource) { // background
        rdo = bkg.begin();
        rdoEnd = bkg.end();
      } else { // signal
        rdo = sig.begin();
        rdoEnd = sig.end();
      } 
      // Loop over all RDOs in the wafer
      for (; rdo!=rdoEnd; ++rdo) {
        const SCT3_RawData* rdo3 = dynamic_cast<const SCT3_RawData*>(*rdo);
        if (!rdo3) {
          std::ostringstream os;
          const auto& elt = **rdo;
          os << "mergeCollectionNew<SCT_RDORawData>(): wrong datum format. Only SCT3_RawData are produced by SCT_RodDecoder and supported by overlay."
             << "For the supplied datum  typeid(datum).name() = " << typeid(elt).name();
          throw std::runtime_error(os.str());
        }
        int strip = parent->get_sct_id()->strip(rdo3->identify());
        int stripEnd = strip + rdo3->getGroupSize();
        int timeBin = rdo3->getTimeBin();
        for (; strip < stripEnd && strip < SCTOverlay::NumberOfStrips; strip++) {
          // Fill timing information for each strips, loop over 3 BCs
          for (unsigned int bc = SCTOverlay::NextBC; bc < SCTOverlay::NumberOfBCs; bc++) {
            if (timeBin & (1 << bc)) stripInfo[bc].set(strip);
          }
        }
      }
    }
    // Get OR for AnyBC, loop over 3 BCs
    for (unsigned int bc = SCTOverlay::NextBC; bc < SCTOverlay::NumberOfBCs; bc++) {
      stripInfo[SCTOverlay::AnyBC] |= stripInfo[bc];
    }
    // Check if we need to use Expanded mode by checking if there is at least one hit in Next BC or Previous BC
    bool anyNextBCHits = stripInfo[SCTOverlay::NextBC].any();
    bool anyPreivousBCHits = stripInfo[SCTOverlay::PreviousBC].any();
    bool isExpandedMode = (anyNextBCHits or anyPreivousBCHits);
    // No error information is recorded because this information is not filled in data and no errors are assumed in MC.
    const int ERRORS = 0;
    const std::vector<int> errvec{};
    if (isExpandedMode) {
      // Expanded mode (record strip one by one)
      const int groupSize = 1;
      int tbin = 0;
      for (unsigned int strip = 0; strip < SCTOverlay::NumberOfStrips; strip++) {
        if (stripInfo[SCTOverlay::AnyBC][strip]) {
          tbin = 0;
          for (unsigned int bc = SCTOverlay::NextBC; bc < SCTOverlay::NumberOfBCs; bc++) {
            if (stripInfo[bc][strip]) {
              tbin |= (1 << bc);
            }
          }
          unsigned int SCT_Word = (groupSize | (strip << 11) | (tbin <<22) | (ERRORS << 25));
          Identifier rdoId = parent->get_sct_id()->strip_id(idColl, strip) ;
          outputCollection->push_back(new SCT3_RawData(rdoId, SCT_Word, &errvec));
        }
      }
    } else {
      // We can record consecutive hits into one RDO if all hits have timeBin of 010.
      unsigned int groupSize = 0;
      const int tbin = (1 << SCTOverlay::CurrentBC);
      for (unsigned int strip=0; strip<SCTOverlay::NumberOfStrips+1; strip++) { // Loop over one more strip to create the last one if any
        if (stripInfo[SCTOverlay::AnyBC][strip]) {
          groupSize++;
        } else {
          if (groupSize>0) {
            unsigned int firstStrip = strip - groupSize;
            unsigned int SCT_Word = (groupSize | (firstStrip << 11) | (tbin <<22) | (ERRORS << 25));
            Identifier rdoId = parent->get_sct_id()->strip_id(idColl, firstStrip) ;
            outputCollection->push_back(new SCT3_RawData(rdoId, SCT_Word, &errvec));
            groupSize = 0;
          }
        }
      }
    }
  } // mergeCollectionsNew()
} // namespace Overlay


SCTOverlay::SCTOverlay(const std::string &name, ISvcLocator *pSvcLocator)
  : IDC_OverlayBase(name, pSvcLocator),
    m_sctId(nullptr)
{
  
}

StatusCode SCTOverlay::initialize()
{
  ATH_MSG_DEBUG("Initializing...");

  if (!m_includeBkg) {
    ATH_MSG_DEBUG("Disabling use of background RDOs...");
    ATH_CHECK( m_bkgInputKey.assign("") );
  }

  // Check and initialize keys
  ATH_CHECK( m_bkgInputKey.initialize(!m_bkgInputKey.key().empty()) );
  ATH_MSG_VERBOSE("Initialized ReadHandleKey: " << m_bkgInputKey);
  ATH_CHECK( m_signalInputKey.initialize() );
  ATH_MSG_VERBOSE("Initialized ReadHandleKey: " << m_signalInputKey);
  ATH_CHECK( m_outputKey.initialize() );
  ATH_MSG_VERBOSE("Initialized WriteHandleKey: " << m_outputKey);

  // Retrieve SCT ID helper
  if (!detStore()->retrieve(m_sctId, "SCT_ID").isSuccess() || !m_sctId) {
    ATH_MSG_FATAL("Cannot retrieve SCT ID helper");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode SCTOverlay::execute()
{
  ATH_MSG_DEBUG("execute() begin");
  
  // Reading the input RDOs
  ATH_MSG_VERBOSE("Retrieving input RDO containers");

  const SCT_RDO_Container *bkgContainerPtr = nullptr;
  if (m_includeBkg) {
    SG::ReadHandle<SCT_RDO_Container> bkgContainer(m_bkgInputKey);
    if (!bkgContainer.isValid()) {
      ATH_MSG_ERROR("Could not get background SCT RDO container " << bkgContainer.name() << " from store " << bkgContainer.store());
      return StatusCode::FAILURE;
    }
    bkgContainerPtr = bkgContainer.cptr();

    ATH_MSG_DEBUG("Found background SCT RDO container " << bkgContainer.name() << " in store " << bkgContainer.store());
    ATH_MSG_DEBUG("SCT Background = " << shortPrint(bkgContainer.cptr(), 50));
  }

  SG::ReadHandle<SCT_RDO_Container> signalContainer(m_signalInputKey);
  if (!signalContainer.isValid()) {
    ATH_MSG_ERROR("Could not get signal SCT RDO container " << signalContainer.name() << " from store " << signalContainer.store());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Found signal SCT RDO container " << signalContainer.name() << " in store " << signalContainer.store());
  ATH_MSG_DEBUG("SCT Signal     = " << shortPrint(signalContainer.cptr(), 50));

  // Creating output RDO container
  SG::WriteHandle<SCT_RDO_Container> outputContainer(m_outputKey);
  ATH_CHECK(outputContainer.record(std::make_unique<SCT_RDO_Container>(signalContainer->size())));
  ATH_MSG_DEBUG("Recorded output SCT RDO container " << outputContainer.name() << " in store " << outputContainer.store());

  if (outputContainer.isValid()) {
    overlayContainerNew(bkgContainerPtr, signalContainer.cptr(), outputContainer.ptr());

    ATH_MSG_DEBUG("SCT Result   = " << shortPrint(outputContainer.ptr(), 50));
  }

  ATH_MSG_DEBUG("execute() end");
  return StatusCode::SUCCESS;
}
