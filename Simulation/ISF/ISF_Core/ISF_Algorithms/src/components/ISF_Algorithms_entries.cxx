#include "GaudiKernel/DeclareFactoryEntries.h"
#include "../SimKernel.h"
#include "../SimKernelMT.h"
#include "../CollectionMerger.h"
#include "../SimHitTreeCreator.h"

DECLARE_NAMESPACE_ALGORITHM_FACTORY( ISF , SimKernel        )
DECLARE_NAMESPACE_ALGORITHM_FACTORY( ISF , SimKernelMT      )
DECLARE_NAMESPACE_ALGORITHM_FACTORY( ISF , CollectionMerger )

DECLARE_FACTORY_ENTRIES( ISF_Algorithms ) {
  DECLARE_NAMESPACE_ALGORITHM( ISF ,  SimKernel        )
  DECLARE_NAMESPACE_ALGORITHM( ISF ,  SimKernelMT      )
  DECLARE_NAMESPACE_ALGORITHM( ISF ,  CollectionMerger )
}
