#include "InDetPhysValMonitoring/InDetPhysValMonitoringTool.h"
#include "InDetPhysValMonitoring/InDetPhysValLargeD0Tool.h"
#include "../InDetPhysValTruthDecoratorAlg.h"
#include "../InDetPhysHitDecoratorAlg.h"
#include "../ParameterErrDecoratorAlg.h"
#include "../TruthClassDecoratorAlg.h"
#include "../TrackTruthSelectionTool.h"
#include "../dRMatchingTool.h"
#include "../TrackSelectionTool.h"
#include "InDetPhysValMonitoring/HistogramDefinitionSvc.h"
#include "InDetPhysValMonitoring/AlgTestHistoDefSvc.h"
#include "InDetPhysValMonitoring/ToolTestMonitoringPlots.h"
#include "../AthTruthSelectionTool.h"

#include "../DummyTrackSlimmingTool.h"

DECLARE_COMPONENT( AlgTestHistoDefSvc )
DECLARE_COMPONENT( HistogramDefinitionSvc )
DECLARE_COMPONENT( InDetPhysValMonitoringTool )
DECLARE_COMPONENT( InDetPhysValLargeD0Tool )
DECLARE_COMPONENT( ToolTestMonitoringPlots )
DECLARE_COMPONENT( InDetPhysValTruthDecoratorAlg )
DECLARE_COMPONENT( InDetPhysHitDecoratorAlg )
DECLARE_COMPONENT( ParameterErrDecoratorAlg )
DECLARE_COMPONENT( TruthClassDecoratorAlg )
DECLARE_COMPONENT( TrackTruthSelectionTool )
DECLARE_COMPONENT( dRMatchingTool )
DECLARE_COMPONENT( TrackSelectionTool )
DECLARE_COMPONENT( DummyTrackSlimmingTool )
DECLARE_COMPONENT( AthTruthSelectionTool )

