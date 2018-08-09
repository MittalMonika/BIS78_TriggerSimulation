///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// Implementation file for class BookkeeperTool
// Authors: Joao Firmino da Costa <joao.costa@cern.ch> and David Cote <david.cote@cern.ch>
///////////////////////////////////////////////////////////////////

#include "EventBookkeeperTools/BookkeeperTool.h"

// STL include
#include <algorithm>

// #include "FillEBCFromFlat.h"

#ifdef ASGTOOL_ATHENA
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/FileIncident.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "AthenaKernel/MetaCont.h"
#include "AthenaKernel/ClassID_traits.h"
#include "AthenaKernel/errorcheck.h"
#include "StoreGate/WriteMetaHandle.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthContainersInterfaces/IConstAuxStoreMeta.h"
#endif

BookkeeperTool::BookkeeperTool(const std::string& type, 
                               const std::string& name,
                               const IInterface*  parent)
  : AthAlgTool(type,name,parent),
    m_inputMetaStore( "StoreGateSvc/InputMetaDataStore", name ),
    m_outputMetaStore( "StoreGateSvc/MetaDataStore", name ),
    m_cutflowTaken(false),
    m_markIncomplete(true)
{
  declareProperty("OutputCollName", m_outputCollName="CutBookkeepers",  
    "The default name of the xAOD::CutBookkeeperContainer for output files");
  declareProperty("InputCollName", m_inputCollName = "CutBookkeepers",
    "The default name of the xAOD::CutBookkeeperContainer for input files");
  declareProperty("CutFlowCollName", m_cutflowCollName = "CutBookkeepersFile",
    "The default name of the xAOD::CutBookkeeperContainer for CutFlowSvc");
  declareProperty("MarkIncomplete", m_markIncomplete = true, 
    "Defaults to filling both complete and incomplete bookkeepers");
#ifdef ASGTOOL_ATHENA
  declareInterface< ::IMetaDataTool >( this );
#endif // ASGTOOL_ATHENA
}



BookkeeperTool::~BookkeeperTool()
{
}



StatusCode
BookkeeperTool::initialize()
{
  ATH_MSG_DEBUG( "Initializing " << name() << " - package version " << PACKAGE_VERSION );

  ATH_MSG_DEBUG("InputCollName = " << m_inputCollName);
  ATH_MSG_DEBUG("OutputCollName = " << m_outputCollName);
  ATH_MSG_DEBUG("CutFlowCollName = " << m_cutflowCollName);

  return StatusCode::SUCCESS;
}


//__________________________________________________________________________
/*
StatusCode BookkeeperTool::beginInputFile()
{
  return this->beginInputFile("Serial");
}
*/

#ifdef ASGTOOL_ATHENA
StatusCode BookkeeperTool::beginInputFile(const SG::SourceID& sid)
{
  ATH_MSG_INFO("beginInputFile " << this->name() << "\n" << outputMetaStore()->dump());
  //OPENING NEW INPUT FILE
  //Things to do:
  // 1) note that a file is currently opened
  // 2) Load CutBookkeepers from input file
  //    2a) if incomplete from input, directly propagate to output
  //    2b) if complete from input, wait for EndInputFile to decide what to do in output

  const std::string storename("MetaDataStore+");
  if (m_inputCollName != "") {  // are there inputs
    //  IF NO METACONT IN OUTPUT STORE YET
    //     Initialize MetaCont for incomplete and tmp containers in output store
    //
    std::string tmp_name = storename+m_outputCollName+"tmp";
    ATH_CHECK(buildAthenaInterface(m_inputCollName,tmp_name,sid));

    // Do the following if we want incomplete processings marked
    if (m_markIncomplete) {
      std::string inc_name = storename+"Incomplete"+m_outputCollName; 
      std::string input_inc_name = "Incomplete"+m_inputCollName;
      ATH_CHECK(buildAthenaInterface(input_inc_name,inc_name,sid));
    }
  } // inputCollName if

  // reset cutflow taken marker
  m_cutflowTaken = false;

  m_read.insert(sid);
  
  return StatusCode::SUCCESS;
}
#endif

