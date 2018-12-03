/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DITAUREC_HADMUBUILDER_H
#define DITAUREC_HADMUBUILDER_H

#include "AsgTools/AsgTool.h"

#include "DiTauRec/ILepHadBuilder.h"

namespace DiTauRec
{
  
  class HadMuBuilder
    : public DiTauRec::ILepHadBuilder
    , public asg::AsgTool
  {

    ASG_TOOL_CLASS( HadMuBuilder,
                    DiTauRec::ILepHadBuilder )
 
    public:

    HadMuBuilder( const std::string& name );

    virtual ~HadMuBuilder() override;

    virtual StatusCode initialize() override;

    virtual StatusCode execute() override;
  private:
    std::string m_sDiTauHadMuContainerName;
    std::string m_sDiTauHadMuAuxContainerName;
    std::string m_sMuonContainerName;
    std::string m_sTauContainerName;
    float m_fTauMinPt;
    unsigned int m_iTauMinNtracks;
    unsigned int m_iTauMaxNtracks;
  };
}
#endif // DITAUREC_HADMUBUILDER_H
