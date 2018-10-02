/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPrepRawData/MMPrepData.h"
#include "GaudiKernel/MsgStream.h"

namespace Muon
{

  MMPrepData::MMPrepData( const Identifier& RDOId,
                          const IdentifierHash &idDE,
                          const Amg::Vector2D& locpos,
                          const std::vector<Identifier>& rdoList,
                          const Amg::MatrixX* locErrMat,
                          const MuonGM::MMReadoutElement* detEl,
			  const int time, 
			  const int charge ) :
    MuonCluster(RDOId, idDE, locpos, rdoList, locErrMat), //call base class constructor
    m_detEl(detEl),
    m_time(time),
    m_charge(charge)
  { }

  MMPrepData::MMPrepData( const Identifier& RDOId,
                          const IdentifierHash &idDE,
                          const Amg::Vector2D& locpos,
                          const std::vector<Identifier>& rdoList,
                          const Amg::MatrixX* locErrMat,
                          const MuonGM::MMReadoutElement* detEl) :
    MuonCluster(RDOId, idDE, locpos, rdoList, locErrMat), //call base class constructor
    m_detEl(detEl),
    m_time(0),
    m_charge(0)
  { }

  // Destructor:
  MMPrepData::~MMPrepData()
  {

  }

  // Default constructor:
  MMPrepData::MMPrepData():
    MuonCluster(),
    m_detEl(0),
    m_time(0),
    m_charge(0)
  { }

  //copy constructor:
  MMPrepData::MMPrepData(const MMPrepData& RIO):
    MuonCluster(RIO),
    m_detEl( RIO.m_detEl ),
    m_time(RIO.m_time),
    m_charge(RIO.m_charge)
  { }

  //move constructor:
  MMPrepData::MMPrepData(MMPrepData&& RIO):
    MuonCluster(std::move(RIO)),
    m_detEl( RIO.m_detEl ),
    m_time(RIO.m_time),
    m_charge(RIO.m_charge)
  { }

  //assignment operator
  MMPrepData&
  MMPrepData::operator=(const MMPrepData& RIO)
  {
    if (&RIO !=this)
      {
	MuonCluster::operator=(RIO);
	m_detEl =  RIO.m_detEl ;
	m_time = RIO.m_time;
	m_charge = RIO.m_charge;
      }
    return *this;

  }

  MMPrepData&
  MMPrepData::operator=(MMPrepData&& RIO)
  {
    if (&RIO !=this)
      {
	MuonCluster::operator=(std::move(RIO));
	m_detEl =  RIO.m_detEl ;
	m_time =  RIO.m_time ;
	m_charge =  RIO.m_charge ;
      }
    return *this;

  }

  MsgStream&
  MMPrepData::dump( MsgStream&    stream) const
  {
    stream << MSG::INFO<<"MMPrepData {"<<std::endl;
    
    MuonCluster::dump(stream);
 
    stream<<"}"<<endmsg;

    return stream;
  }

  std::ostream&
  MMPrepData::dump( std::ostream&    stream) const
  {
    stream << "MMPrepData {"<<std::endl;
    
    MuonCluster::dump(stream);

    stream<<"}"<<std::endl;

    return stream;
  }
  //end of classdef
}//end of ns

