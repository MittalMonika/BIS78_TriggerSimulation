/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_TRIGMUONROIRETRIEVER_H
#define JIVEXML_TRIGMUONROIRETRIEVER_H

#include <string>

#include "JiveXML/IDataRetriever.h"

#include "AthenaBaseComps/AthAlgTool.h"


namespace JiveXML{

  class TrigMuonROIRetriever : virtual public IDataRetriever, public AthAlgTool {

  public:

    /// Standard Constructor
    TrigMuonROIRetriever(const std::string& type, const std::string& name, const IInterface* parent);

    /// Retrieve all the data
    virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool);

    /// Return the name of the data type
    virtual std::string dataTypeName() const { return m_typeName; };

  private:

   ///The data type that is generated by this retriever
    const std::string m_typeName;

    ///The storegate key for the LVL1 Muon RoIs
    std::string m_sgKey;
  };

}
#endif
