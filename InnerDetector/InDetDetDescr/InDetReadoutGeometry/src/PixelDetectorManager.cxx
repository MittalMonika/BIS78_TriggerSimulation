/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "DetDescrConditions/AlignableTransformContainer.h"
#include "DetDescrConditions/AlignableTransform.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"

#include "InDetReadoutGeometry/PixelDetectorManager.h"
#include "InDetIdentifier/PixelID.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetReadoutGeometry/ExtendedAlignableTransform.h"
#include "InDetReadoutGeometry/PixelModuleDesign.h"
#include "StoreGate/StoreGateSvc.h"

namespace InDetDD {

  const int FIRST_HIGHER_LEVEL = 1;


  PixelDetectorManager::PixelDetectorManager(StoreGateSvc* detStore) 
    : SiDetectorManager(detStore, "Pixel"),
    m_idHelper(0)
//        m_isLogical(false) // Change to true to change the definition of local module corrections
  {

    //  
    // Initialized the Identifier helper.
    //

    StatusCode sc = detStore->retrieve(m_idHelper,"PixelID");
    if (sc.isFailure() ) {
      msg(MSG::ERROR) << "Could not retrieve Pixel id helper" << endreq;
    }

    // Initialize the collections.
    if (m_idHelper) {
      m_elementCollection.resize(m_idHelper->wafer_hash_max());
      m_alignableTransforms.resize(m_idHelper->wafer_hash_max());
    } 
  }


  PixelDetectorManager::~PixelDetectorManager()
  {
    // Clean up
    SiDetectorElementCollection::iterator iter;
    for (iter = m_elementCollection.begin(); iter != m_elementCollection.end(); ++iter){
      delete *iter;
    }

    for (size_t i=0; i < m_volume.size(); i++) {
      m_volume[i]->unref();
    }

    for (size_t j=0; j < m_higherAlignableTransforms.size(); j++){
      AlignableTransformMap::iterator iterMap;  
      for (iterMap = m_higherAlignableTransforms[j].begin(); 
      iterMap != m_higherAlignableTransforms[j].end();
      ++iterMap) {
        delete iterMap->second;
      }
    }

    for (size_t k=0; k < m_alignableTransforms.size(); k++){
      delete m_alignableTransforms[k];
    }
  }



  unsigned int PixelDetectorManager::getNumTreeTops() const
  {
    return m_volume.size(); 
  }

  PVConstLink PixelDetectorManager::getTreeTop(unsigned int i) const
  {
    return m_volume[i];
  }

  void PixelDetectorManager::addTreeTop(PVLink vol){
    vol->ref();
    m_volume.push_back(vol);
  }


  SiDetectorElement* PixelDetectorManager::getDetectorElement(const Identifier & id) const
  {  
  // NB the id helpers implementation for getting a hash is not optimal.
  // Essentially does a binary search.
  // Make sure it is a wafer Id
    Identifier waferId = id;
#ifndef __IDENTIFIER_64BIT__
  // pixel ids have special treatement. It is possible to distinguish between
  // a wafer id and a pixel cell id by checking bit 32, which is set for a pixel cell id.
    if (id.is_valid() && (id.get_compact() & 0x80000000)) {
      waferId =  m_idHelper->wafer_id(id);
    }
#else  
    waferId =  m_idHelper->wafer_id(id);
#endif /* __IDENTIFIER_64BIT__ */
    IdentifierHash idHash = m_idHelper->wafer_hash(waferId);
    if (idHash.is_valid()) {
      return m_elementCollection[idHash];
    } else {
      return 0;
    }
  }


  SiDetectorElement*  PixelDetectorManager::getDetectorElement(const IdentifierHash & idHash) const
  {
    return m_elementCollection[idHash];
  }

  SiDetectorElement* PixelDetectorManager::getDetectorElement(int barrel_endcap, int layer_wheel, int phi_module, int eta_module) const
  {
    return getDetectorElement(m_idHelper->wafer_id(barrel_endcap, layer_wheel, phi_module, eta_module));
  }


  const SiDetectorElementCollection* PixelDetectorManager::getDetectorElementCollection() const
  { 
    return &m_elementCollection;
  }

  SiDetectorElementCollection::const_iterator PixelDetectorManager::getDetectorElementBegin() const
  {
    return m_elementCollection.begin();
  }

