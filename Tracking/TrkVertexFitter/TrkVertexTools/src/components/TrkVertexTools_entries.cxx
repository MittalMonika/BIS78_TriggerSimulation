
#include "GaudiKernel/DeclareFactoryEntries.h"

#include "TrkVertexTools/VertexCollectionSortingTool.h"
#include "TrkVertexTools/DummyVertexSelectionTool.h" 
#include "TrkVertexTools/TruthVertexSelectionTool.h"  
 
using namespace Trk;
 
DECLARE_TOOL_FACTORY(VertexCollectionSortingTool)
DECLARE_TOOL_FACTORY(DummyVertexSelectionTool)
DECLARE_TOOL_FACTORY(TruthVertexSelectionTool)

    
DECLARE_FACTORY_ENTRIES(TrkVertexTools)
{
  DECLARE_TOOL(VertexCollectionSortingTool)
  DECLARE_TOOL(DummyVertexSelectionTool)
  DECLARE_TOOL(TruthVertexSelectionTool)
      
}
 

 
