
#include <iostream>
#include <iomanip>

#include "TrigT1Interfaces/Lvl1MuCTPIInputPhase1.h"

namespace LVL1MUONIF {

  Lvl1MuCTPIInputPhase1::Lvl1MuCTPIInputPhase1() {
    for (size_t isys=0; isys<NumberOfMuonSystem; isys++){
      isFilledOutOfTimeCandidates[isys] = false;
    }}

  /////////////
  Lvl1MuCTPIInputPhase1::Lvl1MuCTPIInputPhase1( const Lvl1MuCTPIInputPhase1& right ) {
    *this = right;
  }

  /////////////
  Lvl1MuCTPIInputPhase1::~Lvl1MuCTPIInputPhase1() {
    clearAll();
  }

  /////////////

  Lvl1MuCTPIInputPhase1& Lvl1MuCTPIInputPhase1::operator=( const Lvl1MuCTPIInputPhase1& right ) {
    if ( this != &right ) {
      clearAll();
      for (size_t idSys=0; idSys<NumberOfMuonSystem; idSys++){
	for( size_t ip=0; ip<right.m_data[idSys].size(); ip++){
	  int bc=((right.m_data[idSys]).at(ip)).first;
	  Lvl1MuVect vSL(((right.m_data[idSys]).at(ip)).second);
	  m_data[idSys].push_back(std::make_pair(bc,vSL));
	}
	isFilledOutOfTimeCandidates[idSys] = right.isFilledOutOfTimeCandidates[idSys];
      }	
    }  
    return *this;
  }



  /////////////
  const Lvl1MuSectorLogicDataPhase1& Lvl1MuCTPIInputPhase1::getSectorLogicData( size_t systemAddress,
								    size_t subSystemAddress,
								    size_t sectorAddress,
								    int    bcid             ) const {

    static Lvl1MuBarrelSectorLogicDataPhase1 dummy;
    for( size_t ip=0; ip<m_data[systemAddress].size(); ip++){
      int bc=((m_data[systemAddress]).at(ip)).first;
      if (bc != bcid) continue;
      const Lvl1MuVect vecSL( ((m_data[systemAddress]).at(ip)).second);
      return *(vecSL.at(getSystemIndex(systemAddress,subSystemAddress,sectorAddress)));
    }
    return dummy;
  }

  /////////////
  void Lvl1MuCTPIInputPhase1::setSectorLogicData( const Lvl1MuSectorLogicDataPhase1& data,
					    size_t systemAddress,
					    size_t subSystemAddress,
					    size_t sectorAddress,
					    int    bcid             ) {
    size_t ip=0;
    if (isEmpty(systemAddress,bcid)) {
      ip = reserve( systemAddress, bcid );
    } else {
      ip = getBcidIndex( systemAddress, bcid );
    }

    Lvl1MuVect vecSL((((m_data[systemAddress]).at(ip)).second));
    size_t idx= getSystemIndex(systemAddress,subSystemAddress,sectorAddress);
    if ( systemAddress == Barrel ) {
      *std::dynamic_pointer_cast<Lvl1MuBarrelSectorLogicDataPhase1>(vecSL.at(idx)) = data;
    } else if ( systemAddress == Endcap ) {
      *std::dynamic_pointer_cast<Lvl1MuEndcapSectorLogicDataPhase1>(vecSL.at(idx))= data;
    } else if ( systemAddress == Forward ) {
      *std::dynamic_pointer_cast<Lvl1MuForwardSectorLogicDataPhase1>(vecSL.at(idx))= data;
    }
  }

  /////////////
  void Lvl1MuCTPIInputPhase1::setSectorLogicData( const Lvl1MuSectorLogicDataPhase1::SectorLogicWord & sectorWord,
					    size_t systemAddress,
					    size_t subSystemAddress,
					    size_t sectorAddress,
					    int    bcid              )  {
    // convert to SectorLogicDataPhase1
    Lvl1MuSectorLogicDataPhase1 * pData;
    if ( systemAddress == Barrel ) {
      pData = new Lvl1MuBarrelSectorLogicDataPhase1();
    } else if ( systemAddress == Endcap ) {
      pData = new Lvl1MuEndcapSectorLogicDataPhase1();
    } else if ( systemAddress == Forward ) {
      pData = new Lvl1MuForwardSectorLogicDataPhase1();
    } else {
      return ;
    }
    pData->convertFromWordFormat( sectorWord );
    setSectorLogicData( *pData, systemAddress, subSystemAddress, sectorAddress, bcid );
    delete pData;
  }