  SiDetectorElementCollection::const_iterator PixelDetectorManager::getDetectorElementEnd() const
  {
    return m_elementCollection.end();
  }


  void PixelDetectorManager::addDetectorElement(SiDetectorElement * element)
  {
    IdentifierHash idHash = element->identifyHash();
    if (idHash >=  m_elementCollection.size())
      throw std::runtime_error("PixelDetectorManager: Error adding detector element.");
    m_elementCollection[idHash] = element;
  }

  void PixelDetectorManager::initNeighbours()
  {
    SiDetectorElementCollection::iterator iter;

    // Loop over all elements and set the neighbours
    for (iter = m_elementCollection.begin(); iter != m_elementCollection.end(); ++iter){

      SiDetectorElement * element = *iter;
      if (element) {

        IdentifierHash idHash = element->identifyHash();
        IdentifierHash idHashOther;

        int result;
        // If no neighbour, result != 0 in which case we leave neighbour as null
        result = m_idHelper->get_next_in_eta(idHash, idHashOther);
        if (result==0) element->setNextInEta(m_elementCollection[idHashOther]);

        result = m_idHelper->get_prev_in_eta(idHash, idHashOther);
        if (result==0) element->setPrevInEta(m_elementCollection[idHashOther]);

        result = m_idHelper->get_next_in_phi(idHash, idHashOther);
        if (result==0) element->setNextInPhi(m_elementCollection[idHashOther]);

        result = m_idHelper->get_prev_in_phi(idHash, idHashOther);
        if (result==0) element->setPrevInPhi(m_elementCollection[idHashOther]);
      }
    }
  }


  const PixelID* PixelDetectorManager::getIdHelper() const
  {
    return m_idHelper;
  }

  unsigned int PixelDetectorManager::getBarrelLayers() const
  {
    unsigned int barrelLayers = 0;
    for (int i = numerology().numLayers() - 1; i >= 0; --i) {
      barrelLayers *= 10;
      barrelLayers += numerology().useLayer(i);
    }
    return barrelLayers;
  }

  unsigned int PixelDetectorManager::getEndcapLayers() const
  {
    unsigned int endcapLayers = 0;
    for (int i = numerology().numDisks() - 1; i >= 0; --i) {
      endcapLayers *= 10;
      endcapLayers += numerology().useDisk(i);
    }
    return endcapLayers;
  }


  bool PixelDetectorManager::setAlignableTransformDelta(int level, 
                              const Identifier & id, 
                              const Amg::Transform3D & delta,
                              FrameType frame) const
  {

    if (level == 0) { // At the element level - local shift

      // We retrieve it via a hashId.
      IdentifierHash idHash = m_idHelper->wafer_hash(id);
      if (!idHash.is_valid()) return false;

      if (frame == InDetDD::global) {

        return setAlignableTransformGlobalDelta(m_alignableTransforms[idHash], delta);

      } else if (frame == InDetDD::local) {

        const SiDetectorElement * element =  m_elementCollection[idHash];
        if (!element) return false;

        // Its a local transform
        //See header file for definition of m_isLogical          
        //if( m_isLogical ){
        //    //Ensure cache is up to date and use the alignment corrected local to global transform
        //    element->updateCache();
        //return setAlignableTransformLocalDelta(m_alignableTransforms[idHash], element->transform(), delta);
        //} else 
        //Use default local to global transform
        return setAlignableTransformLocalDelta(m_alignableTransforms[idHash], element->defTransform(), delta);
      } else {
        // other not supported
        msg(MSG::WARNING) << "Frames other than global or local are not supported." << endreq;
        return false;
      }

    } else { // higher level

      if (frame != InDetDD::global) {
        msg(MSG::WARNING) << "Non global shift at higher levels is not possible." << endreq;
        return false;
      }

      int index = level - FIRST_HIGHER_LEVEL; // level 0 is treated separately.
      if (index >=  static_cast<int>(m_higherAlignableTransforms.size())) return false;

      // We retrieve it from a map. 
      AlignableTransformMap::const_iterator iter;    
      iter = m_higherAlignableTransforms[index].find(id);
      if (iter == m_higherAlignableTransforms[index].end()) return false;      

      // Its a global transform
      return setAlignableTransformGlobalDelta(iter->second, delta);
    }

  }


