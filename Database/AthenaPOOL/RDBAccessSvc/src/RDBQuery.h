/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef _RDBQUERY_H_
#define _RDBQUERY_H_

#include "RDBAccessSvc/IRDBQuery.h"
#include <memory>
#include <vector>
#include <string>

namespace coral
{
  class IQuery;
  class ISessionProxy;
  class ICursor;
}

/**
 * @class RDBQuery
 *
 * @brief RDBQuery is an implementation of IRDBQuery interface
 *
 */

class RDBAccessSvc;

class RDBQuery final : public IRDBQuery
{
 public:
  RDBQuery(RDBAccessSvc* _accessSvc,
	   coral::ISessionProxy* _session,
	   const std::string& nodeName,
	   const std::string& tagId);

  ~RDBQuery() override;
  
  void execute() override;
  long size() override;
  void finalize() override;
  void setOrder(const std::string&) override;
  void addToOutput(const std::string&) override;

  bool next();

 private:
  RDBQuery() {}

  coral::IQuery*           m_query;
  coral::IQuery*           m_queryCount;
  RDBAccessSvc*            m_accessSvc;
  coral::ISessionProxy*    m_session;
  std::string              m_nodeName;
  std::string              m_tagId;
  long                     m_size;
  std::vector<std::string> m_fields;
  coral::ICursor*          m_cursor;
  std::string              m_orderField;
};

#endif
