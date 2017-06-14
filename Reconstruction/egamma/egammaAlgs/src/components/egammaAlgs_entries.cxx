#include "GaudiKernel/DeclareFactoryEntries.h"
#include "egammaAlgs/egammaBuilder.h"
#include "egammaAlgs/egammaRecBuilder.h"
#include "egammaAlgs/topoEgammaBuilder.h"
#include "egammaAlgs/egammaForwardBuilder.h"
#include "egammaAlgs/egammaTruthAssociationAlg.h"
#include "egammaAlgs/egammaTrackSlimmer.h"
#include "egammaAlgs/EMBremCollectionBuilder.h"
#include "egammaAlgs/EMVertexBuilder.h"
#include "egammaAlgs/egammaTopoClusterCopier.h"
#include "egammaAlgs/electronSuperClusterBuilder.h"
#include "egammaAlgs/photonSuperClusterBuilder.h"


DECLARE_ALGORITHM_FACTORY( egammaBuilder            )
DECLARE_ALGORITHM_FACTORY( egammaRecBuilder         )
DECLARE_ALGORITHM_FACTORY( topoEgammaBuilder        )
DECLARE_ALGORITHM_FACTORY( egammaForwardBuilder     )
DECLARE_ALGORITHM_FACTORY( egammaTruthAssociationAlg)
DECLARE_ALGORITHM_FACTORY( egammaTrackSlimmer       )
DECLARE_ALGORITHM_FACTORY( EMBremCollectionBuilder  )
DECLARE_ALGORITHM_FACTORY( EMVertexBuilder          )
DECLARE_ALGORITHM_FACTORY( egammaTopoClusterCopier  )
DECLARE_ALGORITHM_FACTORY( electronSuperClusterBuilder  )
DECLARE_ALGORITHM_FACTORY( photonSuperClusterBuilder    )

DECLARE_FACTORY_ENTRIES(egammaAlgs) {
     DECLARE_ALGORITHM( egammaBuilder             )
     DECLARE_ALGORITHM( egammaRecBuilder          )
     DECLARE_ALGORITHM( topoEgammaBuilder         )
     DECLARE_ALGORITHM( egammaForwardBuilder      )
     DECLARE_ALGORITHM( egammaTruthAssociationAlg )
     DECLARE_ALGORITHM( egammaTrackSlimmer        )
     DECLARE_ALGORITHM( EMBremCollectionBuilder   )
     DECLARE_ALGORITHM( EMVertexBuilder           )
     DECLARE_ALGORITHM( egammaTopoClusterCopier   )
     DECLARE_ALGORITHM( electronSuperClusterBuilder  )
     DECLARE_ALGORITHM( photonSuperClusterBuilder    )

}
