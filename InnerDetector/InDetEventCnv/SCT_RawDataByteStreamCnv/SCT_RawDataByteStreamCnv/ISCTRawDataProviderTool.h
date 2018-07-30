/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SCT_RAWDATABYTESTREAMCNV_ISCTRAWDATAPROVIDERTOOL_H
#define SCT_RAWDATABYTESTREAMCNV_ISCTRAWDATAPROVIDERTOOL_H

#include "GaudiKernel/IAlgTool.h"

#include "ByteStreamData/RawEvent.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "InDetByteStreamErrors/InDetBSErrContainer.h"
#include "InDetByteStreamErrors/SCT_ByteStreamFractionContainer.h"

#include <string>

class ISCTRawDataProviderTool : virtual public IAlgTool
{

 public:

  /// Creates the InterfaceID and interfaceID() method
  DeclareInterfaceID(ISCTRawDataProviderTool, 1, 0);

  //! destructor 
  virtual ~ISCTRawDataProviderTool() = default;

  //! this is the main decoding method
  virtual StatusCode convert(std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>&,
                             ISCT_RDO_Container&,
                             InDetBSErrContainer* errs,
                             SCT_ByteStreamFractionContainer* bsFracCont) = 0;

  //Replace the incident calls with private calls, more MT friendly
  virtual void beginNewEvent() = 0;
};

#endif // SCT_RAWDATABYTESTREAMCNV_ISCTRAWDATAPROVIDERTOOL_H