/*
StatusCode BookkeeperTool::endInputFile()
{
  return this->endInputFile("Serial");
}
*/

//#ifdef ASGTOOL_ATHENA
StatusCode BookkeeperTool::endInputFile(const SG::SourceID& sid)
{
  // Add the sid to the list of complete sids
  if (m_inputCollName != "") {  // are there inputs
    m_fullreads.insert(sid);
  }

  return StatusCode::SUCCESS;
}

StatusCode BookkeeperTool::metaDataStop(const SG::SourceID&)
{
  const std::string storename("MetaDataStore+");
  if (m_inputCollName != "") {  // are there inputs
    //TERMINATING THE JOB (EndRun)
    //Things to do:
    // 1) Create new incomplete CutBookkeepers if relevant
    // 2) Print cut flow summary
    // 3) Write root file if requested
    // Now retrieve pointers for the MetaConts
    std::string tmp_name = storename+m_outputCollName+"tmpCont";
    const MetaCont<xAOD::CutBookkeeperContainer>* tmp;
    ATH_CHECK(outputMetaStore()->retrieve(tmp,tmp_name));
    xAOD::CutBookkeeperContainer* outcom = new xAOD::CutBookkeeperContainer();
    xAOD::CutBookkeeperAuxContainer* outcom_aux = new xAOD::CutBookkeeperAuxContainer();
    outcom->setStore(outcom_aux);

    if (m_markIncomplete) {
      std::string inc_name = storename+"Incomplete"+m_outputCollName+"Cont"; 
      // Build incomplete container to fill
      xAOD::CutBookkeeperContainer* outinc = new xAOD::CutBookkeeperContainer();
      xAOD::CutBookkeeperAuxContainer* outinc_aux = new xAOD::CutBookkeeperAuxContainer();
      outinc->setStore(outinc_aux);
      // Check if there were any incomplete inputs
      const MetaCont<xAOD::CutBookkeeperContainer>* inc;
      if(outputMetaStore()->retrieve(inc,inc_name).isSuccess()) {

        // Incomplete inputs can just be merged
        auto sids_inc = inc->sources();
        xAOD::CutBookkeeperContainer* contptr(nullptr);
        for (auto it = sids_inc.begin(); it != sids_inc.end(); ++it) {
          if (!inc->find(*it,contptr)) {
            ATH_MSG_ERROR("Container sid list did not match contents");
          } else {
            ATH_CHECK(updateContainer(outinc,contptr));
          }
          contptr = nullptr; 
        }
      } else {
        ATH_MSG_INFO("Did not find MetaCont for " << inc_name << ", assuming input had no incomplete bookkeepers");
      }

      // Loop over containers and mark based on end files seen
      auto sids_tmp = tmp->sources();
      xAOD::CutBookkeeperContainer* contptr(nullptr);
      for (auto it = sids_tmp.begin(); it != sids_tmp.end(); ++it) {
        if (!tmp->find(*it,contptr)) {
          ATH_MSG_ERROR("Container sid list did not match contents");
        } else {
          bool complete = std::find(m_fullreads.begin(),
                       m_fullreads.end(),
                       *it) != m_fullreads.end();
          bool not_written = std::find(m_written.begin(),
                          m_written.end(),
                          *it) == m_written.end();
          if (complete && not_written) {
            ATH_CHECK(updateContainer(outcom,contptr));
          } else {
            ATH_CHECK(updateContainer(outinc,contptr));
          }
        }
      }

      std::string incout_name = "Incomplete"+m_outputCollName;
      // Do any cleanup
/*
*/
      if (outputMetaStore()->contains<xAOD::CutBookkeeperContainer>(incout_name) ) {
        ATH_MSG_INFO("Cleaning up xAOD::CutBookkeeperContainer for " << incout_name);
        const xAOD::CutBookkeeperContainer* tmpBook(nullptr);
        if ( outputMetaStore()->retrieve(tmpBook,incout_name).isSuccess() ) {
          const SG::IConstAuxStore* tmpBookAux = tmpBook->getConstStore();
          //const xAOD::CutBookkeeperAuxContainer* tba = (const xAOD::CutBookkeeperAuxContainer*)tmpBookAux;
          ATH_CHECK(outputMetaStore()->removeDataAndProxy(tmpBook));
          ATH_CHECK(outputMetaStore()->removeDataAndProxy(tmpBookAux));
        }
        else ATH_MSG_ERROR("StoreGate failed retrieve after contains=true");
      }
      ATH_MSG_INFO("GLARB incout_name" << incout_name);
      ATH_CHECK(outputMetaStore()->record(outinc,incout_name));
      ATH_CHECK(outputMetaStore()->record(outinc_aux,incout_name+"Aux."));
    }  // markIncomplete
    else {
      auto sids_tmp = tmp->sources();
      xAOD::CutBookkeeperContainer* contptr(nullptr);
      // just merge complete inputs into complete/output container
      for (auto it = sids_tmp.begin(); it != sids_tmp.end(); ++it) {
        if (!tmp->find(*it,contptr)) {
          ATH_MSG_ERROR("Container sid list did not match contents");
        } else {
          // default to not worrying about marking
          ATH_CHECK(updateContainer(outcom,contptr));
        }
      }
    }

    // Record container objects directly in store for output
/*
*/
    if (outputMetaStore()->contains<xAOD::CutBookkeeperContainer>(m_outputCollName) ) {
      ATH_MSG_INFO("Cleaning up xAOD::CutBookkeeperContainer for " << m_outputCollName);
      const xAOD::CutBookkeeperContainer* tmpBook(nullptr);
      if ( outputMetaStore()->retrieve(tmpBook,m_outputCollName).isSuccess() ) {
        const SG::IConstAuxStore* tmpBookAux = tmpBook->getConstStore();
        ATH_CHECK(outputMetaStore()->removeDataAndProxy(tmpBook));
        ATH_CHECK(outputMetaStore()->removeDataAndProxy(tmpBookAux));
      }
      else ATH_MSG_ERROR("StoreGate failed retrieve after contains=true");
    }
    ATH_MSG_INFO("GLARB m_outputCollName " << m_outputCollName);
    ATH_CHECK(outputMetaStore()->record(outcom,m_outputCollName));
    ATH_CHECK(outputMetaStore()->record(outcom_aux,m_outputCollName+"Aux."));
  } // inputCollName if

  if (!m_cutflowTaken) {
    if (addCutFlow().isFailure()) {
      ATH_MSG_ERROR("Could not add CutFlow information");
    }
  }
  else {
    ATH_MSG_DEBUG("Cutflow information written into container before metaDataStop");
  }
  //ATH_MSG_INFO("BLARG 2 " << outputMetaStore()->dump());

  // Reset after metadata stop
  m_cutflowTaken = false;

  if (m_inputCollName != "") {  // are there inputs
    // Copy read files into written files
    //std::copy(m_read.begin(),m_read.end(),back_inserter(m_written));
    for (auto it = m_read.begin(); it != m_read.end(); ++it) {
      m_written.insert(*it);
    }
    // Remove completes from read
    for (auto it = m_fullreads.begin(); it != m_fullreads.end(); ++it) {
      m_read.erase(*it);
      //std::remove(m_read.begin(); m_read.end(), *it);
    }
    m_fullreads.clear();
  } // inputCollName if
  
  return StatusCode::SUCCESS;
}
//#endif