  /////////////
  void Lvl1MuCTPIInputPhase1::merge( const Lvl1MuCTPIInputPhase1& right ) {
    if ( this == &right ) return;
    
    for (size_t idSys=0; idSys<NumberOfMuonSystem; idSys++){
      for( size_t ip=0; ip<right.m_data[idSys].size(); ip++){
	int bc=((right.m_data[idSys]).at(ip)).first;
	if (isEmpty( idSys, bc)){
	  Lvl1MuVect vSL(((right.m_data[idSys]).at(ip)).second);
	  m_data[idSys].push_back(std::make_pair(bc,vSL));
	  isFilledOutOfTimeCandidates[idSys] = right.isFilledOutOfTimeCandidates[idSys];
	}
      }	
    }  

  }

  /////////////
  size_t Lvl1MuCTPIInputPhase1::reserve( size_t systemAddress ,
				   int    bcid)          {
    
    Lvl1MuVect vecSL;
    
    if ( systemAddress == Barrel ) {
      for ( size_t id = 0; id < NumberOfBarrelSector*NumberOfMuonSubSystem; id++ ) {
	std::shared_ptr<Lvl1MuBarrelSectorLogicDataPhase1> barrelSect(new Lvl1MuBarrelSectorLogicDataPhase1);
	vecSL.push_back(barrelSect);
      }

    } else if ( systemAddress == Endcap ) {
      for ( size_t id = 0; id < NumberOfEndcapSector*NumberOfMuonSubSystem; id++ ) {
	std::shared_ptr<Lvl1MuEndcapSectorLogicDataPhase1> endcapSect(new Lvl1MuEndcapSectorLogicDataPhase1);
	vecSL.push_back(endcapSect);
      }

    } else if ( systemAddress == Forward ) {
      for ( size_t id = 0; id < NumberOfForwardSector*NumberOfMuonSubSystem; id++ ) {
	std::shared_ptr<Lvl1MuForwardSectorLogicDataPhase1> forwardSect(new Lvl1MuForwardSectorLogicDataPhase1);
	vecSL.push_back(forwardSect);
      }
    }

    size_t ip = m_data[systemAddress].size();
    m_data[systemAddress].push_back( std::make_pair(bcid, vecSL) );

    if (bcid!=0) isFilledOutOfTimeCandidates[systemAddress] = true; 

    return ip;
  }

  /////////////
  bool Lvl1MuCTPIInputPhase1::isEmpty( size_t systemAddress,
				 int    bcid           ) const {
    for( size_t ip=0; ip<m_data[systemAddress].size(); ip++){
      int bc=((m_data[systemAddress]).at(ip)).first;
      if (bc == bcid) return false;;
    }
    return true;
  }

  /////////////
  bool Lvl1MuCTPIInputPhase1::isEmptyAll(int bcid) const {
    if ( isEmpty(Barrel,bcid) && isEmpty(Endcap,bcid) && isEmpty(Forward,bcid) ) {
	return true;
      } else {
	return false;
      }
  }

  /////////////
  void Lvl1MuCTPIInputPhase1::clear( size_t systemAddress ) {
    std::cout << "TW: SysAd: in " << systemAddress << "  size: " << m_data[systemAddress].size()  << std::endl;
    m_data[systemAddress].clear();
  }

  /////////////
  void Lvl1MuCTPIInputPhase1::clearAll() {
    for ( size_t id = 0; id < NumberOfMuonSystem; id++ ) {
      m_data[ id ].clear();
    }


  }

//// routine for testing purposes only
  // void Lvl1MuCTPIInputPhase1::duplicateToOtherBC(int bcidOffset) {

  //   std::cout << "I am faking candidates for testing - do you really want that??" << std::endl;

  //   std::vector<Lvl1MuVectWithBC> dataCopy[ NumberOfMuonSystem ];

  //   for ( size_t id = 0; id < NumberOfMuonSystem; id++ ) {
  //     dataCopy[ id ] = m_data[ id ];
  //   }

