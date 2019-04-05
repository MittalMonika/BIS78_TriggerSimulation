/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include<fstream>
#include "TrigT1RPChardware/RPCReadOut.h"

using namespace std;

  // initialize some fixed binary pattern
const ubit16 RPCReadOut::s_last2bitsON =0xc000;
const ubit16 RPCReadOut::s_last4bitsON =0xf000;
//---------------------------------------------------------------------------//
RPCReadOut::RPCReadOut() {
m_word =0xffff;
m_field =0;
}//end-of-RPCReadOut::RPCReadOut()
//---------------------------------------------------------------------------//
