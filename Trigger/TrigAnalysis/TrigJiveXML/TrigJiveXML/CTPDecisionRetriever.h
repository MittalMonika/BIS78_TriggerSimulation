/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_CTPDECISIONRETRIEVER_H
#define JIVEXML_CTPDECISIONRETRIEVER_H

#include <string>

#include "JiveXML/IDataRetriever.h"

#include "AthenaBaseComps/AthAlgTool.h"


namespace JiveXML{

  class CTPDecisionRetriever : virtual public IDataRetriever, public AthAlgTool {

  public:

    /// Standard Constructor
    CTPDecisionRetriever(const std::string& type, const std::string& name, const IInterface* parent);

    /// Retrieve all the data
    virtual StatusCode retrieve(ToolHandle<IFormatTool> FormatTool);

    /// Return the name of the data type
    virtual std::string dataTypeName() const { return m_typeName; };

  private:

   ///The data type that is generated by this retriever
    const std::string m_typeName;

    ///The storegate key 
    std::string m_sgKey;
  };

}
#endif
