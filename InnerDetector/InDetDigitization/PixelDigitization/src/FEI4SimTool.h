/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELDIGITIZATION_FEI4SimTool_H
#define PIXELDIGITIZATION_FEI4SimTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "FrontEndSimTool.h"

class FEI4SimTool:public FrontEndSimTool {

  public:
    FEI4SimTool( const std::string& type, const std::string& name,const IInterface* parent);

    virtual StatusCode initialize();
    virtual StatusCode finalize();
    virtual ~FEI4SimTool();
    virtual void process(SiChargedDiodeCollection &chargedDiodes,PixelRDO_Collection &rdoCollection);

  private:
    FEI4SimTool();

};

#endif // PIXELDIGITIZATION_FEI4SimTool_H
