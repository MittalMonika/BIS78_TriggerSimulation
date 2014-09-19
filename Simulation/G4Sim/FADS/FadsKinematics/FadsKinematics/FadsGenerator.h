/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FadsGenerator_H
#define FadsGenerator_H

// An abstract base class for a kinematics generator

#include "HepMC/GenEvent.h"
#include <string>

namespace FADS {

class FadsGenerator {
private:
	std::string name;
public:
	std::string GetName() const {return name;}
	FadsGenerator(std::string n): name(n) {}
	virtual ~FadsGenerator() {}
	virtual void Initialize()=0;
	virtual void Terminate()=0;
	virtual void Create() {}
	virtual HepMC::GenEvent* GenerateAnEvent()=0;
};

}	// end namespace

#endif