  void PixelDetectorManager::addAlignableTransform (int level, 
                            const Identifier & id, 
                            GeoAlignableTransform *transform,
                            const GeoVPhysVol * child)
  {
    if (m_idHelper) {

      const GeoVFullPhysVol * childFPV = dynamic_cast<const GeoVFullPhysVol *>(child);
      if (!childFPV) { 
        msg(MSG::ERROR) << "Child of alignable transform is not a full physical volume" 
          << endreq;
      } else {
        addAlignableTransform (level, id, transform, childFPV);
      }
    }
  }

  void PixelDetectorManager::addAlignableTransform (int level, 
                            const Identifier & id, 
                            GeoAlignableTransform *transform,
                            const GeoVFullPhysVol * child)
  {
    if (m_idHelper) {
      if (level == 0) {
        IdentifierHash idHash = m_idHelper->wafer_hash(id);
        if (idHash.is_valid()) {
          m_alignableTransforms[idHash]= new ExtendedAlignableTransform(transform, child);
        } 
      } else {
        // Higher levels are saved in a map. NB the index is level-1 as level=0 is treated above.   
        int index = level - FIRST_HIGHER_LEVEL; 
        if (index >= static_cast<int>(m_higherAlignableTransforms.size())) m_higherAlignableTransforms.resize(index+1); 
        m_higherAlignableTransforms[index][id] = new ExtendedAlignableTransform(transform, child);
      }
    }  
  }

  bool PixelDetectorManager::identifierBelongs(const Identifier & id) const
  {
    return getIdHelper()->is_pixel(id);
  }



  const PixelModuleDesign* PixelDetectorManager::getPixelDesign(int i) const
  {
    return dynamic_cast<const PixelModuleDesign *>(getDesign(i));
  }