/*
StatusCode BookkeeperTool::metaDataStop()
{
  return this->metaDataStop("Serial");
}
*/

StatusCode
BookkeeperTool::finalize()
{
  ATH_MSG_DEBUG( "Finalizing " << name() << " - package version " << PACKAGE_VERSION );
  return StatusCode::SUCCESS;
}


StatusCode BookkeeperTool::initOutputContainer( const std::string& sgkey)
{
  // Append datastore name to key
  //std::string key = "MetaDataStore+"+sgkey;
  std::string key = sgkey;
  // Create the primary container
  //xAOD::CutBookkeeperContainer* coll = new xAOD::CutBookkeeperContainer();
  //xAOD::CutBookkeeperAuxContainer* auxcoll = new xAOD::CutBookkeeperAuxContainer();
  //coll->setStore(auxcoll);
  // Put it in a MetaCont
  MetaCont<xAOD::CutBookkeeperContainer>* mcont = new MetaCont<xAOD::CutBookkeeperContainer>(DataObjID("xAOD::CutBookkeeperContainer",key));
  // Do the same for the auxiliary container
  std::string auxkey(key+"Aux.");
  MetaCont<xAOD::CutBookkeeperAuxContainer>* acont = new MetaCont<xAOD::CutBookkeeperAuxContainer>(DataObjID("xAOD::CutBookkeeperAuxContainer",auxkey));
  ATH_MSG_INFO("GLARB mcont key " << key);
  ATH_MSG_INFO("GLARB mcont auxkey " << auxkey);
  ATH_CHECK(outputMetaStore()->record(std::move(mcont),key));
  ATH_CHECK(outputMetaStore()->record(std::move(acont),auxkey));
  //ATH_MSG_INFO("BLARG 1 " << outputMetaStore()->dump());
  /*
  */
  ATH_CHECK(outputMetaStore()->symLink
          (
            ClassID_traits<MetaCont<xAOD::CutBookkeeperAuxContainer> >::ID(),
            auxkey,
            ClassID_traits<xAOD::CutBookkeeperAuxContainer>::ID()
          ));
  ATH_MSG_INFO("BLARG 2 " << outputMetaStore()->dump());

  return StatusCode::SUCCESS;
}

