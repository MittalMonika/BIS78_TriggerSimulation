/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigHLTJetHypo/TrigHLTJetHypo2.h"
#include "TrigHLTJetHypo/TrigEFRazorAllTE.h"
#include "TrigHLTJetHypo/TrigEFDPhiMetJetAllTE.h"
#include "TrigHLTJetHypo/TrigHLTJetHypo_Dijet.h"
#include "TrigHLTJetHypo/TrigHLTJetHypo_DijetMassDEta.h"
#include "TrigHLTJetHypo/TrigHLTJetHypo_DijetMassDEtaDPhi.h"
#include "TrigHLTJetHypo/TrigHLTJetHypo_EtaEt.h"
#include "TrigHLTJetHypo/TrigHLTJetHypo_HT.h"
#include "TrigHLTJetHypo/TrigHLTJetHypo_TLA.h"
#include "TrigHLTJetHypo/TrigHLTJetHypo_SMC.h"
#include "../TrigJetHypoAlgMT.h"
#include "../TrigJetHypoToolConfig_EtaEt.h"
#include "../TrigJetHypoToolConfig_simple.h"
#include "../TrigJetHypoToolConfig_dijet.h"
#include "../NotHelperTool.h"
#include "../AndHelperTool.h"
#include "../OrHelperTool.h"
#include "../TrigJetHypoToolMT.h"
#include "../TrigJetHypoToolHelperMT.h"
#include "../CombinationsHelperTool.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/BasicCleanerTool.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/AntiCleanerTool.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/EtaEtCleanerTool.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/LlpCleanerTool.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/LooseCleanerTool.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/NullCleanerTool.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/TightCleanerTool.h"

DECLARE_COMPONENT( TrigHLTJetHypo2 )

DECLARE_COMPONENT( TrigEFRazorAllTE )
DECLARE_COMPONENT( TrigEFDPhiMetJetAllTE )
DECLARE_COMPONENT( TrigHLTJetHypo_Dijet )
DECLARE_COMPONENT( TrigHLTJetHypo_DijetMassDEta )
DECLARE_COMPONENT( TrigHLTJetHypo_DijetMassDEtaDPhi )
DECLARE_COMPONENT( TrigJetHypoToolConfig_EtaEt )
DECLARE_COMPONENT( TrigJetHypoToolConfig_simple )
DECLARE_COMPONENT( TrigJetHypoToolConfig_dijet )
DECLARE_COMPONENT( NotHelperTool)
DECLARE_COMPONENT( AndHelperTool)
DECLARE_COMPONENT( OrHelperTool)
DECLARE_COMPONENT( TrigHLTJetHypo_SMC )
DECLARE_COMPONENT( TrigHLTJetHypo_HT )
DECLARE_COMPONENT( TrigHLTJetHypo_TLA )
DECLARE_COMPONENT( TrigHLTJetHypo_EtaEt )

DECLARE_COMPONENT( TrigJetHypoAlgMT )
DECLARE_COMPONENT( TrigJetHypoToolMT )
DECLARE_COMPONENT( TrigJetHypoToolHelperMT )
DECLARE_COMPONENT( TrigJetHypoToolConfig_EtaEt )
DECLARE_COMPONENT( CombinationsHelperTool )

DECLARE_COMPONENT(BasicCleanerTool)
DECLARE_COMPONENT(AntiCleanerTool)
DECLARE_COMPONENT(EtaEtCleanerTool)
DECLARE_COMPONENT(LlpCleanerTool)
DECLARE_COMPONENT(LooseCleanerTool)
DECLARE_COMPONENT(NullCleanerTool)
DECLARE_COMPONENT(TightCleanerTool)