  // The implementation of the new IBLDist DB; 
  // Specific for IBL -> maybe make it different function in future to be more general 
  bool PixelDetectorManager::processSpecialAlignment(const std::string & key) const
  {

  bool alignmentChange = false;

  if(msgLvl(MSG::INFO))
    msg(MSG::INFO) << "Processing IBLDist alignment container." << endreq;

  float ibldist[14] = {}; 
  float iblbaseline[14] = {}; 
  const CondAttrListCollection* atrlistcol=0;
  if (StatusCode::SUCCESS==m_detStore->retrieve(atrlistcol,key)) {
    // loop over objects in collection
    for (CondAttrListCollection::const_iterator citr=atrlistcol->begin(); citr!=atrlistcol->end();++citr) {
  
  const coral::AttributeList& atrlist=citr->second;
  ibldist[atrlist["stave"].data<int>()] = atrlist["mag"].data<float>();
  iblbaseline[atrlist["stave"].data<int>()] = atrlist["base"].data<float>();

  msg(MSG::VERBOSE) << "IBLDist DB -- channel: " << citr->first
        << " ,stave: " << atrlist["stave"].data<int>() 
        << " ,mag: " << atrlist["mag"].data<float>()
        << " ,base: " << atrlist["base"].data<float>() << endreq;
    }
  }
  else {
    if (msgLvl(MSG::INFO))
  msg(MSG::INFO) << "Cannot find IBLDist Container for key "
         << key << " - no IBL bowing alignment " << endreq;
    return alignmentChange;                                   
  }

  /**
    Matthias D. (Oct. 2016): 
    The idea of this first implementation is to get the AlignTransforms for IBL modules from their folder;
    Calculate from the new DB entries the Tx displacement for this module;
    Make a simple transfrom in local frame;
    Add the Tx to the aligntransform;
    Apply this new transform with the setAlignableTransformDelta() function;
    This is non-optimal as it simply overrides the previous Delta (these are included in total Delta);
    Possibly we could extend this in future to add tweak() functionality to GeoAlignableTransform?
  **/

  const AlignableTransformContainer* container;
  if (StatusCode::SUCCESS!=m_detStore->retrieve(container, "/Indet/Align")) {      
    msg(MSG::ERROR) << "Cannot find AlignableTransformContainer for key " 
        << key << " - no misalignment" << endreq;
    // This should not occur in normal situations so we force job to abort.
    throw std::runtime_error("Unable to apply Inner Detector alignments");
  }
  // Check if container is empty - this can occur if it is an invalid IOV.
  if (container->empty()) {
    msg(MSG::ERROR) << "AlignableTransformContainer for key " 
            << key << " is empty. Probably due to out of range IOV" << endreq;
    // This should not occur in normal situations so we force job to abort.
    throw std::runtime_error("Unable to apply Inner Detector alignments.");
  }
  // loop over all the AlignableTransform objects in the collection
  for (DataVector<AlignableTransform>::const_iterator pat=container->begin();
   pat!=container->end();++pat) {
    if ((*pat)->tag()!="/Indet/Align/PIXB1") {  // hard-coded to IBL for now
      msg(MSG::VERBOSE) << "IBLDist; ignoring collections " << (*pat)->tag() << endreq;
    }
    else{
      const AlignableTransform* transformCollection = *pat;
      for (AlignableTransform::AlignTransMem_citr trans_iter = transformCollection->begin(); 
         trans_iter != transformCollection->end(); 
         ++trans_iter) {
        if (msgLvl(MSG::DEBUG)) {
        msg(MSG::DEBUG) << "IBLDist alignment for identifier " 
             << getIdHelper()->show_to_string(trans_iter->identify())   << endreq;
        }
  
        IdentifierHash idHash = getIdHelper()->wafer_hash(trans_iter->identify());
        SiDetectorElement * sielem = m_elementCollection[idHash];
        //This should work as Bowing is in L3 frame, i.e. local module frame                 
        double z = sielem->center()[2];
        const double  y0y0  = 366.5*366.5;
                 
        double bowx = ibldist[getIdHelper()->phi_module(trans_iter->identify())] * ( z*z - y0y0 ) / y0y0;
        double basex= iblbaseline[getIdHelper()->phi_module(trans_iter->identify())];
        // This is in the module frame, as bowing corrections are directly L3                
  
        msg(MSG::DEBUG) << "Total IBL-module Tx shift (baseline+bowing): " << basex+bowx << endreq;
        if ( (basex+bowx)==0 ) continue; // make sure we ignore NULL corrections

        Amg::Transform3D shift = Amg::Translation3D(basex+bowx,0,0) * Amg::RotationMatrix3D::Identity();

        const AlignableTransform* cpat = *pat;
        AlignableTransform *pat_update=const_cast<AlignableTransform*>(cpat);
        AlignableTransform::AlignTransMem_citr this_trans=pat_update->findIdent(trans_iter->identify());
        HepGeom::Transform3D newtrans = Amg::EigenTransformToCLHEP(shift)*this_trans->transform();

        /** Verbose level debug section for transforms **/
        if (msgLvl(MSG::VERBOSE)) {
          msg(MSG::VERBOSE) << "Bowing Transformation only:" << endreq;
          printTransform(shift);
          msg(MSG::VERBOSE) << "Original alignable Transformation from StoreGate:" << endreq;
          printTransform(Amg::CLHEPTransformToEigen(this_trans->transform()));
          msg(MSG::VERBOSE) << "Final mModified Transformation:" << endreq;
          printTransform(Amg::CLHEPTransformToEigen(newtrans));
        }
        /** End of verbose level debug section **/
    
        // Set the new transform; Will replace existing one with updated transform
        bool status = setAlignableTransformDelta(0, 
                   trans_iter->identify(),
                   Amg::CLHEPTransformToEigen(newtrans),
                   InDetDD::local);
 
        if (!status) {
          if (msgLvl(MSG::DEBUG)) {
            msg(MSG::DEBUG) << "Cannot set AlignableTransform for identifier."  
                << getIdHelper()->show_to_string(trans_iter->identify())  
                << " at level 0 for IBLDist bowing deformation " << endreq;
          }
        }

        alignmentChange = (alignmentChange || status);
      }
    }
  }
  

  return alignmentChange;
  }


  // Helpful function for debugging of transforms
  void PixelDetectorManager::printTransform(const Amg::Transform3D & tr) const
  {
  msg(MSG::DEBUG) <<" - translation: "<<tr.translation().x()<<"  "<<tr.translation().y()<<"  "<<tr.translation().z() << endreq;
  msg(MSG::DEBUG) <<" - rotation:"<< endreq;
  msg(MSG::DEBUG) <<"    "<<tr(0,0)<<"  "<<tr(0,1)<<"  "<<tr(0,2)<< endreq;
  msg(MSG::DEBUG) <<"    "<<tr(1,0)<<"  "<<tr(1,1)<<"  "<<tr(1,2)<< endreq;
  msg(MSG::DEBUG) <<"    "<<tr(2,0)<<"  "<<tr(2,1)<<"  "<<tr(2,2)<< endreq;
  return;
  }


} // namespace InDetDD


