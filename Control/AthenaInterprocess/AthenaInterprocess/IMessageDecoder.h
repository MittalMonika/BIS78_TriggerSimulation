/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAINTERPROCESS_IMESSAGEDECODER_H
#define ATHENAINTERPROCESS_IMESSAGEDECODER_H 1

namespace AthenaInterprocess {
  struct ScheduledWork {
    void* data;
    int size;
  };

  class IMessageDecoder
  {
  public:
    virtual ~IMessageDecoder() {}

    virtual ScheduledWork* operator()(const ScheduledWork&) = 0;
  };
}  

#endif
