#include "GaudiKernel/DeclareFactoryEntries.h"
#include "../SharedEvtQueueProvider.h"
#include "../SharedEvtQueueConsumer.h"
#include "../SharedWriterTool.h"
#include "../FileSchedulingTool.h"
#include "../EvtRangeScatterer.h"
#include "../EvtRangeProcessor.h"
#include "../SharedHiveEvtQueueConsumer.h"

DECLARE_TOOL_FACTORY( SharedEvtQueueProvider )
DECLARE_TOOL_FACTORY( SharedEvtQueueConsumer )
DECLARE_TOOL_FACTORY( SharedWriterTool )
DECLARE_TOOL_FACTORY( FileSchedulingTool )
DECLARE_TOOL_FACTORY( EvtRangeScatterer )
DECLARE_TOOL_FACTORY( EvtRangeProcessor )
DECLARE_TOOL_FACTORY( SharedHiveEvtQueueConsumer )

DECLARE_FACTORY_ENTRIES ( AthenaMPTools ) {
  DECLARE_TOOL( SharedEvtQueueProvider );
  DECLARE_TOOL( SharedEvtQueueConsumer );
  DECLARE_TOOL( SharedHiveEvtQueueConsumer );
  DECLARE_TOOL( SharedWriterTool );
  DECLARE_TOOL( FileSchedulingTool );
  DECLARE_TOOL( EvtRangeScatterer );
  DECLARE_TOOL( EvtRangeProcessor );
}


