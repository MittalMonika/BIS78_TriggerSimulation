/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <SampleHandler/DiskOutputLocal.h>

#include <sstream>
#include <RootCoreUtils/Assert.h>
#include <SampleHandler/DiskWriterLocal.h>

//
// method implementations
//

ClassImp (SH::DiskOutputLocal)

namespace SH
{
  void DiskOutputLocal :: 
  testInvariant () const
  {
    RCU_INVARIANT (this != 0);
    RCU_INVARIANT (!m_prefix.empty());
  }



  DiskOutputLocal :: 
  DiskOutputLocal ()
    : m_prefix ("/ThisIsANonExistantPath")
  {
    RCU_NEW_INVARIANT (this);
  }



  DiskOutputLocal :: 
  DiskOutputLocal (const std::string& val_prefix)
    : m_prefix (val_prefix)
  {
    RCU_NEW_INVARIANT (this);
  }



  DiskWriter *DiskOutputLocal ::
  doMakeWriter (const std::string& sample, const std::string& name,
		int index, const std::string& suffix) const
  {
    RCU_READ_INVARIANT (this);
    std::ostringstream file;
    file << m_prefix << "/";
    file << sample;
    if (!sample.empty() && !name.empty())
      file << "-";
    file << name;
    if (index >= 0)
      file << "-" << index;
    file << suffix;
    return new DiskWriterLocal (file.str());
  }
}