//--------------------------------------------------------//
//  MetaConts are only needed when reading in Athena
//  This builds them and populates them with bookeepers from the input store
//--------------------------------------------------------//
StatusCode BookkeeperTool::buildAthenaInterface(const std::string& inputName,
                                                const std::string& outputName,
                                                const SG::SourceID& sid)
{
  // Make sure the MetaCont is ready in the output store for outputName
  //   If not, then create it
  std::string name = outputName+"Cont"; 
  if( !(outputMetaStore()->contains<MetaCont<xAOD::CutBookkeeperContainer> >(name)) ) {
    ATH_CHECK(this->initOutputContainer(name));
  }
  else {
    ATH_MSG_WARNING("incomplete collection already exists");
  }

  // Retrieve pointer for the MetaCont
  MetaCont<xAOD::CutBookkeeperContainer>* mc;
  ATH_CHECK(outputMetaStore()->retrieve(mc,name));

  // Make sure sid does not already exist in the MetaCont
  if ( std::find(mc->sources().begin(),mc->sources().end(),sid) 
                                        != mc->sources().end() ) {
    ATH_MSG_ERROR("Metadata already exists for sid " << sid);
    return StatusCode::FAILURE;   // Fail if sid already exists
  }

  // Get the input bookkeeper of the input metadata store
  const xAOD::CutBookkeeperContainer* cbc;
  if (inputMetaStore()->contains<xAOD::CutBookkeeperContainer>(inputName) ) {
    ATH_MSG_INFO("BLARG input " << inputMetaStore()->dump());
    StatusCode ssc = inputMetaStore()->retrieve( cbc, inputName );
    if (ssc.isSuccess()) {
      // Insert input bookkeeper into MetaCont for this sid
      xAOD::CutBookkeeperContainer* tostore = new xAOD::CutBookkeeperContainer(*cbc);
      if ( !mc->insert(sid,tostore) ) {
        ATH_MSG_ERROR("Unable to insert " << inputName << " for " << sid << " with key " << name);
        return StatusCode::FAILURE;   // Fail if insert to mc fails
      }
    }
    else {
      ATH_MSG_ERROR("Could not retrieve CutBookkeeperContainer with name " << inputName << " in input store");
      return StatusCode::FAILURE;   // Fail if store contains, but not retrieves
    }
  }
  else {
    ATH_MSG_WARNING("No " << inputName << " bookkeepers in this file ");
  }

  return StatusCode::SUCCESS;
}

