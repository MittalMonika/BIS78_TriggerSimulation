/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_ELECTRONRETRIEVER_H
#define JIVEXML_ELECTRONRETRIEVER_H

#include <string>
#include <vector>
#include <map>

#include "JiveXML/IDataRetriever.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "egammaEvent/ElectronContainer.h"

class ElectronContainer;

namespace JiveXML{
  
  /**
   * @class ElectronRetriever
   * @brief Retrieves all @c Electron @c objects (ElectronAODCollection etc.)
   *
   *  - @b Properties
   *    - StoreGateKey: First collection to be retrieved, displayed
   *      in Atlantis without switching. All other collections are 
   *      also retrieved.
   *
   *  - @b Retrieved @b Data
   *    - Usual four-vectors: phi, eta, et etc.
   *    - Associations for clusters and tracks via ElementLink: key/index scheme
   */
  class ElectronRetriever : virtual public IDataRetriever,
                                   public AthAlgTool {
    
    public:
      
      /// Standard Constructor
      ElectronRetriever(const std::string& type,const std::string& name,const IInterface* parent);
      
      /// Retrieve all the data
      virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool); 
      const DataMap getData(const ElectronContainer*);
    
      /// Return the name of the data type
      virtual std::string dataTypeName() const { return typeName; };

    private:
      ///The data type that is generated by this retriever
      const std::string typeName;

      std::string m_sgKey;
  };
}
#endif
