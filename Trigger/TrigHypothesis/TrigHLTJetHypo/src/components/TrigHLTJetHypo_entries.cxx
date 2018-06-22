
#include "TrigHLTJetHypo/TrigHLTJetHypo2.h"
#include "TrigHLTJetHypo/TrigEFRazorAllTE.h"
#include "TrigHLTJetHypo/TrigEFDPhiMetJetAllTE.h"
// #include "TrigHLTJetHypo/TrigHLTJetHypo_HT.h"

#include "../TrigHLTJetHypo_EtaEtTool.h"
#include "../TrigHLTJetHypo_DijetMassDEtaTool.h"
#include "../TrigHLTJetHypo_SMCTool.h"
#include "../TrigHLTJetHypo_TLATool.h"
#include "../TrigJetHypoAlg.h"
#include "../TrigJetHypoTool_EtaEt.h"

#include "../TrigJetHypoToolConfig_EtaEt.h"
#include "../TrigJetHypoToolMT.h"

DECLARE_COMPONENT( TrigHLTJetHypo2 )
DECLARE_COMPONENT( TrigJetHypoAlg )

DECLARE_COMPONENT( TrigEFRazorAllTE )
DECLARE_COMPONENT( TrigEFDPhiMetJetAllTE )

DECLARE_COMPONENT( TrigHLTJetHypo_EtaEtTool )
DECLARE_COMPONENT( TrigHLTJetHypo_DijetMassDEtaTool )
DECLARE_COMPONENT( TrigHLTJetHypo_SMCTool )
DECLARE_COMPONENT( TrigHLTJetHypo_TLATool )


DECLARE_COMPONENT( TrigJetHypoTool_EtaEt )

DECLARE_COMPONENT(TrigJetHypoToolMT)
DECLARE_COMPONENT(TrigJetHypoToolConfig_EtaEt)
