/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_LVL1RESULTRETRIEVER_H
#define JIVEXML_LVL1RESULTRETRIEVER_H

#include <string>

#include "JiveXML/IDataRetriever.h"

#include "AthenaBaseComps/AthAlgTool.h"

#include "TrigDecisionTool/TrigDecisionTool.h"

namespace JiveXML{

  class LVL1ResultRetriever : virtual public IDataRetriever, public AthAlgTool {

  public:

    /// Standard Constructor
    LVL1ResultRetriever(const std::string& type, const std::string& name, const IInterface* parent);

    /// Retrieve all the data
    virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool);

    /// Return the name of the data type
    virtual std::string dataTypeName() const { return m_typeName; };

    //Initialize
    StatusCode initialize(); 

    /// Subroutine for constructing the item list string
    StatusCode getItemLists(const Trig::ChainGroup* chains, 
                            std::string& listName, std::string& prescaleName);

  private:

    ///The data type that is generated by this retriever
    const std::string m_typeName;

    /// A handle to the TrigDecisionTool
    ToolHandle<Trig::TrigDecisionTool> m_trigDec;

    /// the trigger chains
    const Trig::ChainGroup* m_all;
    const Trig::ChainGroup* m_allL1;
    const Trig::ChainGroup* m_allL2;
    const Trig::ChainGroup* m_allEF;
    const Trig::ChainGroup* m_allHLT;
  };

}
#endif
