/*
Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAVALIDATION_BASICHISTOGRAMS_H
#define EGAMMAVALIDATION_BASICHISTOGRAMS_H


#include "xAODBase/IParticle.h"

#include <string>
#include <map>
class ITHistSvc;
class StatusCode;
class TH1D;

namespace egammaMonitoring {

  class IHistograms {

  public:

    IHistograms();
    IHistograms(std::string name,
                   std::string title,
                   std::string folder,
                   ITHistSvc * &rootHistSvc
    ) :
      m_name(name),
      m_title(title),
      m_folder(folder),
      m_rootHistSvc(rootHistSvc) {}

    virtual StatusCode initializePlots() = 0;

    virtual void fill(const xAOD::IParticle&) = 0;
    virtual ~IHistograms(){};

    std::map<std::string, TH1D* > histoMap;

  protected:
    std::string m_name;
    std::string m_title;
    std::string m_folder;
    ITHistSvc*  m_rootHistSvc =  nullptr;


  };

}


#endif //EGAMMAVALIDATION_BASICHISTOGRAM_H
