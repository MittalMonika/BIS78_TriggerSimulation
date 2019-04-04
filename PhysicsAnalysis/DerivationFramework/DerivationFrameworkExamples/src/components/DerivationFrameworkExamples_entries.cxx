#include "GaudiKernel/DeclareFactoryEntries.h"
#include "DerivationFrameworkExamples/SkimmingToolExample.h"
#include "DerivationFrameworkExamples/ThinningToolExample.h"
#include "DerivationFrameworkExamples/AugmentationToolExample.h"
#include "DerivationFrameworkExamples/NANOThinningTool.h"
#include "DerivationFrameworkExamples/FloatCompressionAlg.h"

using namespace DerivationFramework;

DECLARE_TOOL_FACTORY( SkimmingToolExample )
DECLARE_TOOL_FACTORY( ThinningToolExample )
DECLARE_TOOL_FACTORY( AugmentationToolExample )
DECLARE_TOOL_FACTORY( NANOThinningTool )
DECLARE_NAMESPACE_ALGORITHM_FACTORY( DerivationFramework, FloatCompressionAlg )

DECLARE_FACTORY_ENTRIES( DerivationFrameworkExamples ) {
   DECLARE_TOOL( SkimmingToolExample )
   DECLARE_TOOL( ThinningToolExample )
   DECLARE_TOOL( AugmentationToolExample )
   DECLARE_TOOL( NANOThinningTool )
   DECLARE_NAMESPACE_ALGORITHM( DerivationFramework, FloatCompressionAlg )
}