  //   for (std::vector<Lvl1MuVectWithBC>::iterator itb = dataCopy[Barrel].begin(); itb !=dataCopy[Barrel].end(); ++itb){
  //     (*itb).first =bcidOffset ;
  //   }
  //   std::vector<Lvl1MuVectWithBC>::iterator itb = m_data[Barrel].end();
  //   m_data[Barrel].insert(itb, dataCopy[Barrel].begin(), dataCopy[Barrel].end());

  //   for (std::vector<Lvl1MuVectWithBC>::iterator ite = dataCopy[Endcap].begin(); ite !=dataCopy[Endcap].end(); ++ite){
  //     (*ite).first =bcidOffset ;
  //   }
  //   std::vector<Lvl1MuVectWithBC>::iterator ite = m_data[Endcap].end();
  //   m_data[Endcap].insert(ite, dataCopy[Endcap].begin(), dataCopy[Endcap].end());

  //   for (std::vector<Lvl1MuVectWithBC>::iterator itf = dataCopy[Forward].begin(); itf !=dataCopy[Forward].end(); ++itf){
  //     (*itf).first =bcidOffset ;
  //   }
  //   std::vector<Lvl1MuVectWithBC>::iterator itf = m_data[Forward].end();
  //   m_data[Forward].insert(itf, dataCopy[Forward].begin(), dataCopy[Forward].end());
  // }


  std::ostream& operator<<( std::ostream& out, const Lvl1MuCTPIInputPhase1& right ) {

    size_t systemAddress;
    size_t subSystemAddress;
    size_t side, id;

    out << "--------- Barrel Sector ---------------------------" << std::endl;
    systemAddress = right.idBarrelSystem();
    for ( side = 0; side < right.NumberOfMuonSubSystem; ++side ) {
      if ( side == 0 ) {
        out << "   ------ A Side (Z>0)  ---------------------------" << std::endl;
        subSystemAddress = right.idSideA();
      } else {
        out << "   ------ C Side (Z<0)  ---------------------------" << std::endl;
        subSystemAddress = right.idSideC();
      }
      for ( id = 0; id < right.numberOfBarrelSector() ;++id ) {
        out << "      --- Sector ID  :  " << id << "   ----------" << std::endl;
	for( size_t ip=0; ip < right.m_data[systemAddress].size(); ip++){
	  int bc=((right.m_data[systemAddress]).at(ip)).first;
	  out << right.getSectorLogicData( systemAddress, subSystemAddress, id, bc );
	  out << " BC: " << bc;
	  out << std::endl;
	}
      }
    }

    out << "--------- Endcap Sector ---------------------------" << std::endl;
    systemAddress = right.idEndcapSystem();
    for ( side = 0; side < right.NumberOfMuonSubSystem; ++side ) {
      if ( side == 0 ) {
        out << "   ------ A Side (Z>0)  ---------------------------" << std::endl;
        subSystemAddress = right.idSideA();
      } else {
        out << "   ------ C Side (Z<0)  ---------------------------" << std::endl;
        subSystemAddress = right.idSideC();
      }
      for ( id = 0; id < right.numberOfEndcapSector() ;++id ) {
        out << "      --- Sector ID  :  " << id << "   ----------" << std::endl;
	for( size_t ip=0; ip < right.m_data[systemAddress].size(); ip++){
	  int bc=((right.m_data[systemAddress]).at(ip)).first;
	  out << right.getSectorLogicData( systemAddress, subSystemAddress, id, bc );
	  out << " BC: " << bc;
	  out << std::endl;
	}
      }
    }

    out << "--------- Forward Sector ---------------------------" << std::endl;
    systemAddress = right.idForwardSystem();
    for ( side = 0; side < right.NumberOfMuonSubSystem; ++side ) {
      if ( side == 0 ) {
        out << "   ------ A Side (Z>0)  ---------------------------" << std::endl;
        subSystemAddress = right.idSideA();
      } else {
        out << "   ------ C Side (Z<0)  ---------------------------" << std::endl;
        subSystemAddress = right.idSideC();
      }
      for ( id = 0; id < right.numberOfForwardSector() ;++id ) {
        out << "      --- Sector ID  :  " << id << "   ----------" << std::endl;
	for( size_t ip=0; ip < right.m_data[systemAddress].size(); ip++){
	  int bc=((right.m_data[systemAddress]).at(ip)).first;
	  out << right.getSectorLogicData( systemAddress, subSystemAddress, id, bc );
	  out << " BC: " << bc;
	  out << std::endl;
	}
      }
    }

    return out;
  }
}