StatusCode BookkeeperTool::addCutFlow()
{
  // Add the information from the current processing to the complete output
  // --> same paradigm as original CutFlowSvc
  // Get the complete bookkeeper collection of the output meta-data store
  xAOD::CutBookkeeperContainer* completeBook(NULL); 
  if( !(outputMetaStore()->retrieve( completeBook, m_outputCollName) ).isSuccess() ) {
    ATH_MSG_ERROR( "Could not get complete CutBookkeepers from output MetaDataStore" );
    return StatusCode::FAILURE;
  }

  // Get the bookkeeper from the current processing
  const xAOD::CutBookkeeperContainer* fileCompleteBook(NULL);
  if( outputMetaStore()->contains<xAOD::CutBookkeeperContainer>(m_cutflowCollName) ) {
    if( !(outputMetaStore()->retrieve( fileCompleteBook, m_cutflowCollName) ).isSuccess() ) {
      ATH_MSG_WARNING( "Could not get CutFlowSvc CutBookkeepers from output MetaDataStore" );
    }
    else {
      // update the complete output with the complete input
      ATH_CHECK(this->updateContainer(completeBook,fileCompleteBook));
    }
  }
  else {
    ATH_MSG_INFO("No cutflow container " << m_cutflowCollName);
  }

  return StatusCode::SUCCESS;
}


namespace {


xAOD::CutBookkeeper*
resolveLink (const xAOD::CutBookkeeper* old,
             xAOD::CutBookkeeperContainer& contToUpdate,
             const xAOD::CutBookkeeperContainer& otherCont,
             const std::vector<size_t>& otherIndices)
{
  {
    xAOD::CutBookkeeperContainer::iterator matchIter = 
      std::find( contToUpdate.begin(),
                 contToUpdate.end(),
                 old );
    if (matchIter != contToUpdate.end())
      return *matchIter;
  }

  {
    xAOD::CutBookkeeperContainer::const_iterator matchIter = 
      std::find( otherCont.begin(),
                 otherCont.end(),
                 old );
    if (matchIter != contToUpdate.end()) {
      size_t pos = matchIter - otherCont.begin();
      return contToUpdate[otherIndices[pos]];
    }
  }

  // If we didn't find it, we need to add it
  xAOD::CutBookkeeper* newEBK = new xAOD::CutBookkeeper();
  if ( newEBK->usingPrivateStore() ) { newEBK->releasePrivateStore(); }
  newEBK->makePrivateStore(old);
  contToUpdate.push_back( newEBK );
  return newEBK;
}


} // anonymous namespace


