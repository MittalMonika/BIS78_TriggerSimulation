/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARHV_HECHVPAYLOAD_H
#define LARHV_HECHVPAYLOAD_H
#include <iostream>
struct HECHVPayload {
  double         voltage;
  double         current;
  unsigned int   status;
  int            hvLineNo;
};

inline std::ostream & operator << (std::ostream & o, const HECHVPayload & payload) {
  o << "HEC Subgap: HV Line No "  << payload.hvLineNo    << ' ' << payload.voltage << " volts; " << payload.current << " amps; status = " << payload.status << std::endl;
  return o;
}
#endif
