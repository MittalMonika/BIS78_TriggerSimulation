/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGOUTPUTHANDLING_TriggerEDMSerialiserTool_H
#define TRIGOUTPUTHANDLING_TriggerEDMSerialiserTool_H


#include <string>
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "xAODCore/AuxSelection.h"
#include "AthenaKernel/IClassIDSvc.h"
#include "AthenaKernel/IAthenaSerializeSvc.h"
#include "AthenaKernel/IDictLoaderSvc.h"
#include "TrigOutputHandling/HLTResultMTMakerTool.h"

/**
 * @class TriggerEDMSerialiserTool is tool responsible for creation of HLT Result filled with streamed EDM collections
 **/

class TriggerEDMSerialiserTool: public extends<AthAlgTool, HLTResultMTMakerTool>
{ 
  
 public: 

  TriggerEDMSerialiserTool( const std::string& type,
	     const std::string& name, 
	     const IInterface* parent );

  virtual ~TriggerEDMSerialiserTool(); 
  virtual StatusCode fill( HLT::HLTResultMT& resultToFill ) const override;

  virtual StatusCode  initialize() override;

 private: 
  Gaudi::Property<std::vector<std::string>> m_collectionsToSerialize { this, "CollectionsToSerialize", {}, "TYPE#SG.aux1.aux2..etc key of collections to be streamed (like in StreamAOD), the type has to be an exact type i.e. with _vN not the alias type" };


  Gaudi::Property<int> m_moduleID { this, "ModuleID", 0, "The HLT result fragment to which the output should be added"};
  
  // internal structure to keep configuration organised conveniently
  struct Address {
    std::string type;
    CLID clid;
    RootType rt;
    std::string key;
    xAOD::AuxSelection sel;
  };
  
  std::vector< Address > m_toSerialize; // postprocessed configuration info
  
  ServiceHandle<IClassIDSvc> m_clidSvc{ this, "ClassIDSvc", "ClassIDSvc", "Service to translate class name to CLID" };
  ServiceHandle<IAthenaSerializeSvc> m_serializerSvc{ this, "Serializer", "AthenaRootSerializeSvc", "Service that translates transient to persistent respresenation" };


  /**
   * Given the ID if the collection (in address arg) insert basic streaming info into the buffer.
   */
  StatusCode makeHeader( const TriggerEDMSerialiserTool::Address& address, std::vector<uint32_t>& buffer  ) const;

  /**
   * For copy bytest from the memory into the buffer converting from char[] to uint32_t[]
   * This function is candidate to be made global function at some point
   * and we will need also readPayload function
   */  
  StatusCode fillPayload( const void* data, size_t sz, std::vector<uint32_t>& buffer ) const ;

  
}; 


#endif //> !TRIGOUTPUTHANDLING_TriggerEDMSerialiserTool_H