StatusCode
BookkeeperTool::updateContainer( xAOD::CutBookkeeperContainer* contToUpdate,
                             const xAOD::CutBookkeeperContainer* otherCont ) 
{
  ATH_MSG_DEBUG("calling updateContainer(...)" );
  ATH_MSG_VERBOSE("Have container to update with size=" << contToUpdate->size()
                  << ", and other container with size=" << otherCont->size() );

  size_t origSize = contToUpdate->size();

  // Vector of indices in contToUpdate of elements in otherCont.
  std::vector< std::size_t > otherIndices (otherCont->size());
  // Loop through otherCont.
  // If element already in contToUpdate, update event counts, otherwise create new element
  for ( std::size_t i=0; i<otherCont->size(); ++i ) {
    const xAOD::CutBookkeeper* otherEBK = otherCont->at(i);
    ATH_MSG_VERBOSE("Looping through otherCont at index " << i);
    ATH_MSG_VERBOSE("Have otherEBK with: name=" << otherEBK->name()
                    << ", cycle=" << otherEBK->cycle()
                    << ", nAcceptedEvents=" << otherEBK->nAcceptedEvents()
                    << ", inputStream=" << otherEBK->inputStream() );


    // Loop through the container to be updated (contToUpdate) and see if we find a match
    bool foundEBKToUpdate(false);
    for ( std::size_t j=0; j<contToUpdate->size(); ++j ) {
      xAOD::CutBookkeeper* ebkToUpdate = contToUpdate->at(j);
      // Check if they are identical, if so, update; else add otherEBK
      if ( otherEBK->isEqualTo(ebkToUpdate) ) {
        ebkToUpdate->setPayload( ebkToUpdate->payload() + otherEBK->payload() );
        otherIndices[i] = j;
        foundEBKToUpdate = true;
        break;
      }
    } // End: Inner loop over contToUpdate
    if (!foundEBKToUpdate) {
      xAOD::CutBookkeeper* newEBK = new xAOD::CutBookkeeper();
      if ( newEBK->usingPrivateStore() ) { newEBK->releasePrivateStore(); }
      newEBK->makePrivateStore(otherEBK);
      contToUpdate->push_back( newEBK );
      std::size_t ebIdx = newEBK->index();
      otherIndices[i] = ebIdx;
    }
  } // End: Outer loop over contToUpdate

  // Now, we still need to fix the cross-referencing of the newly added CutBookkkeepers
  for ( std::size_t i=origSize; i<contToUpdate->size(); ++i ) {
    xAOD::CutBookkeeper* ebkToModify = contToUpdate->at(i);

    // Parent check
    if ( ebkToModify->hasParent() ) {
      const xAOD::CutBookkeeper* oldParent = ebkToModify->parent();
      const xAOD::CutBookkeeper* newParent = resolveLink (oldParent,
                                                          *contToUpdate,
                                                          *otherCont,
                                                          otherIndices);
      ebkToModify->setParent (newParent);
    } // Done fixing parent

    // Children check
    std::vector< xAOD::CutBookkeeper* > newChildren;
    for ( std::size_t oldIdx=0; oldIdx<ebkToModify->nChildren(); ++oldIdx ) {
      const xAOD::CutBookkeeper* oldEBK = ebkToModify->child(oldIdx);
      newChildren.push_back (resolveLink (oldEBK,
                                          *contToUpdate,
                                          *otherCont,
                                          otherIndices));
    } // Done fixing children
    ebkToModify->setChildren (newChildren);

    // Used others check
    std::vector< xAOD::CutBookkeeper* > newOthers;
    for ( std::size_t oldIdx=0; oldIdx<ebkToModify->nUsedOthers(); ++oldIdx ) {
      const xAOD::CutBookkeeper* oldEBK = ebkToModify->usedOther(oldIdx);
      newOthers.push_back (resolveLink (oldEBK,
                                        *contToUpdate,
                                        *otherCont,
                                        otherIndices));
    } // Done fixing used others
    ebkToModify->setUsedOthers (newOthers);

    // Siblings check
    std::vector< xAOD::CutBookkeeper* > newSiblings;
    for ( std::size_t oldIdx=0; oldIdx<ebkToModify->nSiblings(); ++oldIdx ) {
      const xAOD::CutBookkeeper* oldEBK = ebkToModify->sibling(oldIdx);
      newSiblings.push_back (resolveLink (oldEBK,
                                          *contToUpdate,
                                          *otherCont,
                                          otherIndices));
    } // Done fixing siblings
    ebkToModify->setSiblings (newSiblings);
  } // Done fixing all cross references
  return StatusCode::SUCCESS;
}

