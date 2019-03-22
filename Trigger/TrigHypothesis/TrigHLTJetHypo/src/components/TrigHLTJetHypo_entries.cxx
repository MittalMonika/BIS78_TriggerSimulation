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
#include "../TrigJetNotToolMT.h"
#include "../TrigJetAndToolMT.h"
#include "../TrigJetOrToolMT.h"
#include "../TrigJetHypoToolMT.h"

DECLARE_COMPONENT( TrigHLTJetHypo2 )

DECLARE_COMPONENT( TrigEFRazorAllTE )
DECLARE_COMPONENT( TrigEFDPhiMetJetAllTE )
DECLARE_COMPONENT( TrigHLTJetHypo_Dijet )
DECLARE_COMPONENT( TrigHLTJetHypo_DijetMassDEta )
DECLARE_COMPONENT( TrigHLTJetHypo_DijetMassDEtaDPhi )
DECLARE_COMPONENT( TrigHLTJetHypo_EtaEt )
DECLARE_COMPONENT( TrigJetHypoToolConfig_simple )
DECLARE_COMPONENT( TrigJetHypoToolConfig_dijet )
DECLARE_COMPONENT( TrigJetNotToolMT)
DECLARE_COMPONENT( TrigJetAndToolMT)
DECLARE_COMPONENT( TrigJetOrToolMT)
DECLARE_COMPONENT( TrigHLTJetHypo_SMC )
DECLARE_COMPONENT( TrigHLTJetHypo_HT )
DECLARE_COMPONENT( TrigHLTJetHypo_TLA )
DECLARE_COMPONENT( TrigHLTJetHypo_EtaEt )

DECLARE_COMPONENT( TrigJetHypoAlgMT )
DECLARE_COMPONENT( TrigJetHypoToolMT )
DECLARE_COMPONENT( TrigJetHypoToolConfig_EtaEt )
