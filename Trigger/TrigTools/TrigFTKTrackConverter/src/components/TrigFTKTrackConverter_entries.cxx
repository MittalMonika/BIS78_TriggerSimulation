#include "TrigFTKTrackConverter/TrigFTKTrackConverter.h"
#include "TrigFTKTrackConverter/TrigFTKUncertiantyTool.h"
#include "GaudiKernel/DeclareFactoryEntries.h"

DECLARE_TOOL_FACTORY(TrigFTKTrackConverter)
DECLARE_TOOL_FACTORY(TrigFTKUncertiantyTool)

DECLARE_FACTORY_ENTRIES(TrigFTKTrackConverter)
{
  DECLARE_TOOL(TrigFTKTrackConverter)
  DECLARE_TOOL(TrigFTKUncertiantyTool)
}
