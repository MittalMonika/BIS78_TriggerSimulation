#include "LumiBlockData/BunchCrossingCondData.h"
#include "AthenaKernel/getMessageSvc.h"
#include <algorithm>

const BunchCrossingCondData::bunchTrain_t* BunchCrossingCondData::findTrain(const bcid_type bcid) const {

  const int bcidi=static_cast<int>(bcid);
  const auto it=std::find_if(m_trains.begin(),m_trains.end(),[&bcidi](const bunchTrain_t& bt){return (bt.m_first<=bcidi && bt.m_last>=bcidi);});
  
  if (it==m_trains.end()) {
    return nullptr;
  }
  else {
    return &(*it);
  }

  /*
  auto& it=std::lower_bound(m_trains.begin(),m_trains.end(),bunchTrain_t(bcid,0,0));
  if (it==m_trains.end() || it==m_trains.begin()) return nullptr;

  //it points now tho the train that starts after the bcid we are looking for
  //if the bcid is in a train, it must be the train before. 
  it--;
  if (it->m_first>=bcid && it->m_last<=bcid) {
    return &(*it);
  }
  else {
    return nullptr;
  }
  */


}

int BunchCrossingCondData::distanceFromFront( const bcid_type bcid,
					      const BunchDistanceType type ) const {

  const bunchTrain_t* bt=findTrain(bcid);
  if (!bt) {
    return -1;
  }

  const int dist=bcid-bt->m_first;

  switch( type ) {
  case NanoSec:
    return dist*m_BUNCH_SPACING;
    break;
  case BunchCrossings:
    return dist;
  case FilledBunches:
    return countColliding(bt->m_first,bt->m_last);
  default:
    MsgStream msg(Athena::getMessageSvc(),"BunchCrossingCondData::distanceFromFront");
    msg << MSG::ERROR << "BunchDistanceType not understood!" << endmsg;
    return -1;
  }//end switch
}
  
int BunchCrossingCondData::distanceFromTail( const bcid_type bcid,
					      const BunchDistanceType type ) const {

  const bunchTrain_t* bt=findTrain(bcid);
  if (!bt) {
    return -1;
  }

  const int dist=bt->m_last-bcid;

  switch( type ) {
  case NanoSec:
    return dist*m_BUNCH_SPACING;
    break;
  case BunchCrossings:
    return dist;
  case FilledBunches:
    return countColliding(bt->m_first,bt->m_last);
  default:
    MsgStream msg(Athena::getMessageSvc(),"BunchCrossingCondData::distanceFromTail");
    msg << MSG::ERROR << "BunchDistanceType not understood!" << endmsg;
    return -1;
  }//end switch
}
  
unsigned BunchCrossingCondData::numberOfUnpairedBunches() const {
  return ((m_beam1 | m_beam2) & ~m_luminous).count();
}


std::pair<unsigned,unsigned> BunchCrossingCondData::bunchTrainPopulation(const bcid_type bcid) const {

  const bunchTrain_t* bt=findTrain(bcid);
  if (!bt) {
    return std::pair<unsigned,unsigned>(0,0);
  }
  else {
    return std::pair<unsigned,unsigned>(bt->m_nColl,(bt->m_last-bt->m_first));
  }
}


unsigned BunchCrossingCondData::countColliding(int from, int to) const {
  unsigned ncoll=0;

  if (from<0) {
    //wrap-around ...
    for (int idx=m_MAX_BCID-from;idx<0;++idx) {
      if (m_luminous.test(idx)) ++ncoll;      
    }
    from=0;
  }

  if (to>m_MAX_BCID) {
    for (int idx=0;idx<m_MAX_BCID-to;++idx) {
      if (m_luminous.test(idx)) ++ncoll;
    }
    to=m_MAX_BCID;
  }
	 
  for (int idx=from;idx<to;++idx) {
    if (m_luminous.test(idx)) ++ncoll;      
  }
  return ncoll;
}
