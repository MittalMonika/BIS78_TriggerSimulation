/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// **********
//
// NAME:    TrigHLTJetRecGroomer.h
// PACKAGE: Trigger/TrigAlgorithms/TrigHLTJetRec
//
// AUTHOR:  Steven Schramm
//
// Description: FexAlgo for building a jet container and trimming it
//
// **********


#ifndef TRIGHLTJETREC_TRIGHLTJETRECGROOMER_H
#define TRIGHLTJETREC_TRIGHLTJETRECGROOMER_H

#include "TrigHLTJetRec/TrigHLTJetRecFromCluster.h"
#include "TrigHLTJetRec/ITriggerJetGroomerTool.h"

class TrigHLTJetRecGroomer : public TrigHLTJetRecFromCluster
{
    public:
        TrigHLTJetRecGroomer(const std::string& name, ISvcLocator* pSvcLocator);
        virtual ~TrigHLTJetRecGroomer();

        virtual HLT::ErrorCode hltInitialize() override;

    protected:
        HLT::ErrorCode build(fastjet::ClusterSequence*&, xAOD::JetContainer*&) const override;

    private:
        // properties configurable via python
        ToolHandle<ITriggerJetGroomerTool> m_jetTrimTool;

};


#endif
